#include <unistd.h>
#include <QDebug>
#include "hyssystem.h"
#include "Sensor/sensor.h"
#include "Action/actionctrl.h"
#include "logdatatype.h"
#include "HysAppInteraction/appmsgtype.h"
#include "HysAppInteraction/hysappmsg.h"
#include <QThread>
#include <QDateTime>
#include "./Action/alarm.h"

/********/
#define     SHARED_MEM_DATAGRP_NAME             "HysControllerLogic"

/****/
#define     HYS_SYSTEM_POWERON_TIME_SET       1000
/****/
#define     HYS_SYSTEM_RESETSESON_TIMESET      60000
#define     HYS_SYSTEM_PERSON_REPORT_TIMESET    200
#define     HYS_SYSTEM_PERSONLEAVE_RESUME_TIMESET    5000//人员离开后，恢复时间

#define     HYS_SYSTEM_MERMORY_REFRESH_TIMESET    100

#define     HYS_EXIT_ADDRCONFIGMODE_TIME_SET       3000
/****/
static stZeroPersonLock zeroPersonLock[3];
static u8 AllZeroPersonLockState = 0;//0级标签
static u16 PersonLockStateTimerId;//2S闭锁解除
static u8 UnlockTimerFlag = 0;
static ParamUnit personLockStateLast;
static u16 UnlockTimer = 0;
static u16 personLockTimer = 0;   //2级标签挂起时间

/****/
HysSystem *HysSystem::that =NULL;

/*******/
HysSystem::HysSystem(QObject *parent) :
    QObject(parent)
{
    m_ParamChangedMutex = new QMutex();
    m_SysStopMutex = new QMutex();
    SharedMemCreat();
}

HysSystem::~HysSystem()
{
    delete m_ParamChangedMutex;
}

HysSystem *HysSystem::getInston(QObject *parent)
{
    if( that == NULL)
    {
        that = new HysSystem(parent);
    }
    return that;
}

void HysSystem::Init(void)
{
    memset(SysStopState,0x00,sizeof (SysStopState));
    kzqTurnonReson = 0;

    RunParamChangedListInit();

    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston(this);
    pParamMng->SetRunParamChangeCallBack(InsertRunParamChangedList);
    pParamMng->SetSystemParamChangeCallBack(SystemParamChangedProc);

    zeroPersonLock[0].ZeroPersonLockNum = pParamMng->GetRunValueGlobalCurrentSupportID()-1;
    zeroPersonLock[1].ZeroPersonLockNum = pParamMng->GetRunValueGlobalCurrentSupportID();
    zeroPersonLock[2].ZeroPersonLockNum = pParamMng->GetRunValueGlobalCurrentSupportID()+1;

    /*****************架号信息等初始化***********************/
    u_int16_t ScId = pParamMng->GetRunValueGlobalCurrentSupportID();
    u_int16_t AreaType = pParamMng->GetSystemValueControllerArearType();
    ParamInit(ScId,AreaType);

    /*****************显示内容等初始化***********************/
    QStringList strlist;

    /**单动作显示名称***/
    QMap<int,QString> tempmap;
    tempmap.clear();
    strlist.clear();
    pParamMng->GetRunSingleActIdString(tempmap);
    strlist = tempmap.values();
    SetSingleActString(strlist);

    /**组合动作显示名称***/
    tempmap.clear();
    strlist.clear();
    pParamMng->GetRunAssembleActIdString(tempmap);
    strlist = tempmap.values();
    SetAssembleActString(strlist);

    /**成组动作显示名称***/
    tempmap.clear();
    strlist.clear();
    pParamMng->GetRunGrpActIdString(tempmap);
    strlist = tempmap.values();
    SetGrpActString(strlist);

    /**部件动作显示名称***/
    tempmap.clear();
    strlist.clear();
    pParamMng->GetRunPartIdString(tempmap);
    strlist = tempmap.values();
    SetPartString(strlist);

    /****系统故障显示名称*****************************/
    strlist.clear();
    for(int i = 0;  i < SYSTEM_EVENT_MAX;i++)
    {
        strlist.append(QString::fromUtf8(SystemLogEventTab[i]));
    }
    SetSystemErrString(strlist);

    /****服务类操作显示名称*****************************/
    strlist.clear();
    for(int i = 0;  i < SERVICE_EVENT_MAX;i++)
    {
        strlist.append(QString::fromUtf8(ServiceLogEventTab[i]));
    }
    SetSerivceOperateString(strlist);

    /***设置部件的具体信息***/
    QList<PartActSensorRealatedType> partlist;
    partlist.clear();
    pParamMng->GetRunPartInf(partlist);
    PartInfInit(partlist);

    /**传感器显示名称及ID***/
    strlist.clear();
    QList<SensorDisplayParamType> sensorvaluelist;
    sensorvaluelist.clear();
    pParamMng->GetRunSensorDisplayInf(sensorvaluelist);
    SensorInfInit(sensorvaluelist);
    for(u_int16_t i = 0; i < sensorvaluelist.size();i++)
    {
        strlist.append(sensorvaluelist.at(i).Name);
    }
    SetSensorString(strlist);

    /************************传感器模型初始化********************/
    SensorParamType *pSensorParam;
    SensorStateType *pSensorState;
    for(u_int16_t i = 1; i <= HYS_SENSOR_NUMB_MAX;i++)
    {
        pSensorParam = NULL;
        pSensorParam = pParamMng->GetRunValueSensorParam(i);
        if(pSensorParam == NULL)
            break;
        pSensorState = (SensorStateType *)GetSensorState(i);
        if(pSensorState == NULL)
            break;
        SensorModuleInit(pSensorParam,pSensorState,i-1);
        /***对设备类型为模拟量的传感器，直接设置为设备在线***/
        if(pSensorParam->pointID.driver == DRIVER_AD)
        {
            SensorSetExtDevOffLineApi(pSensorParam->pointID.driver,\
                                      pSensorParam->pointID.device,\
                                      pSensorParam->pointID.subDevice,\
                                      OnlineType_ONLINE);
        }
    }

    /***********************外围设备模型初始化***********************/
    QList<ExtDevDisplayParamType> paramlist;
    strlist.clear();
    paramlist.clear();
    pParamMng->GetRunExtDevDisplayInf(paramlist,strlist);
    SetExtDevString(strlist);
    HysScExtDevType extdev;
    memset(&extdev.Id,0x00,sizeof (HysScExtDevType));
    for(u_int16_t i = 1 ; i <= paramlist.size(); i++)
    {
        extdev.Id = paramlist.at(i-1).Id;
        extdev.Enabled = paramlist.at(i-1).Enabled;
        extdev.DriverId = paramlist.at(i-1).DriverId;
        extdev.DevType = paramlist.at(i-1).DevType;
        extdev.DevId = paramlist.at(i-1).DevId;
        extdev.Status = paramlist.at(i-1).Status;
        if(i == 1)//控制器
        {
            extdev.AppVersionHigh = (A7CONTROLLER_LOGIC_VERSION_1<<8)|A7CONTROLLER_LOGIC_VERSION_2;
            extdev.AppVersionLow = (A7CONTROLLER_LOGIC_VERSION_3<<8)|A7CONTROLLER_LOGIC_VERSION_4;
        }
        else
        {
            extdev.AppVersionHigh = 0;
            extdev.AppVersionLow = 0;
        }
        SetExtDevState(i,extdev);
    }

    /****运行信息初始化**/
    SystemParamIpType sysip;
    pParamMng->GetSystemValueSystemIp(sysip);

    SetRunScSelfScIpAddr((((sysip.IpAddr1&0x00FF)<<8)|sysip.IpAddr2),(((sysip.IpAddr3&0x00FF)<<8)|sysip.IpAddr4));
    SetRunScSelfServerEnabled(pParamMng->GetRunValueGlobalSysServerEnabled());


    SharedMemRefreshProc(0);

    WriteLogApi(SYSTEM_EVENT_PROGRAM_RESET,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_RESET_PORRSTF,0,0);
}

/****/
int HysSystem::SetSingleActString(const QStringList strlist)
{
    SingleActStr = strlist;

    return 0;
}

int HysSystem::SetAssembleActString(const QStringList strlist)
{
    AssembleActStr = strlist;

    return 0;
}

int HysSystem::SetGrpActString(const QStringList strlist)
{
    GrpActStr = strlist;

    return 0;
}

int HysSystem::SetPartString(const QStringList strlist)
{
    PartStr = strlist;

    return 0;
}

int HysSystem::SetSensorString(const QStringList strlist)
{
    SensorStr = strlist;

    return 0;
}

int HysSystem::SetExtDevString(const QStringList strlist)
{
    ExtDevStr = strlist;

    return 0;
}

