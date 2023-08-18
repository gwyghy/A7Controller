#ifndef APP_MSGTYPE_H
#define APP_MSGTYPE_H

#include <sys/types.h>

/*******/
#define APP_MSG_ENTITY_LENGTH   1012

/***设备类型定义**/
enum
{
    MSG_DEV_TYPE_SC = 1,//控制器
    MSG_DEV_TYPE_DYX = 2,//数据转换器
    MSG_DEV_TYPE_OHQ = 3,//耦合器
    MSG_DEV_TYPE_TELEPHONE = 4,//电话
    MSG_DEV_TYPE_SS_REPORT = 0x1FE,//服务器，对外使用。专用于与外部设备的对接
    MSG_DEV_TYPE_MAX = MSG_DEV_TYPE_SS_REPORT
};

//define type
enum
{
    /***数据采集控制相关，1-100**/
    MSG_APP_CTRL_DEV = 7,//动作控制，时间戳(8)+设备类型(2)+设备编号(2)+原有控制指令（见原协议）
    MSG_APP_CTRL_DEV_ACK = 8,//动作控制应答，时间戳(8)+设备类型(2)+设备编号(2)+原有控制指令（见原协议）

    MSG_MANUAL_ACT_HEARTBEAT = 9,//单动作控制手动心跳，时间戳(8)+设备类型(2)+设备编号(2)+原有心跳数据（见原协议）

    MSG_APP_FOLLOWON_INQUIRE = 12,//开启跟机，时间戳(8)+设备类型(2)+设备编号(2)+原有控制指令（见原协议）
    MSG_APP_FOLLOWON_RESPONSE = 13,//开启跟机应答，时间戳(8)+设备类型(2)+设备编号(2)+原有控制指令（见原协议）

    MSG_APP_AUTOBACKFLUSH_ONOFF_INQUIRE = 17,//反冲洗启动或关闭，时间戳(8)+设备类型(2)+设备编号(2)+原有控制指令（见原协议）
    MSG_APP_AUTOBACKFLUSH_ONOFF_RESPONSE = 19,//反冲洗启动或关闭应答，时间戳(8)+设备类型(2)+设备编号(2)+原有控制指令（见原协议）

    MSG_APP_KEY_LED_STATUS_CTRL = 99,//控制按键指示灯状态，时间戳(8)+设备类型(2)+设备编号(2)+相关数据
    MSG_APP_STOP_KEY_STATUS = 100,//停止键，时间戳(8)+设备类型(2)+设备编号(2)+相关数据

    /***参数传输相关，101-200***/
    MSG_DISPLAY_DEV_PARAM_INQUIRE = 113,//显示设备参数请求，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    MSG_DISPLAY_DEV_PARAM_RESPONSE = 114,//显示设备参数应答，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    MSG_REPORTSS_PARAM_CHANGGED = 115,//设备参数修改请求，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    MSG_REPORTSS_PARAM_CHANGGED_RESPONSE = 116,//设备参数修改应答，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    /*****/
    MSG_DISPLAY_SYSTEM_PARAM_INQUIRE = 183,//显示系统参数请求，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    MSG_DISPLAY_SYSTEM_PARAM_RESPONSE = 184,//显示系统参数请求，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    MSG_REPORTSS_SYSTEM_CHANGGED = 185,//系统参数修改请求，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    MSG_REPORTSS_SYSTEM_CHANGGED_RESPONSE = 186,//系统参数修改应答，时间戳(8)+设备类型(2)+设备编号(2)+原有协议数据（见原协议）
    /*****/
    MSG_REPORT_SERVICE_PARAM_CHANGGED = 187,//上报服务类参数修改(含密码)，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议

    MSG_REPORT_KEYCONFIG_PARAM_CHANGGED = 189,//上报按键配置参数修改，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议

    MSG_REPORT_ASSINTCONFIG_PARAM_CHANGGED = 191,//上报辅助菜单参数修改，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议

