#include "hyscontrollerlogic.h"
#include <QDateTime>

/**************/
HysControlerLogic::HysControlerLogic(u_int16_t id,u_int16_t type)
{
    m_basic_locker = new QReadWriteLock();
    m_part_locker = new QReadWriteLock();
    m_sensor_locker = new QReadWriteLock();
    m_extdev_locker = new QReadWriteLock();
    m_system_locker = new QReadWriteLock();
    m_run_locker = new QReadWriteLock();
    m_log_locker = new QReadWriteLock();
    Init(id,type);
}

HysControlerLogic::~HysControlerLogic()
{
    delete m_basic_locker;
    delete m_part_locker;
    delete m_sensor_locker;
    delete m_extdev_locker;
    delete m_system_locker;
    delete m_run_locker;
    delete m_log_locker;
}

/*********************/
void HysControlerLogic::Init(u_int16_t id,u_int16_t type)
{
    int i = 0x00;

    memset((u_int8_t *)&CtrlInf.BasicInf.Id,0x00,sizeof(CtrlInf));//待定
    memset((u_int8_t *)&ActRefList,0xFFFF,sizeof(ActRefList));
    CtrlInf.BasicInf.Id = id;
    CtrlInf.BasicInf.Type = type;

    /****/
    for(i = 0x00; i< HYS_PART_NUMB_MAX;i++)
    {
        CtrlInf.Part[i].PartInf.Id = 0;
    }

    for(i = 0x00; i< HYS_SENSOR_NUMB_MAX;i++)
    {
        CtrlInf.Sensor[i].Id = 0;
        CtrlInf.Sensor[i].OnLineStatus = false;
        CtrlInf.Sensor[i].FaultStatus = false;
    }

    for(i = 0x00; i< HYS_EXTDEV_NUB_MAX;i++)
    {
        CtrlInf.Extdev[i].Id = 0;
        CtrlInf.Extdev[i].Enabled = false;
        CtrlInf.Extdev[i].Status = false;
    }

    memset((u_int8_t *)&CtrlInf.SystemInf.ScSelfStatus.StopStatus,0x00,sizeof(HysScSystemInfType));
    memset((u_int8_t *)&CtrlInf.RunInf.ScRunInf.IpAddrHigh,0x00,sizeof(HysDykRunInfType));
    memset((u_int8_t *)&CtrlInf.LogInf.log,0x00,sizeof(HysDykLogInfType));

    /***特殊处理的地方***/
    SetSystemSelfStateLeftCommErr(1);
    SetSystemSelfStateRightCommErr(1);
    SetSystemSelfStateLeftGlobalParamSame(1);
    SetSystemSelfStateRightGlobalParamSame(1);

    HysRunCommArea commarea;
    commarea.StartScId = 0;
    commarea.EndScId = 0;
    SetRunCommAreaState(commarea);
}

bool HysControlerLogic::ParamInit(u_int16_t CtrlId,u_int16_t Ctrltype)
{
    if((CtrlId < HYS_CTRL_ID_MIN)||(CtrlId > HYS_CTRL_ID_MAX))
        return false;
    if((Ctrltype < HYS_CTRL_TYPE_MIN)||(Ctrltype > HYS_CTRL_TYPE_MAX))
        return false;

    QWriteLocker lock(m_basic_locker);
    /****/
    CtrlInf.BasicInf.Id = CtrlId;
    CtrlInf.BasicInf.Type = Ctrltype;
    return true;
}

bool HysControlerLogic::PartInfInit(QList<PartActSensorRealatedType> list)
{
    if(list.size() == 0)
        return false;

    QWriteLocker lock(m_part_locker);
    for(u_int16_t i = 0x00; i< HYS_PART_NUMB_MAX && i < list.size();i++)
    {
        if(list.at(i).Id != 0)
        {
            CtrlInf.Part[i].PartInf.Id = list.at(i).Id;
            CtrlInf.Part[i].Act[0].Id = list.at(i).Act1_Id;
            CtrlInf.Part[i].Act[1].Id = list.at(i).Act2_Id;
            ActRefList[list.at(i).Act1_Id] = i;//
            ActRefList[list.at(i).Act2_Id] = i;//
            CtrlInf.Part[i].SensorInf.SensorId[0] = list.at(i).Sensor1_Id;
            CtrlInf.Part[i].SensorInf.SensorId[1] = list.at(i).Sensor2_Id;
            CtrlInf.Part[i].SensorInf.SensorId[2] = list.at(i).Sensor3_Id;
            CtrlInf.Part[i].SensorInf.SensorId[3] = list.at(i).Sensor4_Id;
        }
    }
    return true;
}

bool HysControlerLogic::SensorInfInit(QList<SensorDisplayParamType> sensorlist)
{
    if(sensorlist.size() == 0)
        return false;

    int i = 0x00;

    QWriteLocker lock(m_sensor_locker);
    for(i = 0x00; i< HYS_SENSOR_NUMB_MAX && i < sensorlist.size();i++)
    {
        if(sensorlist.at(i).Id != 0)
        {
            CtrlInf.Sensor[i].Id = sensorlist.at(i).Id;
        }
    }
    return true;
}

/*************************/
bool HysControlerLogic::SetBasicScNumber(u_int16_t scid)
{
    if((scid < HYS_CTRL_ID_MIN)||(scid > HYS_CTRL_ID_MAX))
        return false;

    QWriteLocker lock(m_basic_locker);
    CtrlInf.BasicInf.Id = scid;
    return true;
}