int HysSystem::SetSystemErrString(const QStringList strlist)
{
    SystemErrStr = strlist;

    return 0;
}

int HysSystem::SetSerivceOperateString(const QStringList strlist)
{
    ServiceStr = strlist;

    return 0;
}

QString HysSystem::GetSingleActString(int number)
{
    if(number > SingleActStr.size())
        return "";
    else
        return SingleActStr.at(number-1);
}

QString HysSystem::GetAssembleActString(int number)
{
    if(number > AssembleActStr.size())
        return "";
    else
        return AssembleActStr.at(number-1);
}

QString HysSystem::GetGrpActString(int number)
{
    if(number > GrpActStr.size())
        return "";
    else
        return GrpActStr.at(number-1);
}

QString HysSystem::GetPartString(int number)
{
    if(number > PartStr.size())
        return "";
    else
        return PartStr.at(number-1);
}

QString HysSystem::GetSensorString(int number)
{
    if(number > SensorStr.size())
        return "";
    else
        return SensorStr.at(number-1);
}
QString HysSystem::GetSystemErrString(int number)
{
    if(number > SystemErrStr.size())
        return "";
    else
        return SystemErrStr.at(number-1);
}

QString HysSystem::GetSerivceOperateString(int number)
{
    if(number > ServiceStr.size())
        return "";
    else
        return ServiceStr.at(number-1);
}

/***设置驱动的状态***/
bool HysSystem::SetDriverExtDevState(u_int16_t driverid, u_int16_t devid, u_int16_t childdevid, DriverExtDevStatusType driverdevstate)
{
    QList<u_int16_t> extdevid_list;
    QList<u_int16_t> oldstatus_list;
    u8 temp[30];
    u_int16_t length;
    HysScExtDevType  HysScExtDevStatus;

    extdevid_list.clear();
    oldstatus_list.clear();

    if(GetExtDevState(driverid,devid,childdevid,extdevid_list,oldstatus_list) == false)
    {
//        qDebug("++++HysSystem++++GetExtDevState,return is %d",false);
        return false;
    }

//    qDebug("++++HysSystem++++SetDriverExtDevState,driverdevstate status is %d",driverdevstate.Status);

    for(int i = 0; i < extdevid_list.size() && i < HYS_EXTDEV_NUB_MAX; i++)
    {
        if(extdevid_list.at(i) == 0)
            continue;

        SetExtDevState(extdevid_list.at(i),driverdevstate);
        SensorSetExtDevOffLineApi(driverid,devid,childdevid,driverdevstate.Status);

        /******/
        memset(temp,0,sizeof(temp));

        temp[0] = REPORTENUM_DEVICE;
        temp[1] = ENUM_DEVICE;
        temp[2] = extdevid_list.at(i);
        length = sizeof(HysScExtDevType)-0x02;

        GetExtDevState(extdevid_list.at(i),HysScExtDevStatus);
        memcpy(&temp[3],&length,2);
        memcpy(&temp[5],&HysScExtDevStatus.Enabled,length);

        LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
        u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();
        UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                       temp,5+length);
    }
    return true;
}

/**写日志相关函数**/
int HysSystem::WriteFaultApi(u_int16_t host_type, u_int16_t host_name, u_int16_t event, u_int16_t status)
{
    return 0;
}

int HysSystem::WriteLogApi(u_int16_t host, u_int16_t type, u_int16_t status,void *data ,u_int16_t len)
{
    if(type > LOG_TYPE_SYSTEM_ALARM)
        return -1;
    if(status > LOG_STATUS_START_IN)
        return -1;

    /*****/
    LoggerApiType log;
    log.time.clear();
    log.Event.clear();
    log.State.clear();

    QDateTime time;
    time = QDateTime::currentDateTime();
    log.time = time.toString("yyyy-MM-dd hh:mm:ss.zzz");


    switch(type)
    {
        case LOG_TYPE_HANDLE_SINGLE:
            if(host <= SingleActStr.size() )
                log.Event.append(SingleActStr.at(host-1));
            break;

        case LOG_TYPE_HANDLE_COMB:
            if(host <= AssembleActStr.size() )
                log.Event.append(AssembleActStr.at(host-1));
            break;

        case LOG_TYPE_HANDLE_GROUP:
            if(host <= GrpActStr.size() )
                log.Event.append(GrpActStr.at(host-1));
            break;

        case LOG_TYPE_SERVICE:
            if(host <= ServiceStr.size() )
                log.Event.append(ServiceStr.at(host-1));
            break;

        case LOG_TYPE_ALARM:
            if(host <= SensorStr.size() )
                log.Event.append(SensorStr.at(host-1));
            break;

        case LOG_TYPE_SYSTEM_ALARM:
            if(host <= SystemErrStr.size() )
                log.Event.append(SystemErrStr.at(host-1));
            break;

        default:
            break;
    }

    log.State.append(QString::fromUtf8(RunLogStatusTab[status]));
    qDebug()<<log.time<<log.Event<<log.State;
    InsertLogInf(log);
    return 0;
}

/********************************************************************************
* 功能描述： 无线总线
* 入口参数：
* 返回值：无
********************************************************************************/
void HysSystem::SendWirelessBusData(uint8_t state,uint8_t id)//发送无线总线的数据。待定
{
    u_int8_t temp[10];
    temp[0] = state;
    temp[1] = id;

    DevCtrlInquire(DRIVER_UART3,EXTDEV_TYPE_WIRELESS,1,WIRELESS_MODULE_POINT_WIRELESS_BUS,\
                   temp,2);
}

/***紧急事件发生时，控制器间的相关处理***/
void HysSystem::netReportStopLockEndState(u8 stateType,u8 state)
{
    u8 temp[5];

    memset(temp,0,sizeof(temp));

    if(stateType == SystemEmergencySrcType_Stop)
    {
        temp[0] = stateType;
        temp[1] = ENUM_SYSTEM;
        temp[2] = state;

        UdpAppProcCtrl(V4APP_DEV_TYPE_SC,UDP_LINK_BROADCAST_ADDR,V4APP_STATUS_REPORT,temp,3);
        UdpAppProcCtrl(V4APP_DEV_TYPE_SC,UDP_LINK_BROADCAST_ADDR,V4APP_STATUS_REPORT,temp,3);
        LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
        if(pParamMng->GetRunValueGlobalDeviceeWirelessBusEnabled() != 0)//此处发送的无线数据，可能会有问题
        {
           SendWirelessBusData(state,(u8)pParamMng->GetRunValueGlobalCurrentSupportID());
        }
    }
    else if(stateType == SystemEmergencySrcType_Lock)
    {
        temp[0] = stateType;
        temp[1] = ENUM_SYSTEM;
        temp[2] = state;

        UdpAppProcCtrl(V4APP_DEV_TYPE_SC,UDP_LINK_BROADCAST_ADDR,V4APP_STATUS_REPORT,temp,3);
        UdpAppProcCtrl(V4APP_DEV_TYPE_SC,UDP_LINK_BROADCAST_ADDR,V4APP_STATUS_REPORT,temp,3);
    }
    else if(stateType == SystemEmergencySrcType_End)
    {
        temp[0] = stateType;
        temp[1] = ENUM_SYSTEM;
        temp[2] = state;

        UdpAppProcCtrl(V4APP_DEV_TYPE_SC,UDP_LINK_BROADCAST_ADDR,V4APP_STATUS_REPORT,temp,3);
    }
    else if(stateType == SystemEmergencySrcType_Person_Lock)
    {
        temp[0] = stateType;
        temp[1] = ENUM_SYSTEM;
        temp[2] = state;

        /*****/
        LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
        u_int16_t scID = pParamMng->GetRunValueGlobalCurrentSupportID();
        u_int16_t scMax = pParamMng->GetRunValueGlobalMaxSupportID();
        u_int16_t scMin = pParamMng->GetRunValueGlobalMinSupportID();
        if(scID <scMax)
            UdpAppProcCtrl(V4APP_DEV_TYPE_SC,scID+1,V4APP_STATUS_REPORT,temp,3);

        if(scID >scMin)
            UdpAppProcCtrl(V4APP_DEV_TYPE_SC,scID-1,V4APP_STATUS_REPORT,temp,3);
    }
}

