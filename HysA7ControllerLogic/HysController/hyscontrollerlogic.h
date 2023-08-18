#ifndef HYSCONTROLERLOGIC_H
#define HYSCONTROLERLOGIC_H

#include <QList>
#include <QReadWriteLock>
#include "./HysController/hyscontrollertype.h"
#include "libhysa7parammng.h"
#include "./HysDeviceMng/devctrl.h"

/********************************************************************************/
class HysControlerLogic
{
public:
    explicit HysControlerLogic(u_int16_t id = 1, u_int16_t type = 1);
    ~HysControlerLogic();

    /************************/
    bool ParamInit(u_int16_t CtrlId,u_int16_t Ctrltype);
    bool PartInfInit(QList<PartActSensorRealatedType> list);
    bool SensorInfInit(QList<SensorDisplayParamType> sensorlist);

    /************************/
    bool SetBasicScNumber(u_int16_t scid);
    bool SetBasicScType(u_int16_t sctype);
    bool GetBasicScNumber(u_int16_t &scid);
    bool GetBasicScType(u_int16_t &sctype);

    /************************/
    bool SetPartState(u_int16_t part_id,u_int16_t part_state);
    bool SetSingleActState(u_int16_t id,HysScSingleActType actstate);
    bool GetPartState(u_int16_t part_id,u_int16_t &part_state);
    bool GetSingleActState(u_int16_t act_id,HysScSingleActType &actstate);
    int GetPartIdMinMax(int &min, int &max);
    int GetActIdMinMax(int &min,int &max);

    /************************/
    bool SetSensorState(u_int16_t id,HysScSensorType sensor_state);
    bool GetSensorState(u_int16_t sensor_id,HysScSensorType &sensorstate);
    HysScSensorType *GetSensorState(u_int16_t sensor_id);

    /************************/
    HysScSingleActType *GetSingleActState(u_int16_t singleact_id);
    HysPartStateType * GetPartState(u_int16_t part_id);

    //获取所有传感器的状态数据，用于上报给服务器
    uint GetAllSensorStateDataToReport(u_int8_t *pBuf);
    int GetSensorIdMinMax(int &min,int &max);

    /************************/
    bool SetExtDevEnabled(u_int16_t extdev_id, u_int16_t enable);
    bool SetExtDevState(u_int16_t extdev_id,DriverExtDevStatusType extdevstate);
    bool SetExtDevState(u_int16_t extdev_id,HysScExtDevType extdevstate);
    bool SetExtDevState(u_int16_t driverid, u_int16_t devid, u_int16_t childdevid,DriverExtDevStatusType driverdevstate);

    bool GetExtDevState(u_int16_t extdev_id,HysScExtDevType &extdevstate);
    /***获取外围设备的掉线状态***/
    /***存在多个传感器绑定同一外围设备的情况***/
    bool GetExtDevState(u_int16_t driverid, u_int16_t devid, u_int16_t childdevid, QList<u_int16_t> &extdevid_list, QList<u_int16_t> &Status_list);
    int GetExtDevIdMinMax(int &min,int &max);

    /***系统状态中相关访问接口函数***/
    bool SetSystemSelfState(u_int16_t offset,u_int16_t data);//自身状态,同时记录时间
    bool SetSystemSelfState(HysScSystemInfType data);//自身状态,同时记录时间
    /****/
    bool SetSystemSelfStateStopStatus(u_int16_t stop);//急停状态
    bool SetSystemSelfStateHardWareStopStatus(u_int16_t stop);//硬件急停
    bool SetSystemSelfStateLockStatus(u_int16_t flag);//闭锁状态
    bool SetSystemSelfStateHardWareLockStatus(u_int16_t flag);//硬件闭锁
    bool SetSystemSelfStateEndStatus(u_int16_t end);//停止状态
    bool SetSystemSelfStateHardWareEndStatus(u_int16_t end);//停止闭锁

    bool SetSystemSelfStatePersonLockStatus(u_int16_t flag);//人员闭锁
    bool SetSystemSelfStateResetStatus(u_int16_t reset);//复位状态
    bool SetSystemSelfStateB12VStatus(u_int16_t b12v);//B12V状态
    bool SetSystemSelfStateSlaveMode(u_int16_t mode);//从控模式

    bool SetSystemSelfStateLeftCommErr(u_int16_t flag);
    bool SetSystemSelfStateRightCommErr(u_int16_t flag);

    bool SetSystemSelfStateAllGlobalParamSame(u_int16_t flag);
    bool SetSystemSelfStateLeftGlobalParamSame(u_int16_t flag);
    bool SetSystemSelfStateRightGlobalParamSame(u_int16_t flag);

    bool SetSystemSelfStateProgramVerSame(u_int16_t flag);

    bool SetSystemSelfStateIrSensorRecv(INFRED_DATA_TYPE ir);//红外接收的数据

    bool GetSystemSelfState(u_int16_t offset, u_int16_t &data);//自身状态
    bool GetSystemSelfState(HysScSystemInfType &data);//自身状态

    /****/
    u_int16_t GetSystemSelfStateStopStatus(void);//急停
    u_int16_t GetSystemSelfStateLockStatus(void);//闭锁
    u_int16_t GetSystemSelfStateEndStatus(void);//停止
    u_int16_t GetSystemSelfStatePersonLockStatus(void);//人员闭锁
    u_int16_t GetSystemSelfStateSlaveMode(void);//从控模式
    u_int16_t GetSystemSelfStateHardWareStopStatus(void);//硬件急停
    u_int16_t GetSystemSelfStateHardWareLockStatus(void);//硬件闭锁
    u_int16_t GetSystemSelfStateHardWareEndStatus(void);//停止闭锁
    u_int16_t GetSystemSelfStateB12VStatus(void);//B12V状态