bool HysControlerLogic::SetBasicScType(u_int16_t sctype)
{
    if((sctype < HYS_CTRL_TYPE_MIN)||(sctype > HYS_CTRL_TYPE_MAX))
        return false;

    QWriteLocker lock(m_basic_locker);
    CtrlInf.BasicInf.Type = sctype;
    return true;
}

bool HysControlerLogic::GetBasicScNumber(u_int16_t &scid)
{
    QReadLocker lock(m_basic_locker);
    scid = CtrlInf.BasicInf.Id;
    return true;
}

bool HysControlerLogic::GetBasicScType(u_int16_t &sctype)
{
    QReadLocker lock(m_basic_locker);
    sctype = CtrlInf.BasicInf.Type;
    return true;
}

/*************************/
bool HysControlerLogic::SetPartState(u_int16_t part_id,u_int16_t part_state)
{
    if((part_id > HYS_PART_NUMB_MAX)\
        ||(part_state >= PART_STATUS_MAX)\
        ||(part_state == PART_STATUS_INVALID))
    {
        return false;
    }

    QWriteLocker lock(m_part_locker);
    CtrlInf.Part[part_id-1].PartInf.partState = part_state;

    return true;
}

bool HysControlerLogic::SetSingleActState(u_int16_t id,HysScSingleActType actstate)
{
    if((actstate.Id > HYS_SINGLEACT_NUMB_MAX)||(actstate.Id == 0))
         return false;

    QWriteLocker lock(m_part_locker);
    int num = 2;
    if(CtrlInf.Part[ActRefList[actstate.Id]].Act[0].Id == actstate.Id)
        num = 0;
    else if(CtrlInf.Part[ActRefList[actstate.Id]].Act[1].Id == actstate.Id)
       num = 1;

    if(num < 2)
        CtrlInf.Part[ActRefList[actstate.Id]].Act[num] = actstate;

    return true;
}

bool HysControlerLogic::GetPartState(u_int16_t part_id,u_int16_t &part_state)
{
    uint i = 0x00;
    int temp = -1;

    if(part_id > HYS_PART_NUMB_MAX)
    {
        return false;
    }

    QReadLocker lock(m_part_locker);
    part_state = CtrlInf.Part[part_id-1].PartInf.partState;

    return temp;
}

bool HysControlerLogic::GetSingleActState(u_int16_t act_id, HysScSingleActType &actstate)
{
    if(act_id > HYS_SINGLEACT_NUMB_MAX)
         return false;

    if(ActRefList[act_id] == 0xFFFF)
        return false;

    QReadLocker lock(m_part_locker);
    if(CtrlInf.Part[ActRefList[act_id]].Act[0].Id == act_id)
        actstate = CtrlInf.Part[ActRefList[act_id]].Act[0];
    else if(CtrlInf.Part[ActRefList[act_id]].Act[1].Id == act_id)
        actstate = CtrlInf.Part[ActRefList[act_id]].Act[1];
    else
        ;

    return true;
}

int HysControlerLogic::GetPartIdMinMax(int &min,int &max)
{
    min = HYS_PART_NUMB_MAX;
    max = 0;
    for(int i = 0x00; i< HYS_PART_NUMB_MAX;i++)
    {
        if(CtrlInf.Part[i].PartInf.Id != 0)
        {
            if(CtrlInf.Part[i].PartInf.Id > max)
                max = CtrlInf.Part[i].PartInf.Id;
            if(CtrlInf.Part[i].PartInf.Id < min)
                min = CtrlInf.Part[i].PartInf.Id;
        }
        else
            break;
    }
    return 0;
}

int HysControlerLogic::GetActIdMinMax(int &min, int &max)
{
    min = HYS_SINGLEACT_NUMB_MAX;
    max = 0;
    for(int i = 0x00; i< HYS_PART_NUMB_MAX;i++)
    {
        if(CtrlInf.Part[i].PartInf.Id != 0)
        {
            if(CtrlInf.Part[i].Act[0].Id > max)
                max = CtrlInf.Part[i].Act[0].Id;
            if(CtrlInf.Part[i].Act[1].Id > max)
                max = CtrlInf.Part[i].Act[1].Id;

            if((CtrlInf.Part[i].Act[0].Id < min)&&(CtrlInf.Part[i].Act[0].Id != 0))
                min = CtrlInf.Part[i].Act[0].Id;
            if((CtrlInf.Part[i].Act[1].Id < min)&&(CtrlInf.Part[i].Act[1].Id != 0))
                min = CtrlInf.Part[i].Act[1].Id;
        }
        else
            break;
    }
    return 0;
}

/*************************/
bool HysControlerLogic::SetSensorState(u_int16_t sensor_id, HysScSensorType sensorstate)
{
    if(sensor_id > HYS_SENSOR_NUMB_MAX)
         return false;

    QWriteLocker lock(m_sensor_locker);
    CtrlInf.Sensor[sensor_id-1] = sensorstate;

    return true;
}

bool HysControlerLogic::GetSensorState(u_int16_t sensor_id, HysScSensorType &sensorstate)
{
    if(sensor_id > HYS_SENSOR_NUMB_MAX)
         return false;

    QReadLocker lock(m_sensor_locker);
    sensorstate = CtrlInf.Sensor[sensor_id-1];

    return true;
}