/***紧急事件发生时，对遥控器的相关操作***/
void HysSystem::WirelessHandleStopLockEndStateProc(u8 stateType,u8 state)
{
    if(state == SYSSTOP_UNPRESS)
        return ;

    if((stateType == SystemEmergencySrcType_Stop)\
       ||(stateType == SystemEmergencySrcType_Lock)\
       ||(stateType == SystemEmergencySrcType_End))
    {
        if((GetRunScCtrlScWirelessCtrlFlag() != WIRELESS_MODULE_CODE_MATCH_NONE)\
            &&(GetRunScCtrlScWirelessCtrlFlag() != WIRELESS_MODULE_CODE_MATCH_UNMATCED))
        {
            u_int16_t buzzerState = SELF_IO_POINT_BUZZER_MODE_NONE;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzerState,0x02);
            u_int8_t temp[5];
            temp[0] = WIRELESS_MODULE_CODE_MATCH_UNMATCED;
            temp[1] = 0;
            DevCtrlInquire(DRIVER_UART3,EXTDEV_TYPE_WIRELESS,1,WIRELESS_MODULE_POINT_CODE_MATCH,\
                           temp,2);
            qDebug()<<"************WirelessHandleStopLockEndStateProc************"<<stateType<<state;
        }
    }
}

/***SysStopState**/
bool HysSystem::SetSysStopStateStopFlag(u_int16_t scnumber, bool flag)
{
    qDebug()<<"***************SetSysStopStateStopFlag****************"<<scnumber<<flag;
    if(scnumber > V4APP_DEV_ID_MAX+6)
        return false;
    else
    {
        if(SysStopState[scnumber].SysStop.Stop == flag)//规避数据相同的问题
            return false;
        m_SysStopMutex->lock();
        SysStopState[scnumber].SysStop.Stop = flag;
        m_SysStopMutex->unlock();
    }

    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t scID = pParamMng->GetRunValueGlobalCurrentSupportID();
    if(scnumber == scID)
    {
        netReportStopLockEndState(SystemEmergencySrcType_Stop,flag);
        u_int16_t backupfalg = GetSystemSelfStateHardWareStopStatus();
        SetSystemSelfStateHardWareStopStatus(flag);
        if(flag == true)
        {
            u_int16_t ctrlmode = DEV_POINT_CTRL_ON;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_LED_STOP,(u_int8_t *)&ctrlmode,0x02);
            WriteLogApi(SYSTEM_EVENT_HARDWARE_STOP,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_HAPPEN,0,0);
        }
        else
        {
            if(backupfalg == 1)
            {
                u_int16_t ctrlmode = DEV_POINT_CTRL_OFF;
                DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                               SELF_IO_POINT_LED_STOP,(u_int8_t *)&ctrlmode,0x02);
                WriteLogApi(SYSTEM_EVENT_HARDWARE_STOP,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_RESTORE,0,0);
            }
        }
    }

    /****/
    HysScSystemInfType systembackup;
    GetSystemSelfState(systembackup);

    /****/
    if(flag == true)
    {
        SetSystemSelfStateStopStatus(flag);
        SetRunScSelfStopPosition(scnumber);
        qDebug()<<"***************SetRunScSelfStopPosition****************"<<scnumber<<flag;
    }
    else//查找此时是否可以解除急停
    {
        u_int16_t stopflag = 0;
        int i= 0;
        for(i= 0;i< (V4APP_DEV_ID_MAX+6);i++)
        {
            if(SysStopState[i].SysStop.Stop == true)
            {
                qDebug()<<"SysStopState[i].SysStop.Stop"<<i;
                stopflag = 1;
                break;
            }
        }

        if(stopflag == 1)
        {
            SetSystemSelfStateStopStatus(stopflag);
            SetRunScSelfStopPosition(i);
        }
        else
        {
            SetSystemSelfStateStopStatus(0);
            SetRunScSelfStopPosition(0);
        }
    }

    /*****/
    HysScSystemInfType systemnew;
    GetSystemSelfState(systemnew);
    u_int16_t StopDevNumber = 0;
    GetRunScSelfStopPosition(StopDevNumber);

    /***急停动作处理，并上报给服务器***/
    if(memcmp(&systembackup.StopStatus,&systemnew.StopStatus,0x02) != 0)
    {
        if((systemnew.StopStatus == SYSSTOP_PRESS)\
            &&(systembackup.StopStatus == SYSSTOP_UNPRESS))
        {
            ActionCtrl *pActCtrl = ActionCtrl::getInston();
            if((StopDevNumber >= V4APP_DEV_ID_MIN) && (StopDevNumber <= V4APP_DEV_ID_MAX))
                pActCtrl->StopAllAction(ActionCtrl_SourceType_CONTROLLER);
            else
                pActCtrl->StopAllAction(ActionCtrl_SourceType_CONTROLLER+StopDevNumber-V4APP_DEV_ID_MAX);//已经进行强制转换
            u_int16_t buzzermode = SELF_IO_POINT_BUZZER_LOCKED;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
            u_int8_t buf[1] = {0};
            UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(scID),V4APP_STATUS_MANUAL_ACT_HEARTBEAT,buf,0);
            WirelessHandleStopLockEndStateProc(SystemEmergencySrcType_Stop,SYSSTOP_PRESS);
            DevEmvdClearRealTimeActList();
        }
        else if((systemnew.StopStatus == SYSSTOP_UNPRESS)\
                &&(systembackup.StopStatus == SYSSTOP_PRESS))
        {
            u_int16_t buzzermode = SELF_IO_POINT_BUZZER_UNLOCKED;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
        }
        /*****/
        u_int8_t temp[100];

        /*****/
        temp[0] = REPORTENUM_SYSTEM;
        temp[1] = ENUM_SYSTEM;//
        temp[2] = 0;
        u_int16_t length =sizeof(HysScSystemInfType);
        memcpy(&temp[3],&length,2);
        memcpy(&temp[5],&systemnew,length);

        u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();

        UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                           temp,5+length);
    }
    return  true;
}

bool HysSystem::SetSysStopStateLockFlag(u_int16_t scnumber, bool flag)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t scID = pParamMng->GetRunValueGlobalCurrentSupportID();
    /*****/
    if(scnumber > V4APP_DEV_ID_MAX+6)
    {
        return false;
    }
    else if((scnumber != (scID+1))\
          && (scnumber != (scID-1))\
          && (scnumber != scID))
    {
       return false;
    }
    else
    {
        if(SysStopState[scnumber].SysStop.Lock == flag)//规避数据相同的问题
            return false;

        m_SysStopMutex->lock();
        SysStopState[scnumber].SysStop.Lock = flag;
        m_SysStopMutex->unlock();
    }

    /*****/
    if(scnumber == scID)
    {
        netReportStopLockEndState(SystemEmergencySrcType_Lock,flag);
        u_int16_t backupfalg = GetSystemSelfStateHardWareLockStatus();
        SetSystemSelfStateHardWareLockStatus(flag);
        if(flag == true)
        {
            u_int16_t ctrlmode = DEV_POINT_CTRL_ON;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_LED_STOP,(u_int8_t *)&ctrlmode,0x02);
            WriteLogApi(SYSTEM_EVENT_HARDWARE_LOCK,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_HAPPEN,0,0);
        }
        else
        {
            if(backupfalg == 1)
            {
                u_int16_t ctrlmode = DEV_POINT_CTRL_OFF;
                DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                               SELF_IO_POINT_LED_STOP,(u_int8_t *)&ctrlmode,0x02);
                WriteLogApi(SYSTEM_EVENT_HARDWARE_LOCK,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_RESTORE,0,0);
           }
        }
    }

    /****/
    HysScSystemInfType systembackup;
    GetSystemSelfState(systembackup);

    /****/
    if(flag == true)
    {
        SetSystemSelfStateLockStatus(flag);
    }
    else//查找此时是否可以解除闭锁
    {
        u_int16_t lockflag = 0;
        int i = 0;
        for(i = 0;i < (V4APP_DEV_ID_MAX+6);i++)
        {
            if(SysStopState[i].SysStop.Lock == true)
            {
                lockflag = 1;
                break;
            }
        }

        if(lockflag == 1)
        {
            SetSystemSelfStateLockStatus(lockflag);
        }
        else
        {
            SetSystemSelfStateLockStatus(0);
        }
    }

    /*****/
    HysScSystemInfType systemnew;
    GetSystemSelfState(systemnew);

    /***闭锁动作处理，并上报给服务器***/
    if(memcmp(&systembackup.LockStatus,&systemnew.LockStatus,0x02) != 0)
    {
        if((systemnew.LockStatus == SYSSTOP_PRESS)\
            &&(systembackup.LockStatus == SYSSTOP_UNPRESS))
        {
            ActionCtrl *pActCtrl = ActionCtrl::getInston();
            pActCtrl->StopAllAction(ActionCtrl_SourceType_CONTROLLER);
            u_int16_t buzzermode = SELF_IO_POINT_BUZZER_LOCKED;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
            u_int8_t buf[1] = {0};
            UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(scID),V4APP_STATUS_MANUAL_ACT_HEARTBEAT,buf,0);
            WirelessHandleStopLockEndStateProc(SystemEmergencySrcType_Lock,SYSSTOP_PRESS);
            DevEmvdClearRealTimeActList();
        }
        else if((systemnew.LockStatus == SYSSTOP_UNPRESS)\
                &&(systembackup.LockStatus == SYSSTOP_PRESS))
        {
            u_int16_t buzzermode = SELF_IO_POINT_BUZZER_UNLOCKED;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
        }
        /*****/
        u_int8_t temp[100];

        temp[0] = REPORTENUM_SYSTEM;
        temp[1] = ENUM_SYSTEM;//
        temp[2] = 0;
        u_int16_t length =sizeof(HysScSystemInfType);
        memcpy(&temp[3],&length,2);
        memcpy(&temp[5],&systemnew,length);
        u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();

        UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                           temp,5+length);
    }

    return  true;
}

