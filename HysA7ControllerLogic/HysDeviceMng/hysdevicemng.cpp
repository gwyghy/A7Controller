#include "hysdevicemng.h"
#include "devicemngapi.h"
#include <unistd.h>
#include "hysdevmsg.h"
#include "devperson.h"
#include "devblueteeth.h"
#include "devwirelessmodule.h"
#include "devnfc.h"
#include "hysdevmsg.h"
#include "v4hysapp.h"
#include "./HysSystem/hyssystem.h"

/****/
#define     HYS_DEV_MANAGER_COMMLED_TIMESET      3000//对通讯控制指示灯的控制
#define     HYS_EMVD_ACTING_HEARTBEAT_TIMESET    50//

/****/
HysDeviceMng *HysDeviceMng::that = NULL;

/*******/
HysDeviceMng::HysDeviceMng()
{
    m_AppRecvMutex = new QMutex();
    m_DevRecvMutex = new QMutex();
}

HysDeviceMng::~HysDeviceMng()
{
    delete m_AppRecvMutex;
    delete m_DevRecvMutex;
}

HysDeviceMng *HysDeviceMng::getInston()
{
    if( that == NULL)
    {
        that = new HysDeviceMng();
    }
    return that;
}

void HysDeviceMng::Init(void)
{
    HysDeviceMngInit();
    HysDeviceStatusInit();
}

void HysDeviceMng::HysDeviceMngInit(void)
{
    /*****/
    DevCtrlInit();
    /*****/
    DevGeneralInit();
    DevAlaramInit();
    DevEmvdInit();
    DevPersonInit();
    DevSelfAdInit();
    DevSelfInfredInit();
    DevSelfIoInit();
    DevKeyInit();
    DevWirelessModuleInit();
    DevBlueTeethInit();
    DevNfcInit();
    /****/
    GenerateDataProTab();

    /****/
    DeviceMngApi * pDevApi = DeviceMngApi::getInston();
    int ret = pDevApi->registeappid(LOGIC_PROGRAM_ID,1);//？？？？？？？？？？？？？？？？？？？？？？？？？
    if(ret <0)
    {
       qDebug()<<"HysControllerlogic DevMng init error###,Init device error:"<<ret;
    }
    pDevApi->setCallBack_R_Shm(UDP_PROGRAM_ID,AppRecvShmCallback);
    pDevApi->setCallBack_R_Msg(UDP_PROGRAM_ID,RecvMsgCallback);

    pDevApi->setCallBack_R_Shm(CAN_PROGRAM_ID,DevCanRecvShmCallback);
    pDevApi->setCallBack_R_Msg(CAN_PROGRAM_ID,RecvMsgCallback);

    pDevApi->setCallBack_R_Shm(SELF_AD_PROGRAM_ID,DevSelfAdRecvShmCallback);
    pDevApi->setCallBack_R_Msg(SELF_AD_PROGRAM_ID,RecvMsgCallback);

    pDevApi->setCallBack_R_Shm(SELF_IO_PROGRAM_ID,DevSelfIoRecvShmCallback);
    pDevApi->setCallBack_R_Msg(SELF_IO_PROGRAM_ID,RecvMsgCallback);

    pDevApi->setCallBack_R_Shm(SELF_UART_PROGRAM_ID,DevUartRecvShmCallback);
    pDevApi->setCallBack_R_Msg(SELF_UART_PROGRAM_ID,RecvMsgCallback);

    #if 0//暂时不存在此程序
        pDevApi->setCallBack_R_Shm(SELF_I2C_PROGRAM_ID,DevI2CRecvShmCallback);
        pDevApi->setCallBack_R_Msg(SELF_I2C_PROGRAM_ID,RecvMsgCallback);
    #endif
}

void HysDeviceMng::HysDeviceStatusInit(void)
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