HysScSensorType * HysControlerLogic::GetSensorState(u_int16_t sensor_id)
{
    if(sensor_id > HYS_SENSOR_NUMB_MAX)
         return NULL;

    QReadLocker lock(m_sensor_locker);
    return &CtrlInf.Sensor[sensor_id-1];
}

/************************/
HysScSingleActType * HysControlerLogic::GetSingleActState(u_int16_t singleact_id)
{
    if((singleact_id > HYS_SINGLEACT_NUMB_MAX)||(singleact_id == 0))
         return NULL;

    bool ret = false;
    QReadLocker lock(m_sensor_locker);

    for(u_int16_t i = 0; i < HYS_PART_NUMB_MAX;i++)
    {
        if(CtrlInf.Part[i].Act[0].Id == singleact_id)
        {
            return  &CtrlInf.Part[i].Act[0];
        }
        else if (CtrlInf.Part[i].Act[1].Id == singleact_id)
        {
            return  &CtrlInf.Part[i].Act[1];
        }
    }

    return NULL;
}

HysPartStateType * HysControlerLogic::GetPartState(u_int16_t part_id)
{
    if((part_id > HYS_PART_NUMB_MAX)||(part_id == 0))
         return NULL;

    QReadLocker lock(m_sensor_locker);
    return &CtrlInf.Part[part_id-1].PartInf;
}

/*!
 * \brief HysControlerLogic::GetAllSensorStateDataToReport，获取所有传感器的状态数据，用于上报给服务器
 * \param pBuf,数据指针
 * \return，数据的总长度
 */
uint HysControlerLogic::GetAllSensorStateDataToReport(u_int8_t *pBuf)
{
    u8 i;
    u8 sensorIndex = 0;
    for(i = 0;i<SENSOR_MAX;i++)
    {
        if(CtrlInf.Sensor[i].Id == 0)
            break;
        memcpy(pBuf+sensorIndex,&CtrlInf.Sensor[i].Id,sizeof(HysScSensorType));
        sensorIndex += sizeof(HysScSensorType);
    }
    return sensorIndex;
}

int HysControlerLogic::GetSensorIdMinMax(int &min, int &max)
{
    min = HYS_SENSOR_NUMB_MAX;
    max = 0;
    for(int i = 0x00; i< HYS_SENSOR_NUMB_MAX;i++)
    {
        if(CtrlInf.Sensor[i].Id == 0)
            break;
        if(CtrlInf.Sensor[i].Id > max)
        {
            max = CtrlInf.Sensor[i].Id;
        }
        if(CtrlInf.Sensor[i].Id < min)
        {
            min = CtrlInf.Sensor[i].Id;
        }
    }

    return 0;
}

/*************************/
bool HysControlerLogic::SetExtDevEnabled(u_int16_t extdev_id,u_int16_t enable)
{
    if(extdev_id> HYS_EXTDEV_NUB_MAX)
         return false;

    QWriteLocker lock(m_extdev_locker);
    CtrlInf.Extdev[extdev_id-1].Enabled = enable;

    return true;
}

bool HysControlerLogic::SetExtDevState(u_int16_t extdev_id,HysScExtDevType extdevstate)
{
    if((extdevstate.Id > HYS_EXTDEV_NUB_MAX) || (extdevstate.Id == 0))
         return false;

    QWriteLocker lock(m_extdev_locker);
    CtrlInf.Extdev[extdevstate.Id-1] = extdevstate;

    return true;
}

bool HysControlerLogic::SetExtDevState(u_int16_t extdev_id,DriverExtDevStatusType driverdevstate)
{
    if((extdev_id > HYS_EXTDEV_NUB_MAX) || (extdev_id == 0))
         return false;

    QWriteLocker lock(m_extdev_locker);
    memmove(&CtrlInf.Extdev[extdev_id-1].Status, &driverdevstate.Status, sizeof (DriverExtDevStatusType));
    qDebug()<<"*****HysControlerLogic::SetExtDevState**state*version***"<<extdev_id<<driverdevstate.Status<<driverdevstate.AppVersionHigh<<driverdevstate.AppVersionLow\
           <<driverdevstate.BootVersionHigh<<driverdevstate.BootVersionLow;
    return true;
}

bool HysControlerLogic::SetExtDevState(u_int16_t driverid, u_int16_t devid, u_int16_t childdevid,\
                                       DriverExtDevStatusType driverdevstate)
{
     QWriteLocker lock(m_extdev_locker);
     for(int i = 0; i < HYS_EXTDEV_NUB_MAX;i++)
     {
        if((CtrlInf.Extdev[i].DriverId == driverid)\
          &&(CtrlInf.Extdev[i].DevType = devid)\
          &&(CtrlInf.Extdev[i].DevId = childdevid))
        {
            memmove(&CtrlInf.Extdev[i].Status, &driverdevstate.Status, sizeof (DriverExtDevStatusType));
        }
     }

     return true;
}


bool HysControlerLogic::GetExtDevState(u_int16_t extdev_id,HysScExtDevType &extdevstate)
{
    if(extdev_id > HYS_EXTDEV_NUB_MAX)
         return false;

    QReadLocker lock(m_extdev_locker);
    extdevstate = CtrlInf.Extdev[extdev_id-1];

    return true;
}