bool HysSystem::SetSysStopStateEndFlag(u_int16_t scnumber, bool flag)
{
    qDebug()<<"************SetSysStopStateEndFlag************"<<scnumber<<flag;
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t scID = pParamMng->GetRunValueGlobalCurrentSupportID();
    u_int16_t EndScope = pParamMng->GetRunValueGlobalStopScope();
    /****/
    HysScSystemInfType systembackup;
    GetSystemSelfState(systembackup);

    /*****/
    if(scnumber > V4APP_DEV_ID_MAX+6)
        return false;
    else if(((scnumber != V4APP_DEV_ID_YKQ)\
          &&(scnumber != V4APP_DEV_ID_SERVER)\
          &&(scnumber != V4APP_DEV_ID_CONSOLE)\
          &&(scnumber != V4APP_DEV_ID_REMOTE))\
          &&((scnumber > (scID+EndScope))\
          || ((scnumber < (scID-EndScope))&&(scID > EndScope))\
          || ((scnumber < V4APP_DEV_ID_MIN)&&(scID <= EndScope))))
    {
       return false;
    }
    else
    {
        if(SysStopState[scnumber].SysStop.End == flag)//规避数据相同的问题
            return false;

        m_SysStopMutex->lock();
        SysStopState[scnumber].SysStop.End = flag;
        m_SysStopMutex->unlock();
    }

    /*****/
    if(((scnumber == scID)||(scnumber == V4APP_DEV_ID_YKQ))&&(flag == true))
    {
        netReportStopLockEndState(SystemEmergencySrcType_End,flag);
        SetSystemSelfStateHardWareEndStatus(flag);
        WriteLogApi(SYSTEM_EVENT_HARDWARE_END,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_HAPPEN,0,0);
    }


    /****/
    if(flag == true)
    {
        SetSystemSelfStateEndStatus(flag);
        if(scnumber == V4APP_DEV_ID_YKQ)
            SetRunScSelfEndPosition(scID);
        else
            SetRunScSelfEndPosition(scnumber);
    }
    else//查找此时是否可以解除停止
    {
        u_int16_t endflag = 0;
        int i = 0;
        for(i = 0;i < (V4APP_DEV_ID_MAX+6);i++)
        {
            if(SysStopState[i].SysStop.End == true)
            {
                endflag = 1;
                break;
            }
        }

        if(endflag == 1)
        {
            SetSystemSelfStateEndStatus(endflag);
            SetRunScSelfEndPosition(i);
        }
        else
        {
            SetSystemSelfStateEndStatus(0);
            SetRunScSelfEndPosition(0);
        }
    }

    /*****/
    HysScSystemInfType systemnew;
    GetSystemSelfState(systemnew);
    u_int16_t EndDevNumber = 0;
    GetRunScSelfEndPosition(EndDevNumber);

    /***停止动作处理，并上报给服务器***/
    if(memcmp(&systembackup.EndStatus,&systemnew.EndStatus,0x02) != 0)
    {
        if((systemnew.EndStatus == SYSSTOP_PRESS)\
            &&(systembackup.EndStatus == SYSSTOP_UNPRESS))
        {
            ActionCtrl *pActCtrl = ActionCtrl::getInston();
            if((EndDevNumber >= V4APP_DEV_ID_MIN) && (EndDevNumber <= V4APP_DEV_ID_MAX))
                pActCtrl->StopAllAction(ActionCtrl_SourceType_CONTROLLER);
            else
                pActCtrl->StopAllAction(ActionCtrl_SourceType_CONTROLLER+EndDevNumber-V4APP_DEV_ID_MAX);//已经进行强制转换
            u_int16_t buzzermode = SELF_IO_POINT_BUZZER_END_PRESSED;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
            u_int8_t buf[1] = {0};
            UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(scID),V4APP_STATUS_MANUAL_ACT_HEARTBEAT,buf,0);
            DevEmvdClearRealTimeActList();
            if(scnumber != V4APP_DEV_ID_YKQ)
                WirelessHandleStopLockEndStateProc(SystemEmergencySrcType_End,SYSSTOP_PRESS);
            qDebug()<<"************pActCtrl->StopAllActio******stop******";
        }

        /*****/
        u_int8_t temp[100];

        /*****/
        temp[0] = REPORTENUM_SYSTEM;
        temp[1] = ENUM_SYSTEM;//
        temp[2] = 0;
        u_int16_t length =sizeof(HysScSystemInfType);
        memcpy(&temp[3],&length,2);
        memcpy(&temp[5],&systemnew,length);
        u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();

        UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                           temp,5+length);
    }
    return  true;
}

bool HysSystem::GetSysStopStateStopFlag(uint scid)
{
    if(scid > V4APP_DEV_ID_MAX+6)
        return false;
    else
        return  SysStopState[scid].SysStop.Stop;
}

bool HysSystem::GetSysStopStateLockFlag(uint scid)
{
    if(scid > V4APP_DEV_ID_MAX+6)
        return false;
    else
        return  SysStopState[scid].SysStop.Lock;
}

bool HysSystem::GetSysStopStateEndFlag(uint scid)
{
    if(scid > V4APP_DEV_ID_MAX+6)
        return false;
    else
        return  SysStopState[scid].SysStop.End;
}

/***0即级标签**/
bool HysSystem::GetAllZeroPersonLockState(void)
{
    return false;
}

int HysSystem::IsEmergencyStopAction(SysCtrlInfoType *pctrl)
{
    u8 leftid=0;
    u8 rightid=0;
    u8 localid=0;
#if 0
    if((pctrl->actionType ==ENUM_COMBINATION_ACTION)||(pctrl->actionType ==ENUM_GROUP_ACTION)
      ||((pctrl->actionType ==ENUM_SINGLE_ACTION)&&(pctrl->ctrl.CtrlMode==ActionCtrl_ModeType_AUTO)))
    {
        if(SysState.globalState.emergencyState == SYSSTOP_PRESS)
            return 1;
        else
            return 0;
    }
    else
    {
        if(SysParam.globalParam.SupportParam.currentSupportID <SysParam.globalParam.SupportParam.MaxSupportID)
            rightid =SysParam.globalParam.SupportParam.currentSupportID+1;

        if(SysParam.globalParam.SupportParam.currentSupportID >SysParam.globalParam.SupportParam.MinSupportID)
            leftid =SysParam.globalParam.SupportParam.currentSupportID-1;

        localid = SysParam.globalParam.SupportParam.currentSupportID;
        if((leftid!=0)&&(SysStopState[leftid].SYSStop.EMERGENCY == SYSSTOP_PRESS))
            return 1;
        if((rightid!=0)&&(SysStopState[rightid].SYSStop.EMERGENCY == SYSSTOP_PRESS))
            return 1;
        if((localid!=0)&&(SysStopState[localid].SYSStop.EMERGENCY == SYSSTOP_PRESS))
            return 1;

        return 0;
    }
#endif

    return 0;
}

/************************/
CombActionStateType *HysSystem::GetCombActState(u_int16_t comb_id)
{
    if((comb_id > HYS_ASSEMBLELEACT_NUMB_MAX)||(comb_id == 0))
         return NULL;

    return &CombAction[comb_id-1];
}

groupActionStateType * HysSystem::GetGrpActState(u_int16_t grp_id)
{
    if((grp_id > HYS_GRPACT_NUMB_MAX)||(grp_id == 0))
         return NULL;

    return &GroupAction[grp_id-1];
}

