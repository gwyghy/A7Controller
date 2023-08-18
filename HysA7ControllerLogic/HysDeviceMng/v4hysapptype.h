#ifndef V4_HYSAPP_TYPE_H
#define V4_HYSAPP_TYPE_H

#include "libhysa7parammng.h"

/**应用场合的定义**/
enum
{
    HYS_SYSTEM_INVALID = 0,
    HYS_SYSTEM_GONGZUO_MIAN = 1,
    HYS_SYSTEM_JIWEI_CHAOQIAN = 2,
    HYS_SYSTEM_ZIYI_JIWEI = 3,
    HYS_SYSTEM_PINGBAN_LICHE = 4,
    HYS_SYSTEM_JITOU_CHAOQIAN = 5,
    HYS_SYSTEM_POSUIJI = 6,
    HYS_SYSTEM_ZIYI_ZHUANZAI = 7,
    HYS_SYSTEM_STATION_8 = 8,
    HYS_SYSTEM_STATION_9 = 9,
    HYS_SYSTEM_STATION_10 = 10,
    HYS_SYSTEM_MAX = 10,
};

/***设备类型的定义***/
enum
{
    V4APP_DEV_TYPE_SC = 1,
    V4APP_DEV_TYPE_DYX = 2,
    V4APP_DEV_TYPE_OHQ = 3,
    V4APP_DEV_TYPE_TELEPHONE = 4,
    V4APP_DEV_TYPE_SS = 5,//内部使用
    V4APP_DEV_TYPE_MAX = V4APP_DEV_TYPE_SS,
    V4APP_DEV_TYPE_SS_REPORT = 0x1FE,//对外使用。专用于与外部设备的对接
};

/***设备主机与丛机的定义***/
enum
{
    V4APP_DEV_SS_MASTER = 1,
    V4APP_DEV_SS_SLAVE= 2
};

/***设备编号范围的定义***/
enum
{
    V4APP_DEV_ID_ALL = 0,
    V4APP_DEV_ID_MIN = 1,
    V4APP_DEV_ID_MAX = 250,
    V4APP_DEV_ID_IGNORE = 0xFFFF
};

enum
{
    BISUO_SUPPORT_NUM
};
/**设备群发时的地址***/
enum
{
    UDP_LINK_BROADCAST_ADDR = 0
};

enum
{
    V4APP_DEV_ID_YKQ = 251,//不能大于255!遥控器分配的虚拟架号,无实际意义，仅用于数据处理
    V4APP_DEV_ID_SERVER = 252,//不能大于255!遥控器分配的虚拟架号,无实际意义，仅用于数据处理
    V4APP_DEV_ID_CONSOLE = 253,//不能大于255!遥控器分配的虚拟架号,无实际意义，仅用于数据处理
    V4APP_DEV_ID_REMOTE = 254//不能大于255!遥控器分配的虚拟架号,无实际意义，仅用于数据处理
};

/*******包类型定义*******/
enum
{
    /****/
    V4APP_STATUS_GET_SINGLE_INQUIRE = 1,//
    V4APP_STATUS_GET_SINGLE_RESPONSE = 2,//
    V4APP_STATUS_GET_MULTIPLE_INQUIRE = 3,//
    V4APP_STATUS_GET_MULTIPLE_RESPONSE = 4,//

    V4APP_STATUS_REPORT = 5,//
    V4APP_STATUS_REPORT_ACK = 6,

    V4APP_STATUS_CTRL_DEV = 7,//
    V4APP_STATUS_CTRL_DEV_ACK = 8,

    V4APP_STATUS_MANUAL_ACT_HEARTBEAT = 9,
    V4APP_STATUS_LOG_REPORT = 10,

    V4APP_STATUS_DOWNLOAD_SYS_STATUS = 11,//

    V4APP_FOLLOWON_INQUIRE = 12,
    V4APP_FOLLOWON_RESPONSE = 13,

    V4APP_RECV_YKQ_DECODE = 14,//遥控器解除对码
    V4APP_MSGTYPE_RESERVED_15 = 15,
    V4APP_MSGTYPE_RESERVED_16 = 16,
    /****/
    V4APP_AUTOBACKFLUSH_ONOFF_INQUIRE = 17,//mry,22.10.13新增
    V4APP_AUTOBACKFLUSH_ONOFF_STATUS_DOWNLOAD = 18,//mry,22.10.13新增
    V4APP_TRANDEVSTATUS_INQUIRE = 19,//yangyue,23.4.6增加   下发其他设备的状态（用于控制器界面显示）
    V4APP_TERMINAL_INF_QUERY = 20,//yangyue,23.4.6增加   下发其他设备的状态（用于控制器界面显示）
    V4APP_TERMINAL_INF_REPORT = 21,//yangyue 光电终端上报状态
    /****51～100之间，暂定用于应用与数备管理(含驱动)间的通信。mry,2023.04.19*****/
    V4DRIVER_STATUS_GET_SINGLE_INQUIRE = 51,//读驱动状态请求，单一结构
    V4DRIVER_STATUS_GET_SINGLE_RESPONSE = 52,//读驱动状态应答，单一结构
    V4DRIVER_STATUS_GET_MULTIPLE_INQUIRE = 53,//读驱动状态请求，复合结构
    V4DRIVER_STATUS_SERVED_54 = 54,//预留