/***存在多个传感器绑定同一外围设备的情况***/
bool HysControlerLogic::GetExtDevState(u_int16_t driverid, u_int16_t devid,u_int16_t childdevid, QList<u_int16_t> &extdevid_list,QList<u_int16_t> &Status_list)
{
    bool ret = false;

    extdevid_list.clear();
    Status_list.clear();
    QReadLocker lock(m_extdev_locker);
    for(u_int16_t i = 0; i < HYS_EXTDEV_NUB_MAX;i++)
    {
        if(CtrlInf.Extdev[i].DevId == 0)
            break;

        if((CtrlInf.Extdev[i].DriverId == driverid)\
            &&(CtrlInf.Extdev[i].DevType == devid)\
            &&(CtrlInf.Extdev[i].DevId == childdevid))
        {
            extdevid_list.append(CtrlInf.Extdev[i].Id);
            Status_list.append(CtrlInf.Extdev[i].Status);
            ret = true;
            //break;/***存在多个传感器绑定同一外围设备的情况***/
        }
    }

    return ret;
}

int HysControlerLogic::GetExtDevIdMinMax(int &min, int &max)
{
    QReadLocker lock(m_extdev_locker);
    min = HYS_EXTDEV_NUB_MAX;
    max = 0;
    for(int i = 0x00; i< HYS_EXTDEV_NUB_MAX ;i++)
    {
        if(CtrlInf.Extdev[i].Id == 0)
            break;
        if(CtrlInf.Extdev[i].Id > max)
        {
            max = CtrlInf.Extdev[i].Id;
        }
        if(CtrlInf.Extdev[i].Id < min)
        {
            min = CtrlInf.Extdev[i].Id;
        }
    }

    return 0;
}

/************系统状态中相关访问接口函数**********/
HYS_FAULT_TIME GetSystemFaultTime(void)
{
    QDateTime time;
    time = QDateTime::currentDateTime();

    HYS_FAULT_TIME faulttime;
    faulttime.Year = time.date().year()-2000;
    faulttime.Month = time.date().month();
    faulttime.Day = time.date().day();
    faulttime.Hour = time.time().hour();
    faulttime.Minite = time.time().minute();
    faulttime.Seccond = time.time().second();
    return faulttime;
}

bool HysControlerLogic::SetSystemSelfState(u_int16_t offset,u_int16_t data)//自身状态
{
    /**正常数据上传的部分***/
    if(offset > sizeof(CtrlInf.SystemInf.ScSelfStatus))
         return false;

    //故障发生或解除
    if(data == *(u_int16_t *)(&CtrlInf.SystemInf.ScSelfStatus.StopStatus+offset))
        return false;

    QDateTime time;
    time = QDateTime::currentDateTime();

    HYS_FAULT_TIME faulttime;
    faulttime.Year = time.date().year();
    faulttime.Month = time.date().month();
    faulttime.Day = time.date().day();
    faulttime.Hour = time.time().hour();
    faulttime.Minite = time.time().minute();
    faulttime.Seccond = time.time().second();

    QWriteLocker lock(m_system_locker);

    memmove((u_int8_t *)&CtrlInf.SystemInf.ScSelfStatus.StopStatus+offset,&data,0x02);

    /*****/
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfState(HysScSystemInfType data)//自身状态
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus = data;

    return true;
}

/****/
bool HysControlerLogic::SetSystemSelfStateStopStatus(u_int16_t stop)//急停状态
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.StopStatus = stop;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.StopStatus - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateHardWareStopStatus(u_int16_t stop)//硬件急停
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.HardwareStop = stop;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.HardwareStop - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateLockStatus(u_int16_t flag)//闭锁状态
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.LockStatus = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.LockStatus - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

//硬件闭锁
bool HysControlerLogic::SetSystemSelfStateHardWareLockStatus(u_int16_t flag)
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.HardwareLock = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.HardwareLock - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateEndStatus(u_int16_t end)//停止状态
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.EndStatus = end;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.EndStatus - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateHardWareEndStatus(u_int16_t end)//停止闭锁
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.HardwareEnd = end;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.HardwareEnd - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStatePersonLockStatus(u_int16_t flag)//人员闭锁
{
    if(flag > 2)
        return false;

    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.PersonLockStat = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.PersonLockStat - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateResetStatus(u_int16_t reset)//复位状态
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.SysResetFlag = reset;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.SysResetFlag - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateB12VStatus(u_int16_t b12v)//B12V状态
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.B12VErrStatus = b12v;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.B12VErrStatus - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}
bool HysControlerLogic::SetSystemSelfStateSlaveMode(u_int16_t mode)//从控模式
{
    if(mode > SYSMODE_MASTER_CTRL)
        return false;

    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.AssessoryStatus = mode;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.AssessoryStatus - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    if((mode == SYSMODE_SLAVE_CTRL)||(mode == SYSMODE_MASTER_CTRL))
        CtrlInf.RunInf.CtrlInf.ScSelfActFlag = 1;
    else
        CtrlInf.RunInf.CtrlInf.ScSelfActFlag = 0;

    qDebug()<<"**************SetSystemSelfStateSlaveMode***************"\
           <<CtrlInf.SystemInf.ScSelfStatus.AssessoryStatus\
          <<CtrlInf.RunInf.CtrlInf.ScSelfActFlag;
    return true;
}