/********************周期性处理相关的操作*********************************************/
/*******************************************************************************************
*功能描述：设备系统状态 服务器
*入口参数：delay取值：
*            延时时间
*返回值：  无实际意义：
*******************************************************************************************/
int HysSystem::ServerReportSystemState(u16 delay)
{
    u8 temp[100];
    u_int16_t serverID;
    u16 length;
    u_int16_t ret;
    static uint RsetReasonTick = 0;
    static uint resetReasonflag = 1;
    static HysScSystemInfType SystemStateBackup;
    HysScSystemInfType SystemStateNew;

    /******/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    serverID = pParamMng->GetRunValueGlobalSysServerAddr();
    GetRunScSelfServerCommErr(ret);
    if((pParamMng->GetRunValueGlobalSysServerEnabled() == 0)\
        ||(ret != 0))
        return 1;

    GetSystemSelfState(SystemStateNew);
    if(RsetReasonTick < HYS_SYSTEM_RESETSESON_TIMESET)
        RsetReasonTick +=delay;
    if(RsetReasonTick >= HYS_SYSTEM_RESETSESON_TIMESET && resetReasonflag == 1)
    {
        resetReasonflag = 0;
        /**真正的数据从未设置***/
        SystemStateNew.SysResetFlag = kzqTurnonReson;
    }

    if(0 != memcmp(&SystemStateBackup.StopStatus,&SystemStateNew.StopStatus,sizeof (HysScSystemInfType)))
    {
        /*****/
        temp[0] = REPORTENUM_SYSTEM;
        temp[1] = ENUM_SYSTEM;//
        temp[2] = 0;
        length =sizeof(HysScSystemInfType);
        memcpy(&temp[3],&length,2);
        UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SS_REPORT),static_cast<uchar>(serverID),V4APP_STATUS_REPORT,\
                           temp,5+length);
        SystemStateBackup = SystemStateNew;
    }
    return 1;
}

/*******************************************************************************************
*功能描述：向服务器上报人员信息
*入口参数：delay取值：
*            延时时间
*返回值：  无实际意义：
*******************************************************************************************/
int  HysSystem::ServerReportPersonState(u16 DelayTime)
{
    static uint ReportPersonTick = 0;
    HysPersonInf   PersonStatusBackup;
    HysPersonInf   PersonStatusResult;
    u_int8_t temp[20];

    ReportPersonTick += DelayTime;
    if(ReportPersonTick < HYS_SYSTEM_PERSON_REPORT_TIMESET)
        return -1;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();
    ReportPersonTick = 0;
    GetSystemPersonState(PersonStatusBackup);
    memset(&PersonStatusResult.TotalNumber,0,sizeof (HysPersonInf));

    for(u8 i =0;i < PERSON_LABLE_NUMB_MAX;i++)
    {
        if(PersonStatusBackup.PersonInf[i].PersonLableId == 0)
            break;
        if((PersonStatusBackup.PersonInf[i].PersonLableId != 0)\
            &&(PersonStatusBackup.PersonInf[i].PersonLableStatus != PERSON_LABLE_STATUS_INVALID))
        {
            temp[0] = REPORTENUM_PERSON;
            temp[1] = ENUM_DEVICE;//此处所写数据，在服务器端，会进行特殊处理。特此说明
            temp[2] = i+1;
            u_int16_t length =sizeof(PersonLableDevType);
            memcpy(&temp[3],&length,2);
            memcpy(&temp[5],&PersonStatusBackup.PersonInf[i].PersonLableId,0x02);
            memcpy(&temp[5],&PersonStatusBackup.PersonInf[i].PersonLableDistance,0x02);
            memcpy(&temp[5],&PersonStatusBackup.PersonInf[i].PersonLableAuthority,0x02);
            memcpy(&temp[5],&PersonStatusBackup.PersonInf[i].PersonLableStatus,0x02);

            UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                               temp,5+length);

            if(PersonStatusBackup.PersonInf[i].PersonLableStatus == PERSON_LABLE_STATUS_ENTER)
            {
                if(PersonStatusBackup.PersonInf[i].PersonLableAliveTime >= DelayTime)
                    PersonStatusBackup.PersonInf[i].PersonLableAliveTime -= DelayTime;
                else
                    PersonStatusBackup.PersonInf[i].PersonLableAliveTime = 0;
                if(PersonStatusBackup.PersonInf[i].PersonLableAliveTime != 0)
                {
                    memmove(&PersonStatusResult.PersonInf[PersonStatusResult.TotalNumber],\
                            &PersonStatusBackup.PersonInf[i].PersonLableId,\
                            sizeof(PersonLableType));
                    PersonStatusResult.TotalNumber++;
                }
            }
        }
    }

    SetSystemPersonState(PersonStatusResult);
    return 0;
}

/*******************************************************************************************
*功能描述：向服务器上报人员信息
*入口参数：delay取值：
*            延时时间
*返回值：  无实际意义：
*******************************************************************************************/
void HysSystem::SysB12VProcess(void)
{
    u16 flag,state,hostaddr;
    u16 dangganenable,bisuomin,bisuomax;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    hostaddr = pParamMng->GetRunValueGlobalCurrentSupportID();
    flag=0;
    if(pParamMng->GetRunValueGlobalCurrentSupportID() < pParamMng->GetRunValueGlobalMaxSupportID())
    {
        flag |=SysStopState[hostaddr+1].SysStop.Lock;
        flag |=SysStopState[hostaddr+1].SysStop.Stop;
    }
    if(pParamMng->GetRunValueGlobalCurrentSupportID() >pParamMng->GetRunValueGlobalMinSupportID())
    {
        flag |=SysStopState[hostaddr-1].SysStop.Lock;
        flag |=SysStopState[hostaddr-1].SysStop.Stop;
    }
    flag |=SysStopState[hostaddr].SysStop.Lock;
    flag |=SysStopState[hostaddr].SysStop.Stop;


    //ghy 增加特殊支架对B12V处理逻辑
    pParamMng->GetRunValueGlobalDangGanInf(dangganenable,bisuomin,bisuomax);
    if(dangganenable != 0)
    {
        if((hostaddr >= bisuomin)\
          && (hostaddr <= bisuomax))
        {
            flag |= SysStopState[BISUO_SUPPORT_NUM].SysStop.Lock;
        }
    }

    if(GetSystemSelfStateB12VStatus() !=flag)
    {
        SetSystemSelfStateB12VStatus(flag);
        if(GetSystemSelfStateB12VStatus() == DEV_FALUT_HAPPEN)
        {
            state = DEV_POINT_CTRL_OFF;
        }
        else
        {
            state = DEV_POINT_CTRL_ON;
        }
        DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,0x01,\
                       SELF_IO_POINT_B12V, (u_int8_t *)&state, 0x02);
    }

}

void HysSystem::SysEndProcess(u16 delay)
{
    u_int16_t  hostaddr,flag,min,max,i;
    static u16 hwstopTick = 0;
    u_int16_t hwEnd = 0;
    u_int16_t EndFlag = 0;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    hostaddr = pParamMng->GetRunValueGlobalCurrentSupportID();
    EndFlag = GetSystemSelfStateEndStatus();

    if((hostaddr+ pParamMng->GetRunValueGlobalStopScope() ) <= pParamMng->GetRunValueGlobalMaxSupportID())
        max = hostaddr+ pParamMng->GetRunValueGlobalStopScope();
    else
        max = pParamMng->GetRunValueGlobalMaxSupportID();

    if((pParamMng->GetRunValueGlobalMinSupportID()+pParamMng->GetRunValueGlobalStopScope())<=hostaddr)
        min = hostaddr- pParamMng->GetRunValueGlobalStopScope() ;
    else
        min = pParamMng->GetRunValueGlobalMinSupportID();

    if(EndFlag == SYSSTOP_PRESS)
    {
        hwstopTick += delay;
        if(hwstopTick > 2000)
        {
            hwEnd = GetSystemSelfStateHardWareEndStatus();
            SetSystemSelfStateHardWareEndStatus(SYSSTOP_UNPRESS);
            SetSystemSelfStateEndStatus(false);
            SetRunScSelfEndPosition(0);
            hwstopTick=0;
            for(i=min;i<=max;i++)
            {
                SysStopState[i].SysStop.End = SYSSTOP_UNPRESS;
            }
            SysStopState[hostaddr].SysStop.End = SYSSTOP_UNPRESS;//存在异常操作
            SysStopState[V4APP_DEV_ID_YKQ].SysStop.End = SYSSTOP_UNPRESS;//存在异常操作
        }
        else
        {
            return ;
        }
    }

    if((EndFlag != hwEnd)&&(hwEnd == SYSSTOP_PRESS))
    {
        netReportStopLockEndState(SystemEmergencySrcType_End,(u8)hwEnd);
        SetSystemSelfStateEndStatus(hwEnd);
    }


    flag=0;
    for(i=min;i<=max;i++)
    {
        flag |= SysStopState[i].SysStop.End;
    }
    flag |= SysStopState[hostaddr].SysStop.End;   //stop mode need add self state  jhy 20220823

    if(EndFlag !=flag)
    {
        SetSystemSelfStateEndStatus(flag);
        EndFlag = GetSystemSelfStateEndStatus();
        if(EndFlag == SYSSTOP_PRESS)
        {
            ActionCtrl *pActCtrl = ActionCtrl::getInston();
            pActCtrl->StopAllAction(ActionCtrl_SourceType_CONTROLLER);
            u_int8_t buf[1] = {0};
            UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(hostaddr),V4APP_STATUS_MANUAL_ACT_HEARTBEAT,buf,0);
            u_int16_t buzzermode = SELF_IO_POINT_BUZZER_END_PRESSED;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
            DevEmvdClearRealTimeActList();
        }
    }
}

