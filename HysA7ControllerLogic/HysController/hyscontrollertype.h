#ifndef HYS_CONTROLER_TYPE_H
#define HYS_CONTROLER_TYPE_H

#include <sys/types.h>
#include <QString>

/**设置每一子设备的个数**/
#define     HYS_PART_NUMB_MAX               32
#define     HYS_SINGLEACT_NUMB_MAX          64
#define     HYS_ASSEMBLELEACT_NUMB_MAX      16//
#define     HYS_GRPACT_NUMB_MAX             22
#define     HYS_SENSOR_NUMB_MAX             32
#define     HYS_EXTDEV_NUB_MAX              32
#define     HYS_PERSON_NUB_MAX              11

/******/
#define     HYS_GZM_OTHER_DEV_NUMB_MAX      10//工作面其他设备的最大数量
#define     HYS_GZM_OTHER_DEV_POINT_MAX     15//工作面其他设备点的最大数量
/******/
#define     RUN_OTHER_SC_ACT_SENSOR_NUMB_MAX 6
#define     HYS_RUN_RESERVED_DATA_MAX        135
/******/
#define     HYS_LOGGER_RESTORE_MAX           100

/*******/
#define     PERSON_LABLE_ALIVE_TIME_DEFAULT     3000//ms
/***********************一些枚举**************************************/
/************************************/
/**控制器架号范围**/
enum
{
    HYS_CTRL_ID_MIN = 1,
    HYS_CTRL_ID_MAX = 250
};

/**控制器类型范围**/
enum
{
    HYS_CTRL_TYPE_MIN = 1,
    HYS_CTRL_TYPE_MAX = 20
};

/************************************/
/**部件状态枚举，暂定**/
enum
{
    PART_STATUS_INVALID = 0,
    PART_STATUS_IDLE = 0,
    PART_STATUS_SHEN = 1,
    PART_STATUS_SHOU = 2,
    PART_STATUS_SUSPEND = 3,
    PART_STATUS_MAX = 5
};

/**动作状态定义**/
enum
{
    SINGLE_ACT_STATUS_NONE = 0,
    SINGLE_ACT_STATUS_ALARM = 1,
    SINGLE_ACT_STATUS_DELAY = 2,
    SINGLE_ACT_STATUS_STOP = 3,
    SINGLE_ACT_STATUS_START = 4,
    SINGLE_ACT_STATUS_RUNNING = 5,
    SINGLE_ACT_STATUS_MANUAL_RUNNING = 6,
    SINGLE_ACT_STATUS_SUSPEND = 7,
    SINGLE_ACT_STATUS_SUSPEND_TIME_JISHI = 8,
    SINGLE_ACT_STATUS_MAX = SINGLE_ACT_STATUS_SUSPEND_TIME_JISHI
};

/**控制命令字***/
enum
{
    ACTCTRL_START_DIRECT = 0,
    ACTCTRL_STOP,
    ACTCTRL_START_TIMEDELAY,
    ACTCTRL_SUSPEND,
    ACTCTRL_RESUME,
    ACTCTRL_SUSPEND_TIME_JISHI,
    ACTCTRL_MAX = ACTCTRL_SUSPEND_TIME_JISHI
};

/**控制类型***/
enum
{
    ACTCTRL_MANUAL_MODE = 0,
    ACTCTRL_AUTO_MODE,
    ACTCTRL_METHOD_MAX = ACTCTRL_AUTO_MODE
};

/**控制源类型***/
enum
{
    ACT_SRC_SC = 0,
    ACT_SRC_YKQ = 1,
    ACT_SRC_SS = 2,
    ACT_SRC_CAOZUOTAI = 3,
    ACT_SRC_REMOTE = 4,
    ACT_SRC_MAX = ACT_SRC_REMOTE
};

/************************************/
/**控制器所处于的操作状态**/
enum
{
   SCSTATUS_NORMAL = 0,
   SCSTATUS_ACCESSORY,
   SCSTATUS_SET_IP,
};

/************************************/
/**传感器上报故障状态相关**/
enum
{
    SENSOR_FAULT_STATUS_NORMAL = 0,
    SENSOR_FAULT_STATUS_FAULT = 1,
    SENSOR_FAULT_STATUS_REPORT_OVER_LIMIT = 2
};

/************************************/
/***系统中其他设备的相关定义,暂定，与对外协议有关****/
enum
{
    HYS_GZM_OTHERDEV_SELF_INF = 0x30,//自身相关的状态。电压或电流
    HYS_GZM_OTHERDEV_TERMINAL_INF = 0x40,//光电终端
    HYS_GZM_OTHERDEV_QIANLIU_INF = 0x50,//前溜
    HYS_GZM_OTHERDEV_ZHUANZAIJI_INF = 0x60,//转载机
    HYS_GZM_OTHERDEV_HOULIU_INF = 0x70,//后溜
};

/************************************/
/**煤机运行状态**/
enum
{
    MINE_STATE_RUN = 0,
    MINE_STATE_STOP = 1,
};