bool HysControlerLogic::SetSystemSelfStateLeftCommErr(u_int16_t flag)
{
    if(flag > 1)
        return false;

    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.LeftCommStatus = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.LeftCommStatus - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}
bool HysControlerLogic::SetSystemSelfStateRightCommErr(u_int16_t flag)
{
    if(flag > 1)
        return false;

    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.RightCommStatus = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.RightCommStatus - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateAllGlobalParamSame(u_int16_t flag)
{
    if(flag > 1)
        return false;
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.GlobalParamCrc = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.GlobalParamCrc - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateLeftGlobalParamSame(u_int16_t flag)
{
    if(flag > 1)
        return false;

    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.LeftGlobalParamCrcSame = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.LeftGlobalParamCrcSame - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}
bool HysControlerLogic::SetSystemSelfStateRightGlobalParamSame(u_int16_t flag)
{
    if(flag > 1)
        return false;

    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.RightGlobalParamCrcSame = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.RightGlobalParamCrcSame - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateProgramVerSame(u_int16_t flag)
{
    if(flag > 1)
        return false;

    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.ScSelfStatus.NearPrgVersinSame = flag;
    /*****/
    int offset = &CtrlInf.SystemInf.ScSelfStatus.NearPrgVersinSame - &CtrlInf.SystemInf.ScSelfStatus.StopStatus;
    CtrlInf.SystemInf.ScSelfStatusFaultTime.ScSelfFaultTimeInf[offset] = GetSystemFaultTime();

    return true;
}

bool HysControlerLogic::SetSystemSelfStateIrSensorRecv(INFRED_DATA_TYPE ir)//红外接收的数据
{
    QWriteLocker lock(m_system_locker);
    memmove(&CtrlInf.SystemInf.ScSelfStatus.IrSensorRecvId,&ir.InfredId,sizeof(ir));

    return true;
}

bool HysControlerLogic::GetSystemSelfState(u_int16_t offset,u_int16_t &data)//自身状态
{
    if(offset > sizeof(CtrlInf.SystemInf.ScSelfStatus))
         return false;

    QReadLocker lock(m_system_locker);
    memmove(&data,(u_int8_t *)&CtrlInf.SystemInf.ScSelfStatus.StopStatus+offset,0x02);

}

bool HysControlerLogic::GetSystemSelfState(HysScSystemInfType &data)//自身状态
{
    QReadLocker lock(m_system_locker);
    data = CtrlInf.SystemInf.ScSelfStatus;

    return true;
}

u_int16_t HysControlerLogic::GetSystemSelfStateHardWareStopStatus(void)//硬件急停
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.HardwareStop;
}

u_int16_t HysControlerLogic::GetSystemSelfStateHardWareLockStatus(void)//硬件闭锁
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.HardwareLock;
}

u_int16_t HysControlerLogic::GetSystemSelfStateHardWareEndStatus(void)//停止闭锁
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.HardwareEnd;
}

u_int16_t HysControlerLogic::GetSystemSelfStateB12VStatus(void)//B12V状态
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.B12VErrStatus;
}

u_int16_t HysControlerLogic::GetSystemSelfStateStopStatus(void)//急停
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.StopStatus;
}

u_int16_t HysControlerLogic::GetSystemSelfStateLockStatus(void)//闭锁
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.LockStatus;
}

u_int16_t HysControlerLogic::GetSystemSelfStateEndStatus(void)//停止
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.EndStatus;
}

u_int16_t HysControlerLogic::GetSystemSelfStatePersonLockStatus(void)//人员闭锁
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.PersonLockStat;
}

u_int16_t HysControlerLogic::GetSystemSelfStateSlaveMode(void)//从控模式
{
    QReadLocker lock(m_system_locker);
    return CtrlInf.SystemInf.ScSelfStatus.AssessoryStatus;
}

bool HysControlerLogic::SetSystemSsInfDownState(V4AppSsSystemInfDownType   SsStatus)//服务器状态
{
    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.SsDownStatus = SsStatus;
    CtrlInf.SystemInf.ScSelfStatus.MineRunDirection = SsStatus.MineRunDirection;
    CtrlInf.SystemInf.ScSelfStatus.FollowStageNumber = SsStatus.FollowStageNumber;
    CtrlInf.SystemInf.ScSelfStatus.MinePosition = SsStatus.MinePosition;
    CtrlInf.SystemInf.ScSelfStatus.FollowEnabledFlag = SsStatus.FollowEnabledFlag;

    return true;
}

bool HysControlerLogic::GetSystemSsInfDownState(V4AppSsSystemInfDownType   &SsStatus)//服务器状态
{
    QReadLocker lock(m_system_locker);
    SsStatus = CtrlInf.SystemInf.SsDownStatus;

    return true;
}

