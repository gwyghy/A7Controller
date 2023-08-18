#ifndef HYSSYSTEM_H
#define HYSSYSTEM_H

/********/
#include <QObject>
#include <QMutex>
#include "./HysController/hyscontrollerlogic.h"
#include "logdatatype.h"
#include "Action/actionctrldatatype.h"
#include "hyssystemtype.h"
#include <QTimer>
#include "./shm/tshddata.h"

/****逻辑程序的版本号****/
#define A7CONTROLLER_LOGIC_VERSION_1        0x09
#define A7CONTROLLER_LOGIC_VERSION_2        0x08
#define A7CONTROLLER_LOGIC_VERSION_3        0x00
#define A7CONTROLLER_LOGIC_VERSION_4        0x02

/********/
class HysSystem : public QObject,public HysControlerLogic
{
    Q_OBJECT
public:
    HysSystem(QObject *parent = 0);
    ~HysSystem();
    static HysSystem *getInston(QObject *parent = 0);

    void Init(void);

    /******/
    int HysSystemPeriodicProc(void);//周期性处理

    /****/
    int SetSingleActString(const QStringList strlist);
    int SetAssembleActString(const QStringList strlist);
    int SetGrpActString(const QStringList strlist);
    int SetPartString(const QStringList strlist);
    int SetSensorString(const QStringList strlist);
    int SetExtDevString(const QStringList strlist);
    int SetSystemErrString(const QStringList strlist);
    int SetSerivceOperateString(const QStringList strlist);

    /****/
    QString GetSingleActString(int number);
    QString GetAssembleActString(int number);
    QString GetGrpActString(int number);
    QString GetPartString(int number);
    QString GetSensorString(int number);
    QString GetSystemErrString(int number);
    QString GetSerivceOperateString(int number);

    /***设置驱动的状态***/
    bool SetDriverExtDevState(u_int16_t driverid, u_int16_t devid, u_int16_t childdevid,DriverExtDevStatusType driverdevstate);

    /**写日志相关函数**/
    /**编号，类型，状态，数据指针，长度***/
    int WriteFaultApi(u_int16_t host_type, u_int16_t host_name, u_int16_t event, u_int16_t status);
    /**编号，类型，状态，数据指针，长度***/
    int WriteLogApi(u_int16_t host, u_int16_t type, u_int16_t status,void *data ,u_int16_t len);

    /***SysStopState**/
    bool SetSysStopStateStopFlag(u_int16_t scnumber, bool flag);
    bool SetSysStopStateLockFlag(u_int16_t scnumber, bool flag);
    bool SetSysStopStateEndFlag(u_int16_t scnumber, bool flag);

    bool GetSysStopStateStopFlag(uint scid);
    bool GetSysStopStateLockFlag(uint scid);
    bool GetSysStopStateEndFlag(uint scid);
    /***0即级标签**/
    bool GetAllZeroPersonLockState(void);
    /******/
    int IsEmergencyStopAction(SysCtrlInfoType *pctrl);

    /************************/
    CombActionStateType * GetCombActState(u_int16_t comb_id);
    groupActionStateType *GetGrpActState(u_int16_t grp_id);

private:
    /******/
    int ServerReportSystemState(u16 delay);
    int ServerReportPersonState(u16 DelayTime);
    void SysB12VProcess(void);
    void SysEndProcess(u16 delay);
    /*****/
    void PersonCtrlAlarmProc(u8 state);
    void PersonLockControllActionProc(u8 cmd);
    void sysDevicePersonLock(u16 delay);
    void sysZeroPersonLockProc(u16 delay);

private:
    int RunParamChangedListInit(void);
    /*****/
    static int InsertRunParamChangedList(u_int16_t menuID,u_int16_t SubMenuId,ParamChangeDetailInfType strlist);
    /*****/
    static int SystemParamChangedProc(u_int16_t menuID,u_int16_t SubMenuId,ParamChangeDetailInfType strlist);
    int RunParamChangedListPeriodicProc(void);
private:
    //发送无线总线的数据。待定
    void SendWirelessBusData(uint8_t state,uint8_t id);
    /***紧急事件发生时，控制器间的相关处理***/
    void netReportStopLockEndState(u8 stateType,u8 state);
    /***紧急事件发生时，对遥控器的相关操作***/
    void WirelessHandleStopLockEndStateProc(u8 stateType,u8 state);
private:
    bool SharedMemCreat(void);
    void SharedMemRefreshProc(u_int16_t DelayTime);
    void ExitAddrConfigModeProc(u_int16_t DelayTime);
private:
    /****/
    static HysSystem* that;

    /****/
    QStringList SystemErrStr;
    QStringList SingleActStr;
    QStringList AssembleActStr;
    QStringList GrpActStr;
    QStringList PartStr;
    QStringList SensorStr;
    QStringList ExtDevStr;
    QStringList ServiceStr;

private:
    /*****/
    CombActionStateType CombAction[COMBACTION_NUM_MAX];//存放组合动作的状态。新
    groupActionStateType GroupAction[GROUPACTION_NUM_MAX];//存放成组动作的状态。新

    /*****/
    QMutex *m_SysStopMutex;
    SysStopType SysStopState[V4APP_DEV_ID_MAX+6];//用于存放急停闭锁标志.0服务器，1~250控制器，251遥控器
    int kzqTurnonReson;//控制器的复位原因.获取方法待定

    //用于存放手动控制时，被控架传感器实时数值
    HysScSensorType ManualActSensorState[HYS_SENSOR_NUMB_MAX];

    /****/
    QMutex *m_ParamChangedMutex;
    QList<ParamChangedNeedProcType> ParamChangedRestoreList;//用于修改参数修改的ID
    QList<ParamChangedNeedProcType> ParamChangedSetList;//初始设置的需处理的内容
public slots:

};

#endif // HYSSYSTEM_H