/***人员报警，发送的数据，待定***/
void HysSystem::PersonCtrlAlarmProc(u8 state)
{
//    u8 alar[7]= {0};
//    alar[0] = state;
//    alar[1] = 0x01;//frame->u8DT[4];
//    alar[2] = 0x00;
//    alar[3] = 0xE2;
//    alar[4] = 0x00;
//    SysDataFun(DATA_MSG_CTRL,0x03,0x01,0x07,0,alar,0);
}

/*******************************************************************************************
*功能描述：人员闭锁处理过程函数
*入口参数：cmd取值：
*            ActionCtrl_CmdType_STOP：   动作停止
*            ActionCtrl_CmdType_SUSPEND：动作挂起
*            ActionCtrl_CmdType_RESUME： 动作恢复
*返回值：  无：
*******************************************************************************************/
void HysSystem::PersonLockControllActionProc(u8 cmd)
{
    u8 temp[4] = {0};
    u8 i = 0;
    ParamUnit   LockActionID;
    ActionCtrlType type;
    ActionCtrl *pActCtrl = ActionCtrl::getInston();

    //获取当前动作ID状态
    type.CtrlCmd = cmd;
    type.CtrlMode = ActionCtrl_ModeType_AUTO;
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    type.CtrlSourceAddr = pParamMng->GetRunValueGlobalCurrentSupportID();
    type.CtrlSourceType = ActionCtrl_SourceType_CONTROLLER;
    type.UserID = 0;

    globalParam_DevicePeopleType devpeopleparam;
    if(pParamMng->GetRunValueGlobalDevicePeopleParam(devpeopleparam) == false)
        return ;

    for(i = 0; i<2;i++)
    {
        if(i == 0)
            LockActionID = devpeopleparam.LockActionID1;
        else
            LockActionID = devpeopleparam.LockActionID2;

        if(LockActionID == 0)
            continue;
        if(cmd == ActionCtrl_CmdType_STOP)
        {
            if(CombAction[LockActionID-1].RunState == RunStateType_STOP\
                 ||CombAction[LockActionID-1].RunState == RunStateType_IDLE)
            {
                continue;
            }
            else
            {
                PersonCtrlAlarmProc(1);
                pActCtrl->CombActionCtrlFuncApi(type,LockActionID-1);
                if(personLockStateLast == 1)
                {
                    SetSystemSelfStatePersonLockStatus(1);
                    PersonLockStateTimerId = 5000;//设置时间
                    WriteLogApi(SYSTEM_EVENT_PERSON_LOCK,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_HAPPEN,0,0);

                }
                else
                {
                    SetSystemSelfStatePersonLockStatus(0);
                    WriteLogApi(LockActionID,LOG_TYPE_HANDLE_COMB,LOG_STATUS_STOP,0,0);
                    WriteLogApi(SYSTEM_EVENT_PERSON_LOCK,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_RESTORE,0,0);

                    //加日志  移架人员闭锁超时
                }
            }
        }
        else if(cmd == ActionCtrl_CmdType_SUSPEND)//暂停
        {
            if(CombAction[LockActionID-1].RunState == RunStateType_STOP\
                 ||CombAction[LockActionID-1].RunState == RunStateType_IDLE\
                 ||CombAction[LockActionID-1].RunState == RunStateType_SUSPEND)
            {
                continue;
            }
            else
            {
                UnlockTimer = 0;
                UnlockTimerFlag = 0;
                pActCtrl->CombActionCtrlFuncApi(type,LockActionID-1);
                personLockTimer = 1;
                SetSystemSelfStatePersonLockStatus(1);
                WriteLogApi(SYSTEM_EVENT_PERSON_LOCK,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_HAPPEN,0,0);

                PersonCtrlAlarmProc(1);
            }
        }
        else if(cmd == ActionCtrl_CmdType_RESUME)//恢复
        {
            if(CombAction[LockActionID-1].RunState == RunStateType_SUSPEND)
            {
                if(UnlockTimer)
                {
                    PersonCtrlAlarmProc(1);
                    pActCtrl->CombActionCtrlFuncApi(type,LockActionID-1);
                    SetSystemSelfStatePersonLockStatus(0);
                    WriteLogApi(SYSTEM_EVENT_PERSON_LOCK,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_RESTORE,0,0);
                }
                else
                {
                    PersonCtrlAlarmProc(0);
                    UnlockTimerFlag = 1;
                    UnlockTimer = 0;
                    SetSystemSelfStatePersonLockStatus(2);
                    SetRunScSelfPersonLockLeftTime(HYS_SYSTEM_PERSONLEAVE_RESUME_TIMESET);
                }
            }
        }
    }
}