u_int8_t HysControlerLogic::SetSystemPersonState(PersonLableDevType   PersonInf)//人员状态
{
    u_int8_t retId = 0;
    u_int16_t totalnumber = 0;

    QWriteLocker lock(m_system_locker);
    for(u_int8_t i = 0; i < PERSON_LABLE_NUMB_MAX;i++)
    {
        if((CtrlInf.SystemInf.PersonStatus.PersonInf[i].PersonLableId == 0)\
            ||(CtrlInf.SystemInf.PersonStatus.PersonInf[i].PersonLableId == PersonInf.PersonLableId))
        {
            CtrlInf.SystemInf.PersonStatus.PersonInf[i].PersonLableId = PersonInf.PersonLableId;
            CtrlInf.SystemInf.PersonStatus.PersonInf[i].PersonLableAuthority = PersonInf.PersonLableAuthority;
            CtrlInf.SystemInf.PersonStatus.PersonInf[i].PersonLableStatus = PersonInf.PersonLableStatus;
            CtrlInf.SystemInf.PersonStatus.PersonInf[i].PersonLableDistance = PersonInf.PersonLableDistance;
            CtrlInf.SystemInf.PersonStatus.PersonInf[i].PersonLableAliveTime = PERSON_LABLE_ALIVE_TIME_DEFAULT;
            CtrlInf.SystemInf.PersonStatus.PersonInf[i].PersonLableScNumb = 0;
            retId = i+1;
//            qDebug()<<"******************HysControlerLogic::SetSystemPersonState**list,id,status,distance********"<<retId<<PersonInf.PersonLableId<<PersonInf.PersonLableStatus<<PersonInf.PersonLableDistance;

            break;
        }
    }
    return retId;
}

u_int8_t HysControlerLogic::SetSystemPersonState(HysPersonInf   PersonInf)//人员状态
{
    u_int8_t retId = 0;

    QWriteLocker lock(m_system_locker);
    CtrlInf.SystemInf.PersonStatus =  PersonInf;

    return retId;
}

bool HysControlerLogic::GetSystemPersonState(u_int16_t Id, PersonLableType   &PersonInf)//人员状态
{
    if(Id > HYS_PERSON_NUB_MAX)
        return  false;

    QReadLocker lock(m_system_locker);
    PersonInf = CtrlInf.SystemInf.PersonStatus.PersonInf[Id-1];
    return true;
}

bool HysControlerLogic::GetSystemPersonState(HysPersonInf   &PersonStatus)//人员状态
{
    QReadLocker lock(m_system_locker);
    PersonStatus = CtrlInf.SystemInf.PersonStatus;
    return true;
}

bool HysControlerLogic::GetPersonTotalNumber(u_int16_t &number)//人员状态
{
    QReadLocker lock(m_system_locker);
    number = CtrlInf.SystemInf.PersonStatus.TotalNumber;
    return true;
}

//插入系统中其他设备的实时状态.位置非固定
bool HysControlerLogic::InsertSystemOtherDevStatus(HysGzmOtherDevStatus devstatus)
{
    if(devstatus.StartPointId == 0x00)
        return false;

    int position = 0;
    QWriteLocker lock(m_system_locker);
    for(position = 0;position < HYS_GZM_OTHER_DEV_POINT_MAX;position++)
    {
        if(CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].StartPointId == 0)//查找新的位置
            break;
        if(CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].StartPointId == devstatus.StartPointId)//已有的位置
            break;
    }
    if(position != HYS_GZM_OTHER_DEV_POINT_MAX)
    {
        CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position] = devstatus;
        return true;
    }
    else
    {
        return false;
    }
}

bool HysControlerLogic::InsertSystemOtherDevStatus(u_int16_t StartPointId, u_int16_t Point, u_int16_t Value)
{
    if(StartPointId == 0x00)
        return false;

    if(Point > HYS_GZM_OTHER_DEV_POINT_MAX)
        return false;

    int position = 0;
    QWriteLocker lock(m_system_locker);
    for(position = 0;position < HYS_GZM_OTHER_DEV_POINT_MAX;position++)
    {
        if(CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].StartPointId == 0)//查找新的位置
            break;
        if(CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].StartPointId == StartPointId)//已有的位置
            break;
    }
    if(position != HYS_GZM_OTHER_DEV_POINT_MAX)
    {
        if(CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].StartPointId != StartPointId)
            CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].StartPointId = StartPointId;
        CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].PointValue[Point-1] = Value;
        //qDebug()<<"******InsertSystemOtherDevStatus*****"<<position<<Point<<Value << "value:"\
                <<  CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].StartPointId\
                <<  CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].PointValue[Point-1];
        return true;
    }
    else
    {
        return false;
    }
}

//查询系统中其他设备的实时状态.位置非固定
bool HysControlerLogic::SerchSystemOtherDevStatus(HysGzmOtherDevStatus &devstatus)//系统中其他设备的实时状态
{
    if(devstatus.StartPointId == 0x00)
        return false;

    int position = 0;
    QReadLocker lock(m_system_locker);
    for(position = 0;position < HYS_GZM_OTHER_DEV_POINT_MAX;position++)
    {
        if(CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].StartPointId == devstatus.StartPointId)
        {
            memmove(devstatus.PointValue,CtrlInf.SystemInf.OtherDevStatus.OtherDevInf[position].PointValue,sizeof(devstatus.PointValue));
            break;
        }
    }
    if(position < HYS_GZM_OTHER_DEV_POINT_MAX)
        return true;
    else
        return false;
}

/************************/
/***运行，自身设备状态***/
bool HysControlerLogic::SetRunScSelfState(u_int16_t offset,u_int16_t data)
{
    QWriteLocker lock(m_run_locker);
    memmove(&CtrlInf.RunInf.ScRunInf.IpAddrHigh+offset,&data,0x02);
    return true;
}

bool HysControlerLogic::SetRunScSelfState(HysRunScSelfType scstatus)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf = scstatus;
    return true;
}