    V4DRIVER_ORIGIN_VALUE_REPORT = 55,//驱动上报原始数值
    V4DRIVER_STATUS_SERVED_56 = 56,//预留

    V4DRIVER_STATUS_CTRL_DEV = 57,//控制设备请求
    V4DRIVER_STATUS_CTRL_DEV_ACK = 58,//控制设备请求的应答

    V4DRIVER_STATUS_MANUAL_ACT_HEARTBEAT = 59,//控制驱动设备时维持心跳
    V4DRIVER_STATUS_RESERVED_60 = 60,//暂时未使用

    V4DRIVER_STATUS_GET_EXTDEV_STATUS_MULTIPLE = 61,//读驱动的外围设备状态，复合结构(所有)

    V4DRIVER_STATUS_SERVED_62 = 62,//暂时未使用
    V4DRIVER_STATUS_SERVED_63 = 63,//暂时未使用

    V4DRIVER_STATUS_SERVED_64 = 64,//暂时未使用
    V4DRIVER_REPORT_EXTDEV_STATUS = 65,//驱动上报外未设备的状态

    /****/
    V4APP_PARAM_GET_SINGLE_INQUIRE = 101,
    V4APP_PARAM_GET_SINGLE_RESPONSE = 102,
    V4APP_PARAM_GET_MULTIPLE_INQUIRE = 103,
    V4APP_PARAM_GET_MULTIPLE_RESPONSE = 104,
    V4APP_PARAM_SET_SINGLE_INQUIRE = 105,
    V4APP_PARAM_SET_SINGLE_RESPONSE = 106,
    V4APP_PARAM_SET_MULTIPLE_INQUIRE = 107,
    V4APP_PARAM_SET_MULTIPLE_RESPONSE = 108,
    V4APP_PARAM_CHECK_INQUIRE = 109,
    V4APP_PARAM_CHECK_RESPONSE = 110,

    V4APP_PARAM_SET_REPORT = 111,//参数修改上传,运行参数
    V4APP_SERVER_OPERRATE_REQUEST = 112,//服务类参数请求，进行设备的软件复位

    /*******/
    V4APP_DISPLAY_SS_PARAM_INQUIRE = 113,
    V4APP_DISPLAY_SS_PARAM_RESPONSE = 114,
    V4APP_REPORTSS_PARAM_CHANGGED = 115,
    V4APP_REPORTSS_PARAM_CHANGGED_RESPONSE = 116,
    V4APP_SET_LASCFINDLINE        = 117,//mry,22.10.13新增。V1.1.4及以上的版本，修改了LASC数据的格式并增加了应答
    V4APP_SET_LASCFINDLINE_ACK = 118,//mry,22.10.13新增。V1.1.4及以上的版本，修改了LASC数据的格式并增加了应答
    V4APP_SYSPARAM_SYNC_INQUIRE = 119,//mry,22.10.19新增。V1.1.4及以上的版本，系统级参数同步

    V4APP_REPORTSS_SYSTEM_CHANGGED = 185,//mry,23.06.09,系统参数修改请求，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    V4APP_REPORTSS_SYSTEM_CHANGGED_RESPONSE = 186,//mry,23.06.09,系统参数修改请求及应答，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）

    /**201之后，为网络维护。**/
    V4APP_FORCE_ENTER_SET_IP_STATUS_INQUIRE = 201,
    V4APP_FORCE_ENTER_SET_IP_STATUS_ACK = 202,
    V4APP_SET_DEV_IP = 203,//设置IP地址等。22.10.25，mry
    V4APP_NET_HEART_BEAT = 205,//网络心跳
    V4APP_SET_NETWORK_TIME = 207,//设置系统时间。22.10.25，mry
    V4APP_NEIGHBOUR_COMM_CHECK = 209,//设置系统时间。22.10.25，mry

    V4APP_RSYNC_FILE_INQUIRE = 211,//同步文件请求
};