/*******************************************************************************************
*功能描述：人员闭锁周期处理
*入口参数：delay取值：
*            延时时间50ms
*返回值：  无实际意义：
*******************************************************************************************/
void HysSystem::sysDevicePersonLock(u16 delay)
{
    static ParamUnit personLockState[PERSON_LABLE_NUMB_MAX];
    static uint ClearPersonLockState = 0;

    int i;
    u8  driver=0;
    uint data;
    u8 personGrades = 0;//标签等级
    u16 personDis = 0;//标签距离
    uint personNum = 0;//标签ID
    ParamUnit  Persontmp;
    ParamUnit   LockActionID;


    //人员定位未使能
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    globalParam_DevicePeopleType devpeopleparam;
    if(pParamMng->GetRunValueGlobalDevicePeopleParam(devpeopleparam) == false)
        return ;

    if(devpeopleparam.PeoplePosEnable == EnableType_DISABLE)
    {
        SetSystemSelfStatePersonLockStatus(0);
        return;
    }
    //人员闭锁未使能
    if(devpeopleparam.PeopleBisuoEnable == EnableType_DISABLE)
    {
        SetSystemSelfStatePersonLockStatus(0);
        return;
    }

    //急停闭锁停止，人员闭锁消除
    if(GetSystemSelfStateLockStatus() == 1 || GetSystemSelfStateHardWareLockStatus() == 1 \
        ||GetSystemSelfStateHardWareStopStatus()== 1|| GetSystemSelfStateStopStatus() == 1\
        ||GetSystemSelfStateHardWareEndStatus() == 1|| GetSystemSelfStateEndStatus() == 1)
    {
        SetSystemSelfStatePersonLockStatus(0);
    }

    //主要用于显示，无任何实际意义!
    if((GetSystemSelfStatePersonLockStatus() == 1)&&(PersonLockStateTimerId != 0))
    {
        if(PersonLockStateTimerId > delay)
            PersonLockStateTimerId -= delay;
        else
            PersonLockStateTimerId = 0;
        if(PersonLockStateTimerId == 0)
            SetSystemSelfStatePersonLockStatus(0);
    }

    //5后自动恢复
    if(UnlockTimerFlag)
    {
        UnlockTimer+=delay;
        SetRunScSelfPersonLockLeftTime(HYS_SYSTEM_PERSONLEAVE_RESUME_TIMESET-UnlockTimer);
    }

    if(UnlockTimer >= HYS_SYSTEM_PERSONLEAVE_RESUME_TIMESET && UnlockTimerFlag)
    {
        PersonLockControllActionProc(ActionCtrl_CmdType_RESUME);//恢复
        SetRunScSelfPersonLockLeftTime(0);
        UnlockTimer = 0;
        UnlockTimerFlag = 0;
        //清除标志
    }

    //遍历所有人员定位信息

    PersonLableType persontemp;
    for(i=0;i<PERSON_LABLE_NUMB_MAX;i++)
    {
        if(false == GetSystemPersonState(i+1,persontemp))
            break;

        personNum = persontemp.PersonLableId;//低16位
        personNum |= ((uint)persontemp.PersonLableAuthority&0xFF00)<<8;
        personDis = persontemp.PersonLableDistance;
        personGrades = persontemp.PersonLableAuthority&0xFF;//低8位
        personLockState[i] = 0xFF;

        //人员定位最小为301，300一下不处理
        if(personNum<PERSONLABLE_ID_MIN)
            continue;

        //人员距离大于闭锁距离+解锁范围
        if(personDis > devpeopleparam.BisuoRange + devpeopleparam.BisuoDifference)
        {
            personLockState[i] = 0xFF;
        }
        else if(personDis <= devpeopleparam.BisuoRange)
            personLockState[i] = personGrades;
        else
            personLockState[i] = 0xFF;
    }

    //默认值不闭锁
    Persontmp = PERSON_GRADES_15;
    //检测人员定位是否有闭锁
    for(i=0;i<PERSON_LABLE_NUMB_MAX;i++)
    {
        //查询所有标签的最小等级
        if(Persontmp>personLockState[i]/*&& personLockState[i] != 0*/)
            Persontmp = personLockState[i];
    }
    //默认值不闭锁，等级为0
    if(Persontmp >= PERSON_GRADES_3)
        Persontmp = PERSON_GRADES_3;
    else
        ClearPersonLockState = 0;

    if(Persontmp == PERSON_GRADES_0)
    {
//		if(localZeroPersonLock == 0)
        {
            zeroPersonLock[1].ZeroPersonLockState = 1;
        }
    }
    else
    {
        zeroPersonLock[1].ZeroPersonLockState = 0;
        switch(Persontmp)
        {
                //当前是1
            case PERSON_GRADES_1:
                {
                    personLockTimer = 0;
                    //上次不为1，停动作
    //				if(personLockStateLast != PERSON_GRADES_1)
                    {
                        personLockStateLast = Persontmp;
                        PersonLockControllActionProc(ActionCtrl_CmdType_STOP);//停动作
                    }
                }
                break;

                //当前是2
                case PERSON_GRADES_2:
                    {
                        switch(personLockStateLast)
                        {
                            //上次为3需要暂停
                            case PERSON_GRADES_3:
                                //暂停
                                personLockTimer = 0;
                                PersonLockControllActionProc(ActionCtrl_CmdType_SUSPEND);//暂停
                                break;
                            case PERSON_GRADES_1:
                                personLockTimer = 0;
                                break;
                            case PERSON_GRADES_2:
                                if(personLockTimer == 0)
                                    PersonLockControllActionProc(ActionCtrl_CmdType_SUSPEND);//暂停
                                else if(personLockTimer < devpeopleparam.BisuoTime*100)
                                {
                                    personLockTimer +=delay;
    //								PersonLockControllActionProc(ActionCtrl_CmdType_SUSPEND);//暂停
                                }
                                else if(personLockTimer >= devpeopleparam.BisuoTime*100)
                                {
                                    personLockTimer = 0;
                                    PersonLockControllActionProc(ActionCtrl_CmdType_STOP);//停动作
                                }
                                break;
                        }
                    }
                    break;

                case PERSON_GRADES_3:
                    {
                        personLockTimer = 0;
                        switch(personLockStateLast)
                        {
                            case PERSON_GRADES_1:
                                break;
                            case PERSON_GRADES_2:
                                PersonLockControllActionProc(ActionCtrl_CmdType_RESUME);//恢复
                                break;
                            case PERSON_GRADES_3:
                                //两倍解锁时间强制解锁
                                if(GetSystemSelfStatePersonLockStatus() != 0)
                                    ClearPersonLockState += delay;
                                if(GetSystemSelfStatePersonLockStatus() == 0)
                                    ClearPersonLockState = 0;
                                if(ClearPersonLockState>devpeopleparam.BisuoTime*100*2 && GetSystemSelfStatePersonLockStatus() != 0)
                                {
                                    if(zeroPersonLock[0].ZeroPersonLockState == 0 && \
                                       zeroPersonLock[1].ZeroPersonLockState == 0 &&\
                                       zeroPersonLock[2].ZeroPersonLockState == 0)
                                    {
                                        SetSystemSelfStatePersonLockStatus(0);
                                        ClearPersonLockState = 0;
                                    }
                                }
                                break;
                        }
                    }
                    break;
        }
    }
    personLockStateLast = Persontmp;
}

void HysSystem::sysZeroPersonLockProc(u16 delay)
{
    u8 i = 0;
    u8 localLock = 0;
    static u8 localLockLast = 0;
    if(zeroPersonLock[1].LastZeroPersonLockState != zeroPersonLock[1].ZeroPersonLockState)
    {
        zeroPersonLock[1].LastZeroPersonLockState = zeroPersonLock[1].ZeroPersonLockState;
        netReportStopLockEndState(SystemEmergencySrcType_Person_Lock,zeroPersonLock[1].ZeroPersonLockState);
    }
    for(i = 0;i<3;i++)
    {
        localLock |= zeroPersonLock[i].ZeroPersonLockState;
    }
    if(localLock != localLockLast)
    {
        if(localLock)
        {
            AllZeroPersonLockState = 1;
            SetSystemSelfStatePersonLockStatus(1);
            ActionCtrl *pActCtrl = ActionCtrl::getInston();
            pActCtrl->StopAllAction(ActionCtrl_SourceType_CONTROLLER);
            u_int8_t buf[1] = {0};
            u_int16_t scID = LibHysA7ParamMng::getInston()->GetRunValueGlobalCurrentSupportID();
            UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(scID),V4APP_STATUS_MANUAL_ACT_HEARTBEAT,buf,0);
            DevEmvdClearRealTimeActList();
        }
        else
        {
            AllZeroPersonLockState = 0;
            SetSystemSelfStatePersonLockStatus(0);
        }
    }
    localLockLast = localLock;
}

int HysSystem::HysSystemPeriodicProc(void)//周期性处理
{
    u_int16_t DelayTime = 10;//ms

    usleep(HYS_SYSTEM_POWERON_TIME_SET*1000);
    while(1)
    {
        usleep(DelayTime*1000);
        ServerReportSystemState(DelayTime);
        SysB12VProcess();
        SysEndProcess(DelayTime);
        sysDevicePersonLock(DelayTime);
        sysZeroPersonLockProc(DelayTime);
        ServerReportPersonState(DelayTime);
//        SysFailProcess();

        /***有问题**/
        RunParamChangedListPeriodicProc();
        SharedMemRefreshProc(DelayTime);
        ExitAddrConfigModeProc(DelayTime);
    }
}

bool HysSystem::SharedMemCreat(void)
{
    TShdData *pShdData = TShdData::getInstance();
    bool ret = pShdData->setDataUnitSize(sizeof(int));  
    ret = pShdData->openSharedData(SHARED_MEM_DATAGRP_NAME,sizeof(HysControllerInfType)/sizeof(int));
    return ret;
}

void HysSystem::SharedMemRefreshProc(u_int16_t DelayTime)
{
    static u_int16_t refreshTime = 0;//ms

    refreshTime += DelayTime;
    if(refreshTime >= HYS_SYSTEM_MERMORY_REFRESH_TIMESET)
       refreshTime = 0;
    else
        return ;
    HysControllerInfType temp;
    GetCtrlInfAll(temp);
    TShdData *pShdData = TShdData::getInstance();
    pShdData->setDataValue(0,sizeof(HysControllerInfType)/sizeof(int),(int *)&temp.BasicInf.Id);
}

/***配址模式退出的特殊处理***/
void HysSystem::ExitAddrConfigModeProc(u_int16_t DelayTime)
{
    static u_int16_t refreshTime = 0;//ms
    u_int16_t SelfStateSlaveMode = GetSystemSelfStateSlaveMode();

    if(SelfStateSlaveMode != SYSMODE_CONFIG)
    {
        refreshTime = 0;
    }
    else
    {
        refreshTime += DelayTime;
        if(refreshTime >= HYS_EXIT_ADDRCONFIGMODE_TIME_SET)
        {
           refreshTime = 0;

            if(SelfStateSlaveMode == SYSMODE_CONFIG)
            {
                SetSystemSelfStateSlaveMode(SYSMODE_NORMAL);
            }
        }
    }
}