bool HysControlerLogic::GetRunScSelfState(u_int16_t offset,u_int16_t &data)
{
    QReadLocker lock(m_run_locker);
    memmove(&data,&CtrlInf.RunInf.ScRunInf.IpAddrHigh+offset,0x02);
    return true;
}

bool HysControlerLogic::GetRunScSelfState(HysRunScSelfType &scstatus)
{
    QReadLocker lock(m_run_locker);
    scstatus = CtrlInf.RunInf.ScRunInf;
    return true;
}

bool HysControlerLogic::SetRunScSelfScIpAddr(u_int16_t IpAddrHigh,u_int16_t IpAddrLow)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf.IpAddrHigh = IpAddrHigh;
    CtrlInf.RunInf.ScRunInf.IpAddrLow = IpAddrLow;
    return true;
}

bool HysControlerLogic::SetRunScSelfServerEnabled(u_int16_t data)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf.SsServerEnabled = data;
    return true;
}

bool HysControlerLogic::SetRunScSelfServerCommErr(u_int16_t data)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf.SsServerCommErr = data;
    return true;
}

bool HysControlerLogic::SetRunScSelfWirelessSigStrength(u_int16_t data)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf.WirelessSigStrength = data;
    return true;
}

bool HysControlerLogic::SetRunScSelfBuleTeethSigStrength(u_int16_t data)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf.BuleTeethSigStrength = data;
    return true;
}
bool HysControlerLogic::SetRunScSelfStopPosition(u_int16_t data)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf.StopPosition = data;
//    qDebug()<<"***************CtrlInf.RunInf.ScRunInf.StopPosition****************"<<data<<CtrlInf.RunInf.ScRunInf.StopPosition;
    return true;
}
bool HysControlerLogic::SetRunScSelfEndPosition(u_int16_t data)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf.EndPosition = data;
    qDebug()<<"***************CtrlInf.RunInf.ScRunInf.EndPosition****************"<<data<<CtrlInf.RunInf.ScRunInf.EndPosition;
    return true;
}
bool HysControlerLogic::SetRunScSelfPersonLockLeftTime(u_int16_t data)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.ScRunInf.PersonLockLeftTime = data;
    return true;
}

bool HysControlerLogic::GetRunScSelfScIpAddr(u_int16_t &IpAddrHigh,u_int16_t &IpAddrLow)
{
    QReadLocker lock(m_run_locker);
    IpAddrHigh = CtrlInf.RunInf.ScRunInf.IpAddrHigh;
    IpAddrLow = CtrlInf.RunInf.ScRunInf.IpAddrLow;
    return true;
}

bool HysControlerLogic::GetRunScSelfServerEnabled(u_int16_t &data)
{
    QReadLocker lock(m_run_locker);
    data = CtrlInf.RunInf.ScRunInf.SsServerEnabled;
    return true;
}

bool HysControlerLogic::GetRunScSelfServerCommErr(u_int16_t &data)
{
    QReadLocker lock(m_run_locker);
    data = CtrlInf.RunInf.ScRunInf.SsServerCommErr;
    return true;
}
bool HysControlerLogic::GetRunScSelfWirelessSigStrength(u_int16_t &data)
{
    QReadLocker lock(m_run_locker);
    data = CtrlInf.RunInf.ScRunInf.WirelessSigStrength;
    return true;
}
bool HysControlerLogic::GetRunScSelfBuleTeethSigStrength(u_int16_t &data)
{
    QReadLocker lock(m_run_locker);
    data = CtrlInf.RunInf.ScRunInf.BuleTeethSigStrength;
    return true;
}
bool HysControlerLogic::GetRunScSelfStopPosition(u_int16_t &data)
{
    QReadLocker lock(m_run_locker);
    data = CtrlInf.RunInf.ScRunInf.StopPosition;
    return true;
}
bool HysControlerLogic::GetRunScSelfEndPosition(u_int16_t &data)
{
    QReadLocker lock(m_run_locker);
    data = CtrlInf.RunInf.ScRunInf.EndPosition;
    return true;
}
bool HysControlerLogic::GetRunScSelfPersonLockLeftTime(u_int16_t &data)
{
    QReadLocker lock(m_run_locker);
    data = CtrlInf.RunInf.ScRunInf.PersonLockLeftTime;
    return true;
}

/***运行，惯导状态***/
bool HysControlerLogic::SetRunLascState(HysRunLascData lascstatus)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.LascInf = lascstatus;
    return true;
}

bool HysControlerLogic::GetRunLascState(HysRunLascData &lascstatus)
{
    QReadLocker lock(m_run_locker);
    lascstatus = CtrlInf.RunInf.LascInf;
    return true;
}

/***运行，反冲洗状态***/
bool HysControlerLogic::SetRunBackFlushState(HysRunBackflushData bfstatus)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.FlushInf = bfstatus;
    return true;
}

bool HysControlerLogic::GetRunBackFlushState(HysRunBackflushData &bfstatus)
{
    QReadLocker lock(m_run_locker);
    bfstatus = CtrlInf.RunInf.FlushInf;
    return true;
}

/***运行，通讯状态***/
bool HysControlerLogic::SetRunCommAreaState(HysRunCommArea commstatus)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.CommInf = commstatus;
    return true;
}