/****控制器上报数据中子设备类型定义**/
/*上报状态的类型*/
enum
{
    REPORTENUM_ACTION=0,
    REPORTENUM_EMERGENCY=1,
    REPORTENUM_BISUO=2,
    REPORTENUM_STOP=3,
    REPORTENUM_FAIL=4,
    REPORTENUM_SENSOR=5,
    REPORTENUM_DEVICE=6,
    REPORTENUM_PERSON=7,
    REPORTENUM_SYSTEM=8,
    REPORTENUM_MAX
};

/**子设备的类型***/
enum
{
    ENUM_SINGLE_ACTION =0,
    ENUM_COMBINATION_ACTION=1,
    ENUM_GROUP_ACTION=2,
    ENUM_ALARM=3,
    ENUM_SENSOR=4,
    ENUM_PART=5,
    ENUM_DEVICE=6,
    ENUM_SYSTEM=7,
    ENUM_GROUP_ACTION_KZQ=8,
    ENUMTYPE_MAX
};

/**控制器处于的工作状态****/
enum
{
    SYSMODE_NORMAL =0,
    SYSMODE_SLAVE_CTRL,
    SYSMODE_CONFIG,
    SYSMODE_CONFIG_FIN,
    SYSMODE_MASTER_CTRL
};

/**控制器获取服务器参数，获取内容定义***/
enum
{
    V4APP_DISPLAY_PARAM_MENU_TEXT = 1,//菜单标题
    V4APP_DISPLAY_PARAM_MENU_NEXT_LIST = 2,//菜单的下一级菜单名称
    V4APP_DISPLAY_PARAM_MENU_CONTENT = 3//此菜单中的具体内容
};

/**参数修改的方式**/
enum
{
    V4APP_PARAMCHANGGE_METHOD_LOCAL = 0,
    V4APP_PARAMCHANGGE_METHOD_GLOBAL = 1
};

/**服务类操作的具体数值定义**/
enum
{
    AppProcServerType_reset_controller=1,
    AppProcServerType_reset_defaultParam,
    AppProcServerType_MAX
};

/******/
typedef struct
{
    ParamUnit	CtrlCmd;	   		//控制命令
    ParamUnit	CtrlMode;			//控制方式
    ParamUnit	CtrlSourceAddr;		//控制源地址
    ParamUnit	CtrlSourceType;		//控制源类型
    ParamUnit	UserID;				//用户ID
}ActionCtrlType;

typedef enum
{
    RunStateType_IDLE = 0,	// 0空闲状态
    RunStateType_ALARM,	    // 1当前为预警状态
    RunStateType_DELAY,	    // 2当前为延时状态
    RunStateType_STOP,	    // 3当前为停止状态
    RunStateType_STARTUP,	  // 4当前为启动状态
    RunStateType_RUN ,	    // 5当前为运行状态
    RunStateType_RUN_MANUAL,// 6当前为运行状态(手动时)
    RunStateType_SUSPEND ,	// 7当前为暂停状态
    RunStateType_SUSPEND_ACTION ,// 8当前为暂停状态,动作继续计时
    RunStateType_MAX
}Action_RunStateType;

typedef enum
{
//	ActionCtrl_CmdType_NONE=0,      //空命令
    ActionCtrl_CmdType_STARTUP=0,   //0直接启动
    ActionCtrl_CmdType_STOP,        //1停止
    ActionCtrl_CmdType_WAITSTARTUP, //2延时启动
    ActionCtrl_CmdType_SUSPEND,     //3暂停
    ActionCtrl_CmdType_RESUME,      //4恢复
    ActionCtrl_CmdType_SUSPEND_ACTION,//5暂停动作，继续计时
    ActionCtrl_CmdTypeMAX
}ActionCtrl_CmdType;

typedef enum
{
    ActionCtrl_ModeType_MANUAL =0,  //手动控制
    ActionCtrl_ModeType_AUTO        //自动控制
}ActionCtrl_ModeType;

typedef enum
{
    ActionCtrl_SourceType_CONTROLLER=0,	//控制器
    ActionCtrl_SourceType_HANDLER,		//遥控器
    ActionCtrl_SourceType_SERVER,		//服务器
    ActionCtrl_SourceType_CONSOLE,		//操作台
    ActionCtrl_SourceType_REMOTE,		//远控
    ActionCtrl_SourceType_MAX
}ActionCtrl_SourceType;

/***接收到的应用数据的类型***/
class HysSystemRecvShmClass
{
public:
    uchar childdevtype; //应用场合
    uchar devtype;      //设备类型
    ushort numb;        //设备编号
    ushort frametype;   //包类型
    uchar data[1024];    //数据
    int len;            //长度
};

/**共享内存的处理***/
typedef  int(*HysAppShmProcFuncType)(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

#endif // V4_HYSAPP_TYPE_H