/*******************************************************************/
int HysDeviceMng::GenerateDataProTab(void)
{
    /*********************************************************/
    devshmproctab.clear();
    devshmproctab.insert(EXTDEV_TYPE_EMVD,      DevEmvdFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_ANGLE,     DevGeneralFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_HIGH,      DevGeneralFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_ALARM,     DevAlaramFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_PERSON,    DevPersonFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_IRRX,      DevSelfInfredFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_KEYBOARD,  DevGeneralFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_KEYBOARD,      DevKeyFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_WIRELESS,  DevWirelessModuleFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_HUB_1,     DevGeneralFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_HUB_2,     DevGeneralFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_HUB_3,     DevGeneralFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_ANGLE_HEIGHT,  DevGeneralFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_MULTI_SOUND,   DevGeneralFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_SELF_AD,   DevSelfAdFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_SELF_IO,   DevSelfIoFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_SELF_INFRED,   DevSelfInfredFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_BLUE_TEETH,    DevBlueTeethFrameProc);
    devshmproctab.insert(EXTDEV_TYPE_NFC,           DevNfcFrameProc);

    /*********************************************************/
    appshmproctab.clear();
    /******/
    appshmproctab.insert(V4APP_STATUS_GET_SINGLE_INQUIRE,   FrameGetSingleInquireProc);
    appshmproctab.insert(V4APP_STATUS_GET_MULTIPLE_INQUIRE, FrameGetMultiInquireProc);
    appshmproctab.insert(V4APP_STATUS_REPORT,               FrameStatusReportProc);
    appshmproctab.insert(V4APP_STATUS_CTRL_DEV,             FrameCtrlDevProc);
    appshmproctab.insert(V4APP_STATUS_MANUAL_ACT_HEARTBEAT, FrameManulHeartBeatProc);
    appshmproctab.insert(V4APP_STATUS_DOWNLOAD_SYS_STATUS,  FrameDownloadSysStatusProc);
    appshmproctab.insert(V4APP_FOLLOWON_RESPONSE,           FrameFollowOnResponseProc);
    /*appshmproctab.insert(V4APP_AUTOBACKFLUSH_ONOFF_INQUIRE, FrameDownloadAutoBackFlushOnOffProc);*/
    appshmproctab.insert(V4APP_AUTOBACKFLUSH_ONOFF_STATUS_DOWNLOAD,FrameDownloadAutoBackFlushStatusProc);
    appshmproctab.insert(V4APP_RECV_YKQ_DECODE,             FrameDownloadAutoBackFlushStatusProc);
    appshmproctab.insert(V4APP_TRANDEVSTATUS_INQUIRE,       FrameTranDevStatusProc);
    /*appshmproctab.insert(V4APP_TRANDEVSTATUS_INQUIRE,       FrameTerminalInfReportProc);*/
    appshmproctab.insert(V4APP_TERMINAL_INF_REPORT,         FrameTerminalInfReportProc);

    /******/
    appshmproctab.insert(V4APP_PARAM_GET_SINGLE_INQUIRE,    FrameParamGetSingleResponseProc);
    appshmproctab.insert(V4APP_PARAM_GET_MULTIPLE_INQUIRE,  FrameParamGetMultiResponseProc);
    appshmproctab.insert(V4APP_PARAM_SET_SINGLE_INQUIRE,    FrameParamSetSingleInquireProc);
    appshmproctab.insert(V4APP_PARAM_SET_MULTIPLE_INQUIRE,  FrameParamSetMultiInquireProc);
    appshmproctab.insert(V4APP_PARAM_CHECK_INQUIRE,         FrameParamCheckInquireProc);
    appshmproctab.insert(V4APP_PARAM_SET_REPORT,            FrameParamNoticeProc);
    appshmproctab.insert(V4APP_SERVER_OPERRATE_REQUEST,     FrameParamServerOperateRequestProc);
    appshmproctab.insert(V4APP_DISPLAY_SS_PARAM_RESPONSE,   FrameParamSSDisplayResponseProc);
    //appshmproctab.insert(V4APP_REPORTSS_PARAM_CHANGGED,     FrameParamReportParamChangedProc);
    appshmproctab.insert(V4APP_REPORTSS_PARAM_CHANGGED_RESPONSE,FrameParamReportParamChangedAckProc);
    appshmproctab.insert(V4APP_SET_LASCFINDLINE,            FrameParamSetLascDataProc);
    appshmproctab.insert(V4APP_SYSPARAM_SYNC_INQUIRE,       FrameParamSystemParamInquireProc);
    appshmproctab.insert(V4APP_FORCE_ENTER_SET_IP_STATUS_INQUIRE,       FrameForceEnterSetIpStatusProc);
    appshmproctab.insert(V4APP_REPORTSS_SYSTEM_CHANGGED,    FrameParamReportSystemParamChangedProc);

    /**201之后，为网络维护。**/
    appshmproctab.insert(V4APP_RSYNC_FILE_INQUIRE,          FrameNetRsyncFileInquireProc);

    /*********************************************************/
    msgproctab.clear();
    msgproctab.insert(MSG_DRIVER_STATUS, HysMsgRxDriverStatus);
    msgproctab.insert(MSG_TRAN_FILE_ACK, HysMsgRxTransFileStatus);
    msgproctab.insert(MSG_RECV_FILE_OK, HysMsgRxRecvFileOkStatus);
    msgproctab.insert(MSG_RECV_FILE_ACK, HysMsgRxRecvFileIngDetailStatus);
    msgproctab.insert(MSG_REPORT_ADDR_CONFIG_MODE,HysMsgRxReportAddrConfigMode);
    msgproctab.insert(MSG_REPORT_ADDR_CONFIG_RESULT,HysMsgRxReportAddrConfigResult);
    msgproctab.insert(MSG_REPORT_ERR_STATUS,HysMsgRxReportErrStatus);
    msgproctab.insert(MSG_REPORT_COMM_AREA_CHECK_RESULT,HysMsgRxCommAreaCheckResult);
    msgproctab.insert(MSG_REPORT_RSYNC_FILE_CHECK_RESULT,HysMsgRsyncFileCheckResult);
    return 0;
}