    MSG_REPORT_HOMEDISPLAY_PARAM_CHANGGED = 193,//上报主界面显示参数修改，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议

    MSG_REPORT_GRPKEYCONF_PARAM_CHANGGED = 195,//上报成组配置菜单参数修改，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议

    MSG_REPORT_SINGLE_ACTPARAM_CHANGGED = 196,//上报单动作中，动作参数的修改，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议
    MSG_REPORT_COMMB_ACTPARAM_CHANGGED = 197,//上报组合动作中，动作参数的修改，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议
    MSG_REPORT_GRP_ACTPARAM_CHANGGED = 198,//上报成组动作中，动作参数的修改，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议

    /***网络维护及文件传输相关，201-300.以下为自定义***/
    MSG_SERVICE_OPERATE_INF = 201,//进行了服务类的操作，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议

    MSG_DOWNLOAD_DEV_PRG_CXB = 202,//进行文件下载(程序棒)，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议
    MSG_DOWNLOAD_DEV_PRG_CXB_ACK = 203,//进行文件下载(程序棒)，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议
    MSG_DOWNLOAD_DEV_PRG_REMOTE = 204,//进行文件下载(远程)，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议
    MSG_DOWNLOAD_DEV_PRG_REMOTE_ACK = 205,//进行文件下载(远程)，时间戳(8)+设备类型(2)+设备编号(2)+自定义协议

    MSG_UPDATE_DEV_PRG = 206,//进行程序更新，时间戳(8)+设备类型(2)+设备编号(2)+文件类型(2)+设备类型(2)+获取的方向(2)+文件绝对路径及名称
    MSG_UPDATE_DEV_PRG_ACK = 207,//进行程序更新应答，时间戳(8)+设备类型(2)+设备编号(2)+文件类型(2)+设备类型(2)+获取的方向(2)+文件绝对路径及名称

    MSG_TRANS_FILE_RESULT_REPORT = 208, //传文件结果，时间戳(8)+设备类型(2)+设备编号(2)+文件类型(2)+目的设备类型(2)+目的设备编号(2)+结果(2个字节。0~99,传输过程中；100完成；254文件错误；255失败，后跟失败的架号(多个))
    MSG_RECV_FILE_RESULT_REPORT = 209, //接收文件，时间戳(8)+设备类型(2)+设备编号(2)+文件类型(2)+源设备类型(2)+源设备编号(2)+文件绝对路径及名称

    /***以下为网络维护相关的定义***/
    /***2023.07.11,新增加的获取光电终端及数据转换器信息的接口***/
    MSG_CHECK_COMM_AREA_INQUIRE = 301,//获取通讯范围
    MSG_CHECK_DATA_TRANS_INF_INQUIRE = 302,//检查数据转换器的相关信息
    MSG_CHECK_DATA_TRANS_INF_RESPONSE = 303,//检查数据转换器的相关信息，的应答.暂不实现
    MSG_CHECK_EMVD_POINT_STATUS_INQUIRE = 304,//进行阀检测
    MSG_CHECK_EMVD_POINT_STATUS_RESPONSE  = 305,//进行阀检测，应答数据
    MSG_CHECK_TERMINAL_VALTAGE_FRE_INQUIRE = 306,//立即查询光电终端的电压及频率
    MSG_TYPE_MAX
};

/***本架服务类相关的操作***/
enum
{
    MSG_SERVICE_OPERATE_RESET_SC = 1,
    MSG_SERVICE_OPERATE_RESET_SS = 2,
    MSG_SERVICE_OPERATE_TRANS_EMVD_IO_CONFIG = 3,
    MSG_SERVICE_OPERATE_TRANS_SENSOR_CONFIG = 4,
    MSG_SERVICE_OPERATE_TRANS_ASSIST_KEY = 5,
    MSG_SERVICE_OPERATE_TRANS_KEY_CONFIG = 6,
    MSG_SERVICE_OPERATE_TRANS_GLOBAL_PARAM = 7,
    MSG_SERVICE_OPERATE_TRANS_ALL_PARAM = 8,
    MSG_SERVICE_OPERATE_RESTORE_DEFAULT_PARAM = 9,
};