/**煤机运行方向**/
enum
{
    MINE_FORWARD_NONE   =   0,
    MINE_FORWARD_LITTLE = 1,
    MINE_FORWARD_BIG    = 2,
    MINE_FORWARD_INVALID = 3
};

/************************************/
/***信号强度***/
enum
{
    SIGNAL_STRENTH_NONE = 0,
    SIGNAL_STRENTH_LITTLE,
    SIGNAL_STRENTH_MIDDLE,
    SIGNAL_STRENTH_HIGH
};

/************************************/
/***遥控控制***/
enum
{
    WIRELESS_CTRL_NONE = 0,
    WIRELESS_CTRL_INFRED_SUCCEED,
    WIRELESS_CTRL_WIRELESS_SUCCEED,
    WIRELESS_CTRL_SC_SELF_ACT,
    WIRELESS_CTRL_SC_NEIGBOR_ACT
};
/***********************支架属性相关定义**************************************/
typedef struct
{
    u_int16_t Id;
    u_int16_t Type;
}HysScBasicInfType;

/***********************部件相关定义**************************************/
/**单动作类型结构体**/
typedef struct
{
    u_int16_t CtrlCmd;
    u_int16_t CtrlMethod;
    u_int16_t CtrlSrcAddr;
    u_int16_t CtrlSrcType;
    u_int16_t UserId;
}HYS_SC_ACT_INF_TYPE;

typedef struct
{
    u_int16_t Id;
    u_int16_t Status;
    u_int16_t StatusBackup;
    u_int16_t RunTime;
    u_int16_t CombinActId;

    HYS_SC_ACT_INF_TYPE cmd;
}HysScSingleActType;

/**部件关联的传感器**/
typedef struct
{
    u_int16_t SensorId[4];
}HYS_SC_PART_SENSOR_LIST_TYPE;

typedef struct
{
    u_int16_t   Id;          //新增加。mry
    u_int16_t	partState;	//部件状态
}HysPartStateType;

/**部件对象**/
typedef struct
{
    HysPartStateType PartInf;
    HysScSingleActType Act[2];

    HYS_SC_PART_SENSOR_LIST_TYPE SensorInf;
}HysScPartType;

/***********************传感器相关定义**************************************/
typedef struct
{
    u_int16_t Id;//Id
    u_int16_t OriginalValue;//原始值
    u_int16_t Value;//计算值
    u_int16_t RateOfChange;//变化率

    u_int16_t OnLineStatus;//在线状态
    u_int16_t FaultStatus;//故障状态
}HysScSensorType;

/***********************外围设备相关定义**************************************/
typedef struct
{
    u_int16_t Id;
    u_int16_t Enabled;
    u_int16_t DriverId;
    u_int16_t DevType;
    u_int16_t DevId;
    u_int16_t Status;//设备在线状态//(0离线1在线)
    u_int16_t Value;
    u_int16_t AppVersionHigh;
    u_int16_t AppVersionLow;
    u_int16_t BootVersionHigh;
    u_int16_t BootVersionLow;
}HysScExtDevType;

/**************************系统相关定义**************************************/
typedef struct //0正常1故障,共计24个
{
    u_int16_t StopStatus;
    u_int16_t LockStatus;
    u_int16_t EndStatus;
    u_int16_t HardwareStop;
    u_int16_t HardwareLock;

    u_int16_t HardwareEnd;
    u_int16_t PersonLockStat;//人员闭锁
    u_int16_t SysResetFlag;//复位状态
    u_int16_t B12VErrStatus;
    u_int16_t AssessoryStatus;

    u_int16_t LeftCommStatus;
    u_int16_t RightCommStatus;
    u_int16_t GlobalParamCrc;//2023.07.05,新增加
    u_int16_t LeftGlobalParamCrcSame;
    u_int16_t RightGlobalParamCrcSame;

    u_int16_t GlobalPrgVersinSame;//2023.07.05,新增加
    u_int16_t NearPrgVersinSame;
    /****/
    u_int16_t IrSensorRecvId;//14
    u_int16_t IrSensorRecvStatus;//12
    u_int16_t IrSensorRecvTimes;//10

    /**可以暂时不进行处理的状态,2023.07.05,新增加***/
    u_int16_t MineRunDirection;//8//2023.07.05,新增加
    u_int16_t FollowStageNumber;//6//2023.07.05,新增加
    u_int16_t MinePosition;//4//2023.07.05,新增加
    u_int16_t FollowEnabledFlag;//2//2023.07.05,新增加
}HysScSystemInfType;

/*****服务器下发的原始系统数据***/
typedef struct
{
    u_int16_t MineRunDirection;//0左1右
    u_int16_t FollowStageNumber;
    u_int16_t MinePosition;
    u_int16_t FollowEnabledFlag;
    u_int16_t NowCutNumber;//暂定用于传输当前的刀数

    /**2022.10.19,mry,新增**/
    u_int16_t MineStopStatus;//0否1是
    u_int16_t MineStartStation;//煤机所在位置的起始
    u_int16_t MineEndStation;//煤机所在位置的结束
}V4AppSsSystemInfDownType;