/******/
int HysDeviceMng::AppRecvShmCallback(uchar childdevtype,uchar devtype,ushort numb,ushort frametype,uchar*data,int len)
{
    /**特殊处理***/
    devtype |= (numb&0x100)>>8;//暂时进行特殊处理。X与实际对应不上。
    numb &= 0xFF;//暂时进行特殊处理。X与实际对应不上。

    /**进行数据存储***/
    if(len > 1024)//此处可能存在丢数据的风险
        return -1;

    HysSystemRecvShmClass *precv = new HysSystemRecvShmClass();
    precv->childdevtype = childdevtype;
    precv->devtype = devtype;
    precv->numb = numb;
    precv->frametype = frametype;
    memmove(precv->data,data,len);
    precv->len = len;

    that->m_AppRecvMutex->lock();
    that->AppRecvDataList.append(precv);
    that->m_AppRecvMutex->unlock();

    /***for test**/
    that->AppShmRecvTotalumber++;
    return 0;
}

int HysDeviceMng::DevRecvShmProc(uchar drivertype,uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len)
{
//    if(frametype == V4DRIVER_REPORT_EXTDEV_STATUS)
//        qDebug()<<"DevRecvShmProcr###"<<drivertype<<devid<<childdevid<<point<<frametype<<len<<*data;

    uchar tempDriverNumber = ((devid&0xE0)>>5);//硬件设备的具体编号
    uchar tempDevType = (devid&0x1F);//设备类型

    /**进行数据存储***/
    if(len > 50)//此处可能存在丢数据的风险
        return -1;

    HysDevRecvShmClass *precv = new HysDevRecvShmClass();
    precv->driverid = drivertype+tempDriverNumber-1;
    precv->devid = tempDevType;
    precv->childdevid = childdevid;
    precv->point = point;
    precv->frametype = frametype;
    memmove(precv->data,data,len);
    precv->len = len;

    that->m_DevRecvMutex->lock();
    that->DevRecvDataList.append(precv);
    that->m_DevRecvMutex->unlock();

    /***for test**/
    that->DevShmRecvTotalumber++;
    return 0;
}

int HysDeviceMng::DevCanRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len)
{
    return that->DevRecvShmProc(DRIVER_CAN1,devid,childdevid,point,frametype,data,len);
}

int HysDeviceMng::DevSelfAdRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len)
{
    //qDebug()<<"###############DevSelfAdRecvShmCallback############"<<devid<<childdevid<<point<<frametype<<len<<*data<<*(data+1);

    return that->DevRecvShmProc(DRIVER_AD,devid,childdevid,point,frametype,data,len);
}

int HysDeviceMng::DevSelfIoRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len)
{
    return that->DevRecvShmProc(DRIVER_IO,devid,childdevid,point,frametype,data,len);
}

int HysDeviceMng::DevUartRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len)
{
    return that->DevRecvShmProc(DRIVER_UART2,devid,childdevid,point,frametype,data,len);
}

int HysDeviceMng::DevI2CRecvShmCallback(uchar devid,uchar childdevid,ushort point,ushort frametype,uchar*data,int len)
{
    return 0;
    //return that->DevRecvShmProc(DRIVER_I2C,devid,childdevid,point,frametype,data,len);
}

int HysDeviceMng::RecvMsgCallback(uint8_t childdevtype,uint8_t devtype,ushort numb,uchar *data,int len)
{
//    qDebug()<<"++++++logic recv driver msg "<<childdevtype<<devtype<<numb<<(data[0]|(data[1]<<8))<<len;
    if(devtype > V4APP_DEV_TYPE_MAX)
        return -1;

    /**可能会数据无法对齐****/
    u_int16_t frametype = (data[0]|(data[1]<<8));

    if(frametype > MSG_APP_MAX)
        return -1;

    if(that->msgproctab.contains(frametype))
        return that->msgproctab[frametype](childdevtype,devtype,numb,data+4,len-4);
}

int HysDeviceMng::HysSystemAppRecvShmProc(void)
{
    while(1)
    {
        usleep(10*1000);
      //qDebug()<<"*************HysSystemAppRecvShmProc,**********";
        QList<HysSystemRecvShmClass *> recvDataList_temp;
        m_AppRecvMutex->lock();
        recvDataList_temp.append(AppRecvDataList);
        AppRecvDataList.clear();
        m_AppRecvMutex->unlock();

        if(recvDataList_temp.size() == 0)
            continue;

        /***for test**/
        AppShmHaveProcTotalumber += recvDataList_temp.size();

        foreach (HysSystemRecvShmClass *item, recvDataList_temp)
        {
            if(recvDataList_temp.size() == 0)
                break;
            if(that->appshmproctab.contains(item->frametype))
                that->appshmproctab[item->frametype](item->childdevtype,item->devtype,item->numb,\
                                                  item->frametype, item->data,item->len);

            recvDataList_temp.removeOne(item);
            delete item;
            item = NULL;
        }
    }
}