/**参数修改/或操作的方式****/
enum
{
    CHANGED_METHOD_SELF = 0,
    CHANGED_METHOD_ALL = 1,
    CHANGED_METHOD_LEFT = 2,
    CHANGED_METHOD_RIGHT = 3
};

/**程序更新，程序类型的方式****/
enum
{
    UPDATE_PRG_PRGTYPE_CONFIG = 1,//属性文件
    UPDATE_PRG_PRGTYPE_VALUE = 2,//数值文件
    UPDATE_PRG_PRGTYPE_PROGRAM = 3,//程序文件
    UPDATE_PRG_PRGTYPE_PICTURE = 4,//图片文件
    UPDATE_PRG_PRGTYPE_SOUND = 5,//音频
    UPDATE_PRG_PRGTYPE_BOOT = 6,//引导程序
    UPDATE_PRG_PRGTYPE_MAX = UPDATE_PRG_PRGTYPE_BOOT,

    UPDATE_PRG_PRGTYPE_CONFIG_GB2312 = 101,//GB2312版本的属性文件。A7中不考虑
};

/**程序更新获取来源的方式****/
enum
{
    UPDATE_PRG_METHOD_SELF = 0,//本架
    UPDATE_PRG_METHOD_ALL = 1,//全部
    UPDATE_PRG_METHOD_LEFT = 2,//从左侧
    UPDATE_PRG_METHOD_RIGHT = 3//从右侧
};

/***程序更新，设备类型****/
enum
{
    UPDATE_PRG_DEVTYPE_SC = 0,              //控制器
    UPDATE_PRG_DEVTYPE_EMVD = 1,			//电磁阀驱动器
    UPDATE_PRG_DEVTYPE_ANGLE = 2,			//角度传感器
    UPDATE_PRG_DEVTYPE_HIGH = 3,            //高度
    UPDATE_PRG_DEVTYPE_ALARM = 4,			//声光报警器
    UPDATE_PRG_DEVTYPE_PERSON = 5,          //人员
    UPDATE_PRG_DEVTYPE_IRRX = 6,            //红外接收
    UPDATE_PRG_DEVTYPE_KEYBOARD = 7,        //键盘模块
    UPDATE_PRG_DEVTYPE_WIRELESS = 8,        //无线模块
    /***以下为新增加的设备编号***/
    UPDATE_PRG_DEVTYPE_HUB = 9,          //HUB

    UPDATE_PRG_DEVTYPE_ANGLE_HEIGHT = 10,   //角高一体
    UPDATE_PRG_DEVTYPE_MULTI_SOUND = 11,    //多功能声光
    UPDATE_PRG_DEVTYPE_DATA_CONVERTER  = 12,  //数据转换器
    UPDATE_PRG_DEVTYPE_SIGNAL_CONVERTER = 13,  //信号转换器
    UPDATE_PRG_DEVTYPE_LIGHT_ELECTRIL_TERMINAL = 14,  //光电终端
    UPDATE_PRG_DEVTYPE_PHONE = 15,

    UPDATE_PRG_DEVTYPE_MAX = 16
};

//消息实体
typedef struct
{
     long long int dt;          //8Byte
     u_int16_t devtype;         //2Byte
     u_int16_t devid;           //2Byte
     u_int8_t  data[APP_MSG_ENTITY_LENGTH];      //1012Byte
}MsgEntity;

/**消息的处理***/
typedef  int(*AppMsgProcFuncType)(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
/**消息的发送***/
typedef  int(*HysAppSendMsgProcFuncType)(void);

#endif // APP_MSGTYPE_H