/********/
typedef struct
{
    u_int16_t PersonLableId;//ID
    u_int16_t PersonLableAuthority;//权限
    u_int16_t PersonLableStatus;//状态。低8位标签状态1进入2离开3故障，高8位bit7呼救0否1是/bit0~bit6电池电量百分比。mry,2023.03.15
    u_int16_t PersonLableScNumb;
    u_int16_t PersonLableDistance;//距离
    u_int16_t PersonLableAliveTime;//此标签的生命周期，0为标签已经离开
}PersonLableType;

typedef struct
{
    u_int16_t TotalNumber;
    PersonLableType PersonInf[HYS_PERSON_NUB_MAX];
}HysPersonInf;

/***自身故障发生的具体时刻****/
typedef struct
{
    u_int8_t Year;
    u_int8_t Month;
    u_int8_t Day;
    u_int8_t Hour;
    u_int8_t Minite;
    u_int8_t Seccond;
}HYS_FAULT_TIME;

typedef struct
{
    /***2023.07.05,将个数由18个变更为24个**/
    HYS_FAULT_TIME ScSelfFaultTimeInf[24];//必须与HysScSystemInfType中，个数一致。//18
}HysScSystemFaultTime;

/***定义系统中其他设备的实时状态***/
typedef struct
{
    u_int16_t StartPointId;//设备起始代号
    u_int16_t PointValue[HYS_GZM_OTHER_DEV_POINT_MAX];//设备特定点的具体数值
}HysGzmOtherDevStatus;

typedef struct
{
    HysGzmOtherDevStatus OtherDevInf[HYS_GZM_OTHER_DEV_NUMB_MAX];
}HysGzmOtherDevType;

/***********************************/
typedef struct
{
    HysScSystemInfType ScSelfStatus;
    V4AppSsSystemInfDownType   SsDownStatus;
    HysScSystemFaultTime    ScSelfStatusFaultTime;
    HysPersonInf PersonStatus;
    HysGzmOtherDevType OtherDevStatus;
}HysDykSystemInfType;

/**************************运行状态相关定义**************************************/
typedef struct
{
    u_int16_t IpAddrHigh;
    u_int16_t IpAddrLow;
    u_int16_t SsServerEnabled;
    u_int16_t SsServerCommErr;
    u_int16_t WirelessSigStrength;
    u_int16_t BuleTeethSigStrength;

    u_int16_t StopPosition;
    u_int16_t EndPosition;
    u_int16_t PersonLockLeftTime;
}HysRunScSelfType;

typedef struct
{
    u_int16_t LascEnabled;
    u_int16_t LascData;
}HysRunLascData;

typedef struct
{
    u_int16_t BackFlushEnabled;
    u_int16_t BackFlushScId;
}HysRunBackflushData;

typedef struct
{
    u_int16_t StartScId;
    u_int16_t EndScId;
}HysRunCommArea;

typedef struct
{
    u_int16_t Id;
    u_int16_t Value;
}HysRunActSensorDataType;

typedef struct
{
    u_int16_t ScSelfActFlag;
    u_int16_t ScWirelessCtrlFlag;
    u_int16_t OtherControlledScId;
    HysRunActSensorDataType OtherScSensorInf[RUN_OTHER_SC_ACT_SENSOR_NUMB_MAX];
}HysScCtrlInfType;

typedef struct
{
    u_int16_t Data[HYS_RUN_RESERVED_DATA_MAX];
}HysRunReservedType;

/*****/
typedef struct
{
    HysRunScSelfType ScRunInf;
    HysRunLascData LascInf;
    HysRunBackflushData FlushInf;
    HysRunCommArea CommInf;
    HysScCtrlInfType CtrlInf;
    HysRunReservedType ReservedInf;
}HysDykRunInfType;

/*********日志记录相关定义************/
/**日志记录所使用的数据类型**/
typedef struct
{
    QString time;
    QString Event;
    QString State;
}LoggerApiType;


typedef struct
{
    u_int8_t time[8];
    u_int8_t Event[30];
    u_int8_t State[16];
}LoggerRestoreType; //54

/*****/
typedef struct
{
    LoggerRestoreType log[HYS_LOGGER_RESTORE_MAX];
}HysDykLogInfType;

/**************************相关定义**************************************/
typedef struct
{
    HysScBasicInfType BasicInf;
    HysScPartType Part[HYS_PART_NUMB_MAX];
    HysScSensorType Sensor[HYS_SENSOR_NUMB_MAX];
    HysScExtDevType Extdev[HYS_EXTDEV_NUB_MAX];
    HysDykSystemInfType SystemInf;
    HysDykRunInfType RunInf;
    HysDykLogInfType LogInf;
}HysControllerInfType;


#endif // HYS_CONTROLER_TYPE_H