int HysDeviceMng::HysSystemDevRecvShmProc(void)
{
    QList<HysDevRecvShmClass *> recvDataList_temp;
    m_DevRecvMutex->lock();
    recvDataList_temp.append(DevRecvDataList);
    DevRecvDataList.clear();
    m_DevRecvMutex->unlock();

    if(recvDataList_temp.size() == 0)
        return 0;

    /***for test**/
    DevShmHaveProcTotalumber += recvDataList_temp.size();

    foreach (HysDevRecvShmClass *item, recvDataList_temp)
    {
        if(recvDataList_temp.size() == 0)
            break;

        if(that->devshmproctab.contains(item->devid))
            that->devshmproctab[item->devid](item->driverid,item->devid,item->childdevid,item->point,\
                                              item->frametype, item->data,item->len);

        recvDataList_temp.removeOne(item);
        delete item;
        item = nullptr;
    }
    return 0;
}

int HysDeviceMng::HysSystemDevCommLedStatusProc(int delay)
{
    static int ledctrltimer = 0;
    static int InfredGetCommLedStatus = DEV_POINT_CTRL_OFF;
    static int WirelessSetCommLedStatus = DEV_POINT_CTRL_OFF;

    /*****/
    if(ledctrltimer < HYS_DEV_MANAGER_COMMLED_TIMESET)
    {
        ledctrltimer += delay;
        return -1;
    }
    else
    {
        ledctrltimer = 0;

        u_int16_t CtrlFlag = HysSystem::getInston()->GetRunScCtrlScWirelessCtrlFlag();
        /***查找此时是否已经空闲或解除对码的状态***/
        if((CtrlFlag != WIRELESS_MODULE_CODE_MATCH_NONE)\
            &&(CtrlFlag != WIRELESS_MODULE_CODE_MATCH_UNMATCED))
            return -1;
        if(ledctrltimer >= HYS_DEV_MANAGER_COMMLED_TIMESET)
        {
            if(((DevSelfInfredGetCommLedStatus() == DEV_POINT_CTRL_ON)\
                &&(InfredGetCommLedStatus == DEV_POINT_CTRL_ON))\
                ||((DevWirelessModuleGetCommLedStatus() == DEV_POINT_CTRL_ON)\
                    &&(WirelessSetCommLedStatus == DEV_POINT_CTRL_ON)))
            {
                DevSelfInfredSetCommLedStatus(DEV_POINT_CTRL_OFF);
                DevWirelessModuleSetCommLedStatus(DEV_POINT_CTRL_OFF);
            }
            InfredGetCommLedStatus = DevSelfInfredGetCommLedStatus();
            WirelessSetCommLedStatus = DevWirelessModuleGetCommLedStatus();
        }
    }
    return 0;
}

int HysDeviceMng::HysSystemEmvdActHeartbeatProc(int delay)
{
    static int actctrltimer = 0;

    /*****/
    if(actctrltimer < HYS_EMVD_ACTING_HEARTBEAT_TIMESET)
        actctrltimer += delay;
    else
        actctrltimer = 0;

    if(actctrltimer >= HYS_EMVD_ACTING_HEARTBEAT_TIMESET)
    {
        QList<u_int16_t> list;
        list.clear();

        if(DevEmvdGetRealTimeActList(list) == -1)
            return -1;

        u_int8_t buf[100];
        u_int8_t length = 0;

        for(int i = 0; i < list.size()&& i < sizeof (buf); i++)
        {
            if(list.at(i) > DEV_POINT_32)
                break;
            buf[length++] = list.at(i);
            buf[length++] = 0;
        }

        if(length != 0)
        {
            DeviceMngApi *pDevApi = DeviceMngApi::getInston();
            pDevApi->writeshmp(CAN_PROGRAM_ID,HARDWARE_NUMBER_2|EXTDEV_TYPE_EMVD,\
                               DEV_CHILDDEV_1,0,V4DRIVER_STATUS_MANUAL_ACT_HEARTBEAT,buf,length);
        }
    }
    return 0;
}

int HysDeviceMng::HysSystemDevPeriodicProc(void)
{
    while(1)
    {
        usleep(10*1000);
        HysSystemDevRecvShmProc();
        HysSystemDevCommLedStatusProc(10);
        HysSystemEmvdActHeartbeatProc(10);
    }
}