bool HysControlerLogic::GetRunCommAreaState(HysRunCommArea &commstatus)
{
    QReadLocker lock(m_run_locker);
    commstatus = CtrlInf.RunInf.CommInf;
    return true;
}

/***运行，控制信息状态***/
bool HysControlerLogic::SetRunScCtrlInfState(HysScCtrlInfType ctrlstatus)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.CtrlInf = ctrlstatus;
    return true;
}

bool HysControlerLogic::GetRunScCtrlInfState(HysScCtrlInfType &ctrlstatus)
{
    QReadLocker lock(m_run_locker);
    ctrlstatus = CtrlInf.RunInf.CtrlInf;
    return true;
}

bool HysControlerLogic::SetRunScCtrlScSelfActFlag(u_int16_t flag)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.CtrlInf.ScSelfActFlag = flag;
    return true;
}
bool HysControlerLogic::GetRunScCtrlScSelfActFlag(u_int16_t &flag)
{
    QReadLocker lock(m_run_locker);
    flag = CtrlInf.RunInf.CtrlInf.ScSelfActFlag;
    return true;
}

bool HysControlerLogic::SetRunScCtrlScWirelessCtrlFlag(u_int16_t flag)
{
    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.CtrlInf.ScWirelessCtrlFlag = flag;
    return true;
}
u_int16_t HysControlerLogic::GetRunScCtrlScWirelessCtrlFlag(void)
{
    QReadLocker lock(m_run_locker);
    return CtrlInf.RunInf.CtrlInf.ScWirelessCtrlFlag;
}

bool HysControlerLogic::IsSensorIdInRunScCtrlOther(u_int16_t scid, u_int16_t sensorid,u_int16_t &listnumber)
{
    listnumber = 0;

    QReadLocker lock(m_run_locker);
    if(scid != CtrlInf.RunInf.CtrlInf.OtherControlledScId)
        return false;
    for(u_int16_t i = 0; i < RUN_OTHER_SC_ACT_SENSOR_NUMB_MAX;i++)
    {
        if(CtrlInf.RunInf.CtrlInf.OtherScSensorInf[i].Id == sensorid)
        {
            listnumber = i+1;
            break;
        }
    }
    if(listnumber == 0)
        return false;
    else
        return true;
}

u_int16_t HysControlerLogic::SetRunOtherControlledScIdSensor(u_int16_t scid,u_int16_t sensor1,u_int16_t sensor2,u_int16_t sensor3,\
                                                            u_int16_t sensor4,u_int16_t sensor5,u_int16_t sensor6)
{
    if((scid < HYS_CTRL_ID_MIN)||(scid > HYS_CTRL_ID_MAX))
        return false;

    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.CtrlInf.OtherControlledScId = scid;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[0].Id = sensor1;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[0].Value = 0;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[1].Id = sensor2;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[1].Value = 0;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[2].Id = sensor3;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[2].Value = 0;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[3].Id = sensor4;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[3].Value = 0;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[4].Id = sensor5;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[4].Value = 0;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[5].Id = sensor6;
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[5].Value = 0;

    return true;
}

u_int16_t HysControlerLogic::GetRunOtherControlledScId(void)
{
    QReadLocker lock(m_run_locker);
    return CtrlInf.RunInf.CtrlInf.OtherControlledScId;
}

bool HysControlerLogic::SetRunScCtrlOtherScSensorInf(u_int16_t id, HysRunActSensorDataType sensor)
{
    if((id > HYS_GZM_OTHER_DEV_NUMB_MAX)||(id == 0))
        return false;

    QWriteLocker lock(m_run_locker);
    CtrlInf.RunInf.CtrlInf.OtherScSensorInf[id-1] = sensor;
    return true;
}

bool HysControlerLogic::GetRunScCtrlOtherScSensorInf(u_int16_t id, HysRunActSensorDataType &sensor)
{
    if((id > HYS_GZM_OTHER_DEV_NUMB_MAX)||(id == 0))
        return false;

    QReadLocker lock(m_run_locker);
    sensor = CtrlInf.RunInf.CtrlInf.OtherScSensorInf[id-1];
    return true;
}

/************************/
bool HysControlerLogic::InsertLogInf(LoggerApiType log)
{
    HysDykLogInfType temp;
    QByteArray arry;
    int length = 0;

    memset(&temp,0x00,sizeof(temp));
    arry = log.time.toUtf8();
    memmove(&temp.log[0].time,arry.data()+11,0x08);

    arry.clear();
    arry = log.Event.toUtf8();
    if(arry.size()>sizeof(temp.log[0].Event))
        length = sizeof(temp.log[0].Event);
    else
        length = arry.size();
    memmove(&temp.log[0].Event,arry.data(),length);

    arry.clear();
    arry = log.State.toUtf8();
    if(arry.size()>sizeof(temp.log[0].State))
        length = sizeof(temp.log[0].State);
    else
        length = arry.size();
    memmove(&temp.log[0].State,arry.data(),length);

    memmove(temp.log[1].time,\
            CtrlInf.LogInf.log[0].time,\
            sizeof (HysDykLogInfType)-sizeof (LoggerRestoreType));

    QReadLocker lock(m_run_locker);
    CtrlInf.LogInf = temp;
    return true;
}


/*****************************/
bool HysControlerLogic::GetCtrlInfAll(HysControllerInfType &inf)
{
   QReadLocker lock(m_basic_locker);
   inf = CtrlInf;
   return true;
}