    /*******/
    bool SetSystemSsInfDownState(V4AppSsSystemInfDownType   SsStatus);//服务器状态
    bool GetSystemSsInfDownState(V4AppSsSystemInfDownType   &SsStatus);//服务器状态

    u_int8_t SetSystemPersonState(PersonLableDevType   PersonInf);//直接写入人员状态
    u_int8_t SetSystemPersonState(HysPersonInf   PersonInf);//人员状态

    bool GetSystemPersonState(u_int16_t Id, PersonLableType   &PersonInf);//人员状态
    bool GetSystemPersonState(HysPersonInf   &PersonStatus);//人员状态
    bool GetPersonTotalNumber(u_int16_t &number);//人员状态

    //插入系统中其他设备的实时状态.位置非固定
    bool InsertSystemOtherDevStatus(HysGzmOtherDevStatus devstatus);//系统中其他设备的实时状态
    bool InsertSystemOtherDevStatus(u_int16_t StartPointId, u_int16_t Point, u_int16_t Value);
    //查询系统中其他设备的实时状态.位置非固定
    bool SerchSystemOtherDevStatus(HysGzmOtherDevStatus &devstatus);//系统中其他设备的实时状态

    /************************/
    /***运行，自身设备状态***/
    bool SetRunScSelfState(u_int16_t offset,u_int16_t data);
    bool SetRunScSelfState(HysRunScSelfType scstatus);
    bool GetRunScSelfState(u_int16_t offset,u_int16_t &data);
    bool GetRunScSelfState(HysRunScSelfType &scstatus);

    bool SetRunScSelfScIpAddr(u_int16_t IpAddrHigh,u_int16_t IpAddrLow);
    bool SetRunScSelfServerEnabled(u_int16_t data);
    bool SetRunScSelfServerCommErr(u_int16_t data);
    bool SetRunScSelfWirelessSigStrength(u_int16_t data);
    bool SetRunScSelfBuleTeethSigStrength(u_int16_t data);
    bool SetRunScSelfStopPosition(u_int16_t data);
    bool SetRunScSelfEndPosition(u_int16_t data);
    bool SetRunScSelfPersonLockLeftTime(u_int16_t data);


    bool GetRunScSelfScIpAddr(u_int16_t &IpAddrHigh,u_int16_t &IpAddrLow);
    bool GetRunScSelfServerEnabled(u_int16_t &data);
    bool GetRunScSelfServerCommErr(u_int16_t &data);
    bool GetRunScSelfWirelessSigStrength(u_int16_t &data);
    bool GetRunScSelfBuleTeethSigStrength(u_int16_t &data);
    bool GetRunScSelfStopPosition(u_int16_t &data);
    bool GetRunScSelfEndPosition(u_int16_t &data);
    bool GetRunScSelfPersonLockLeftTime(u_int16_t &data);

    /***运行，惯导状态***/
    bool SetRunLascState(HysRunLascData lascstatus);
    bool GetRunLascState(HysRunLascData &lascstatus);

    /***运行，反冲洗状态***/
    bool SetRunBackFlushState(HysRunBackflushData bfstatus);
    bool GetRunBackFlushState(HysRunBackflushData &bfstatus);

    /***运行，通讯状态***/
    bool SetRunCommAreaState(HysRunCommArea commstatus);
    bool GetRunCommAreaState(HysRunCommArea &commstatus);

    /***运行，控制信息状态***/
    bool SetRunScCtrlInfState(HysScCtrlInfType ctrlstatus);
    bool GetRunScCtrlInfState(HysScCtrlInfType &ctrlstatus);

    bool SetRunScCtrlScSelfActFlag(u_int16_t flag);
    bool GetRunScCtrlScSelfActFlag(u_int16_t &flag);
    bool SetRunScCtrlScWirelessCtrlFlag(u_int16_t flag);
    u_int16_t GetRunScCtrlScWirelessCtrlFlag(void);

    bool IsSensorIdInRunScCtrlOther(u_int16_t scid, u_int16_t sensorid,u_int16_t &listnumber);
    u_int16_t SetRunOtherControlledScIdSensor(u_int16_t scid,u_int16_t sensor1,u_int16_t sensor2,u_int16_t sensor3,\
                                              u_int16_t sensor4,u_int16_t sensor5,u_int16_t sensor6);
    u_int16_t GetRunOtherControlledScId(void);
    bool SetRunScCtrlOtherScSensorInf(u_int16_t id, HysRunActSensorDataType sensor);
    bool GetRunScCtrlOtherScSensorInf(u_int16_t id, HysRunActSensorDataType &sensor);

    /************************/
    bool InsertLogInf(LoggerApiType log);

    /*****************************/
    bool GetCtrlInfAll(HysControllerInfType &inf);
 private:
    void Init(u_int16_t id,u_int16_t type);

private:
    HysControllerInfType CtrlInf;//支架控制器的对象模型，存储重要的数据以及共享内存的构成

private:
    QReadWriteLock *m_basic_locker;
    QReadWriteLock *m_part_locker;
    QReadWriteLock *m_sensor_locker;
    QReadWriteLock *m_extdev_locker;
    QReadWriteLock *m_system_locker;
    QReadWriteLock *m_run_locker;
    QReadWriteLock *m_log_locker;
    /****/
    u_int16_t ActRefList[HYS_SINGLEACT_NUMB_MAX+2];//find
    /****/
};

#endif // HYSCONTROLERLOGIC_H