/**********/
int HysSystem::RunParamChangedListInit(void)
{
    ParamChangedNeedProcType tab[] =
    {
        {SC_RUNPARAM_ENUM_SINGLE_ACT,   0x00,                       MSG_REPORT_SINGLE_ACTPARAM_CHANGGED,    HysAppMsgSendSingleActParamChanedProc},
        {SC_RUNPARAM_ENUM_COMB_ACT,     0x00,                       MSG_REPORT_COMMB_ACTPARAM_CHANGGED,     HysAppMsgSendCommbActParamChanedProc},
        {SC_RUNPARAM_ENUM_GRP_ACT,      0x00,                       MSG_REPORT_GRP_ACTPARAM_CHANGGED,       HysAppMsgSendGrpActParamChanedProc},

        /****/
        {SC_RUNPARAM_ENUM_GLOBAL,       globalParamEnum_Support,    MSG_REPORT_SERVICE_PARAM_CHANGGED,      HysAppMsgSendServiceParamChanedProc},
        {SC_RUNPARAM_ENUM_GLOBAL,       globalParamEnum_Server,     MSG_REPORT_SERVICE_PARAM_CHANGGED,      HysAppMsgSendServiceParamChanedProc},
        {SC_RUNPARAM_ENUM_GLOBAL,       globalParamEnum_Key,        MSG_REPORT_KEYCONFIG_PARAM_CHANGGED,    HysAppMsgSendKeyConfigParamChanedProc},
        {SC_RUNPARAM_ENUM_GLOBAL,       globalParamEnum_AssistKey,  MSG_REPORT_ASSINTCONFIG_PARAM_CHANGGED, HysAppMsgSendAssintConfigParamChanedProc},
        {SC_RUNPARAM_ENUM_GLOBAL,       globalParamEnum_UIConfig,   MSG_REPORT_HOMEDISPLAY_PARAM_CHANGGED,  HysAppMsgSendHomeDisplayParamChanedProc},
        {SC_RUNPARAM_ENUM_GLOBAL,       globalParamEnum_GroupAssistKey,MSG_REPORT_GRPKEYCONF_PARAM_CHANGGED,HysAppMsgSendGrpKeyConfParamChanedProc},
        {SC_RUNPARAM_ENUM_MAX,          0,                         0}

    };
    m_ParamChangedMutex->lock();
    ParamChangedRestoreList.clear();
    for(int i = 0; i < 20;i++)
    {
        if(tab[i].menuID == SC_RUNPARAM_ENUM_MAX)
            break;
        ParamChangedSetList.append(tab[i]);
    }
    m_ParamChangedMutex->unlock();
    return 0;
}

int HysSystem::InsertRunParamChangedList(u_int16_t menuID,u_int16_t SubMenuId,ParamChangeDetailInfType strlist)
{
    qDebug()<<"************InsertRunParamChangedList************"<<menuID<<SubMenuId;

    that->m_ParamChangedMutex->lock();
    ParamChangedNeedProcType temp;
    temp.menuID = menuID;
    temp.SubMenuId = SubMenuId;
    temp.AppMsgType = 0;
    that->ParamChangedRestoreList.append(temp);
    that->m_ParamChangedMutex->unlock();

    return 0;
}

int HysSystem::RunParamChangedListPeriodicProc(void)
{
    QList<ParamChangedNeedProcType> ParamChangedTemp;

    if(ParamChangedRestoreList.size() == 0)
        return -1;
    /*****/
    ParamChangedTemp.clear();
    m_ParamChangedMutex->lock();
    ParamChangedTemp = ParamChangedRestoreList;
    ParamChangedRestoreList.clear();
    m_ParamChangedMutex->unlock();

    /***消息进行同步***/
    for(int i = 0; i < ParamChangedTemp.size();i++)
    {
        for(int j = 0 ; j < ParamChangedSetList.size();j++)
        {
            if((ParamChangedTemp.at(i).menuID == ParamChangedSetList.at(j).menuID)\
                &&(ParamChangedTemp.at(i).SubMenuId == ParamChangedSetList.at(j).SubMenuId)\
                &&(ParamChangedSetList.at(j).AppMsgType != 0))
            {
                ParamChangedSetList.at(j).msgfunc();
                break;
            }
        }
    }
    /*******************相关运行参数的初始化****************************/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    SetBasicScNumber(pParamMng->GetRunValueGlobalCurrentSupportID());
    SetRunScSelfServerEnabled(pParamMng->GetRunValueGlobalSysServerEnabled());

    /***********************外围设备模型参数重新设置***********************/
    QList<ExtDevDisplayParamType> paramlist;
    QStringList strlist;
    strlist.clear();
    paramlist.clear();
    pParamMng->GetRunExtDevDisplayInf(paramlist,strlist);

    HysScExtDevType extdev;
    HysScExtDevType extdevstatebackup;
    memset(&extdev.Id,0x00,sizeof (HysScExtDevType));
    for(u_int16_t i = 1 ; i <= paramlist.size(); i++)
    {
        GetExtDevState(i,extdevstatebackup);
        memmove(&extdev.Id,&extdevstatebackup.Id,sizeof (HysScExtDevType));

        extdev.Enabled = paramlist.at(i-1).Enabled;
        extdev.DriverId = paramlist.at(i-1).DriverId;
        extdev.DevType = paramlist.at(i-1).DevType;
        extdev.DevId = paramlist.at(i-1).DevId;

        SetExtDevState(i,extdev);
    }

    /***驱动重新初始化***/
    for(int i = 0; i < ParamChangedTemp.size();i++)
    {
        if((ParamChangedTemp.at(i).menuID == SC_RUNPARAM_ENUM_GLOBAL)\
            &&(ParamChangedTemp.at(i).SubMenuId == globalParamEnum_Support))
        {
            DevNetMsgRunParamInit();
            DevPersonMsgRunParamInit();
            DevKeyMsgRunParamInit();
            DevWirelessModuleMsgRunParamInit();
            DevSelfInfredMsgRunParamInit();
            DevBlueTeethMsgRunParamInit();
            DevNfcMsgRunParamInit();
            DevSelfAdMsgRunParamInit();
            DevSelfIoMsgRunParamInit();
        }
        else if((ParamChangedTemp.at(i).menuID == SC_RUNPARAM_ENUM_GLOBAL)\
            &&(ParamChangedTemp.at(i).SubMenuId == globalParamEnum_DevicePeople))
        {
            DevPersonMsgRunParamInit();
        }
    }

    /***声光模型的再次初始化***/
    globalParam_DeviceAlarmType alarmparam;
    pParamMng->GetRunValueGlobalDeviceAlarmParam(alarmparam);
    Alarm::getInston()->AlarmModuleEnable((EnableType)alarmparam.DevAlarmEnable);

    return 0;
}

/*****/
int HysSystem::SystemParamChangedProc(u_int16_t menuID,u_int16_t SubMenuId,ParamChangeDetailInfType strlist)
{
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    SystemParamIpType sysip;
    u_int16_t OldIpAddrHigh = 0;
    u_int16_t OldIpAddrLow = 0;
    u_int16_t NewIpAddrHigh = 0;
    u_int16_t NewIpAddrLow = 0;
    u_int16_t OldScType = 0;
    u_int8_t buf[10];
    bool flag = false;

    qDebug()<<"************SystemParamChangedProc************"<<menuID<<SubMenuId;

    that->GetBasicScType(OldScType);
    pParamMng->GetSystemValueSystemIp(sysip);

    /*****/
    that->GetRunScSelfScIpAddr(OldIpAddrHigh,OldIpAddrLow);
    if(OldScType != sysip.AppArea)
        that->SetBasicScType(sysip.AppArea);
    NewIpAddrHigh = (((sysip.IpAddr1&0x00FF)<<8)|sysip.IpAddr2);
    NewIpAddrLow = (((sysip.IpAddr3&0x00FF)<<8)|sysip.IpAddr4);

    //暂时去掉参数一致的限制。23.08.03
    if(strlist.id == SYSTEM_PARAM_MENU_IP_ADDR_SET)
    {
        that->SetRunScSelfScIpAddr(NewIpAddrHigh,NewIpAddrLow);
        if((OldIpAddrLow&0x00FF) != (NewIpAddrLow&0x00FF))//修改了架号
            DevNetMsgRunParamInit(NewIpAddrLow&0x00FF);

        flag = HysMsgGetAddrConfigResultFlagAndClear();
        if(flag == false)
        {
            buf[0] = static_cast<u_int8_t>(MSG_ADDR_CONFIG_MODE_ENTER&0x00FF);
            buf[1] = static_cast<u_int8_t>((MSG_ADDR_CONFIG_MODE_ENTER&0xFF00)>>8);
            buf[2] = static_cast<u_int8_t>(MSG_ADDR_CONFIG_RESULT_INVALID&0x00F);
            buf[3] = static_cast<u_int8_t>((MSG_ADDR_CONFIG_RESULT_INVALID&0xFF00)>>8);
            HysMsgTxEnterSetIpConfigProc(DRIVER_NET,sysip.AppArea,V4APP_DEV_TYPE_SC,\
                                   pParamMng->GetRunValueGlobalCurrentSupportID(),buf,4);
            that->SetSystemSelfStateSlaveMode(SYSMODE_CONFIG);
        }
    }
    return 0;
}
