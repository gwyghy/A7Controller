#ifndef HYSMSG
#define HYSMSG

#include <sys/types.h>


//消息子类型（2）+子数据长度（2）+具体数据
enum
{
    MSG_DRIVER_STATUS = 0, //驱动号（1）+状态（0/1）
    MSG_RESERVED_1 = 1, //预留功能,功能待定。控制器中不涉及
    MSG_TRAN_FILE = 2, //传文件并立即更新，文件类型(2)+目的设备类型(2)+目的设备编号(2)+文件绝对路径及名称
    MSG_TRAN_FILE_ACK = 3, //传文件结果，文件类型(2)+目的设备类型(2)+目的设备编号(2)+结果(2个字节。0~99,传输过程中；100完成；254文件错误；255失败，后跟失败的架号(多个))

    /***两个接收文件，上报结果方式都可以***/
    MSG_RECV_FILE_OK = 4, //接收文件，文件类型(2)+目的设备类型(2)+目的设备编号(2)+文件绝对路径及名称
    MSG_RECV_FILE_ACK = 5, //接收文件结果，文件类型(2)+目的设备类型(2)+目的设备编号(2)+结果(2个字节。0~99,传输过程中；100完成；254文件错误；255失败，后跟失败的架号(多个))

    MSG_DOWNLOAD_FILE = 6,//下载文件请求,文件类型(2)+目的设备类型(2)+目的设备编号(2)
    MSG_DOWNLOAD_FILE_ACK = 7,//下载文件请求的应答,文件类型(2)+目的设备类型(2)+目的设备编号(2)+结果()
    MSG_INITIAL_UDP_DRIVER = 8,//初始化设备，并进行查询。
    MSG_DEVICE_MAP_INQUIRE = 9,//请求MAP表，确认（1）。控制器中不涉及
    MSG_DEVICE_MAP_REPORT = 10,//MAP表数据上报.控制器中不涉及 。设备总数+当前包设备起始+当前包设备结束+设备数据(4个字节)(每一包最多100个设备)

    /****/
    MSG_REPORT_ADDR_CONFIG_MODE = 11,//上报配址模式
    MSG_REPORT_ADDR_CONFIG_RESULT = 12,//上报配址模式最终的结果
    MSG_REPORT_ERR_STATUS = 13,//异常信息上报

    /****/
    MSG_CHILDDEV_INF_REPORT = 14,//子设备信息上报。专用于数据转换器
    MSG_ENTER_COMM_AREA_CHECK = 15,//通知v4driver进入通讯范围检测
    MSG_REPORT_COMM_AREA_CHECK_RESULT = 16,//通知v4driver进入通讯范围检测

    MSG_ENTER_RSYNC_FILE_CHECK = 17,//通知v4driver进入文件同步
    MSG_REPORT_RSYNC_FILE_CHECK_RESULT = 18,//通知v4driver文件同步的具体结果
    MSG_APP_MAX = MSG_REPORT_RSYNC_FILE_CHECK_RESULT

};

/***配址模式相关**/
enum
{
    MSG_ADDR_CONFIG_MODE_ENTER =1,
    MSG_ADDR_CONFIG_MODE_EXIT =2,
};

enum
{
    MSG_ADDR_CONFIG_RESULT_INVALID =0,
    MSG_ADDR_CONFIG_RESULT_SUCCEED =1,
    MSG_ADDR_CONFIG_RESULT_TIMEOUT=2
};

/***异常信息相关**/
enum
{
    MSG_ERR_GLOBAL_PARAM_DISAGREE = 1,
    MSG_ERR_NEIGHBOUR_DEVIICE_DISAGREE =2,
    MSG_ERR_NEIGHBOUR_DEVIICE_COMM_ERR = 3
};

enum
{
    MSG_ERR_STATUS_HAPPEN = 1,
    MSG_ERR_STATUS_RESTORE = 2
};

/***接收相关***/
int HysMsgRxDriverStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);
int HysMsgRxTransFileStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);
int HysMsgRxRecvFileOkStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);
int HysMsgRxRecvFileIngDetailStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);
int HysMsgRxReportAddrConfigMode(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);
int HysMsgRxReportAddrConfigResult(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);
int HysMsgRxReportErrStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);
int HysMsgRxCommAreaCheckResult(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);
int HysMsgRsyncFileCheckResult(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);

/***发送相关***/
int HysMsgTxTransFileProc(u_int8_t driverid, u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len);
int HysMsgTxDownLoadFileProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len);
int HysMsgTxInitDevInfProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len);

/*****/
/***进入配址模式相关的***/
int HysMsgTxEnterSetIpConfigProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len);
/***进入通讯范围检测相关的***/
int HysMsgTxEnterCommAreaCheckProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len);
/***进入同步文件相关的***/
int HysMsgTxRSyncFileProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len);

/***获取相关***/
/***获取是否通过消息形式，已完成了配地址功能***/
bool HysMsgGetAddrConfigResultFlagAndClear(void);
#endif // HYSMSG

