#ifndef DEVGLOBALTYPE_H
#define DEVGLOBALTYPE_H

#include "qglobal.h"
#include <QString>

/********程序编号定义******/
#define LOGIC_PROGRAM_ID         1
#define UI_PROGRAM_ID            2
#define UDP_PROGRAM_ID           21
#define CAN_PROGRAM_ID           31
#define SELF_AD_PROGRAM_ID       41
#define SELF_IO_PROGRAM_ID       51
#define SELF_UART_PROGRAM_ID     61
#define SELF_I2C_PROGRAM_ID      71

/*********本地，更新程序的存储路径,应用程序***************/
#define APP_PROGRAM_DOWN_FILE_PATH  "/opt/config/usr/download/app"
/*********本地，更新程序的存储路径,引导程序***************/
#define BOOT_PROGRAM_DOWN_FILE_PATH  "/opt/config/usr/download/boot"

/*********本地，更新程序的文件夹名称,除控制器文件外，不区分应用和引导程序***************/
#define SC_PROGRAM_APP_FILE_NAME                   "/DykA7ScSys-allinstall/"
#define SC_PROGRAM_BOOT_FILE_NAME                  "/A7ScSystemBoot/"

#define EMVD_PROGRAM_APP_BOOT_FILE_NAME            "/emvd/"
#define ANGLE_PROGRAM_APP_BOOT_FILE_NAME           "/angle/"
#define HEIGHT_PROGRAM_APP_BOOT_FILE_NAME          "/height/"
#define ALARM_PROGRAM_APP_BOOT_FILE_NAME           "/alarm/"
#define PERSON_PROGRAM_APP_BOOT_FILE_NAME          "/person/"
#define IRRX_PROGRAM_APP_BOOT_FILE_NAME            "/irrx/"
#define KEY_PROGRAM_APP_BOOT_FILE_NAME             "/key/"
#define WIRELESS_PROGRAM_APP_BOOT_FILE_NAME        "/wireless/"
#define HUB_PROGRAM_APP_BOOT_FILE_NAME             "/hub/"
#define HEIGHT_ANGLE_PROGRAM_APP_BOOT_FILE_NAME    "/height_angle/"
#define MULTI_ALARM_PROGRAM_APP_BOOT_FILE_NAME     "/multi_alarm/"
#define DATA_TRANSFER_PROGRAM_APP_BOOT_FILE_NAME   "/datatransfer/"
#define LIGHT_TRANSFER_PROGRAM_APP_BOOT_FILE_NAME  "/lighttransfer/"
#define LIGHT_TERMINAL_PROGRAM_APP_BOOT_FILE_NAME  "/lightterminal/"
#define TELEPHONE_PROGRAM_APP_BOOT_FILE_NAME       "/telephone/"

/*********远程控制器，更新程序的存储路径,应用程序以及引导程序***************/
#define SC_REMETE_LOGIN_USER                        "root"

/*********远程服务器，更新程序的存储路径,应用程序以及引导程序***************/
#define SS_REMETE_LOGIN_USER                        "root"
#define SS_APP_PROGRAM_DOWN_FILE_PATH               "/media/mmcblk0p1/ftpfile/app"
#define SS_BOOT_PROGRAM_DOWN_FILE_PATH              "/media/mmcblk0p1/ftpfile/boot"

/*********远程监控主机，更新程序的存储路径,应用程序以及引导程序***************/
#define MD_REMETE_LOGIN_USER                        "root"
#define MD_APP_PROGRAM_DOWN_FILE_PATH               "/cygdrive/c/rsync_server_file/app"
#define MD_BOOT_PROGRAM_DOWN_FILE_PATH              "/cygdrive/c/rsync_server_file/boot"

/*******驱动类型定义*******/
enum
{
    DRIVER_NET = 0,     //左右邻架网络/can，新增加
    DRIVER_CAN1=1,      //本架CAN1
    DRIVER_CAN2=2,      //本架CAN2
    DRIVER_AD=3,        //spi-ad
    DRIVER_IO = 4,      //本架IO
    DRIVER_UART2 = 5,   //本架UART2(键盘)
    DRIVER_UART3 = 6,   //本架UART3(无线)
    DRIVER_UART4 = 7,   //本架UART4(预留蓝牙)
    DRIVER_UART5 = 8,   //本架UART5(红外接收串口)
    DRIVER_I2C = 9,   //本架I2C(预留)
    DRIVER_MAX
};

/***硬件设备编号的ID，占用高半字节***/
enum
{
    HARDWARE_NUMBER_0 = 0x00,
    HARDWARE_NUMBER_1 = 0x20,
    HARDWARE_NUMBER_2 = 0x40,
    HARDWARE_NUMBER_3 = 0x60,
    HARDWARE_NUMBER_4 = 0x80,
    HARDWARE_NUMBER_5 = 0xa0,
    HARDWARE_NUMBER_6 = 0xc0,
    HARDWARE_NUMBER_7 = 0xe0
};

//设备RID,TID，设备号定义
enum
{
    EXTDEV_TYPE_SC = 0,
    EXTDEV_TYPE_EMVD = 1,			//电磁阀驱动器1
    EXTDEV_TYPE_CXB = 2,				//程序棒2
    EXTDEV_TYPE_ANGLE = 3,			//角度传感器5
    EXTDEV_TYPE_HIGH = 4,            //高度4
    EXTDEV_TYPE_ALARM = 5,			//声光报警器6
    EXTDEV_TYPE_PERSON = 6,          //人员7
    EXTDEV_TYPE_IRRX = 7,            //红外接收
    EXTDEV_TYPE_KEYBOARD = 8,        //键盘模块
    EXTDEV_TYPE_WIRELESS = 9,        //无线模块
    /***以下为新增加的设备编号***/
    EXTDEV_TYPE_HUB_1 = 10,          //HUB1
    EXTDEV_TYPE_HUB_2 = 11,          //HUB2
    EXTDEV_TYPE_HUB_3 = 12,          //HUB3
    EXTDEV_TYPE_ANGLE_HEIGHT = 13,   //角高一体
    EXTDEV_TYPE_MULTI_SOUND = 14,    //多功能声光
    EXTDEV_TYPE_SELF_AD = 15,        //本架模拟量
    EXTDEV_TYPE_SELF_IO  = 16,       //本架IO
    EXTDEV_TYPE_SELF_INFRED = 17,    //本架红外接收
    EXTDEV_TYPE_BLUE_TEETH = 18, //本架蓝牙模块
    EXTDEV_TYPE_NFC = 19,         //本架NFC
    EXTDEV_TYPE_DATA_CONVERTER  = 20,  //数据转换器
    EXTDEV_TYPE_SIGNAL_CONVERTER = 21,  //信号转换器
    EXTDEV_TYPE_LIGHT_ELECTRIL_TERMINAL = 22,  //光电终端
    EXTDEV_TYPE_PHONE = 23,               //电话
    EXTDEV_TYPE_MAX
};

/**********************以下为具体设备下子设备及点号的定义*****************************************/
/*******无需区分子设备号******/
enum
{
    DEV_CHILDDEV_ALL = 0,
    DEV_CHILDDEV_IGNOED = 0,
    DEV_CHILDDEV_1 = 1,
    DEV_CHILDDEV_2 = 2,
    DEV_CHILDDEV_3 = 3,
    DEV_CHILDDEV_4 = 4
};

enum//通用的点号定义
{
    DEV_POINT_ALL = 0,
    DEV_POINT_1,
    DEV_POINT_2,
    DEV_POINT_3,
    DEV_POINT_4,
    DEV_POINT_5,
    DEV_POINT_6,
    DEV_POINT_7,
    DEV_POINT_8,
    DEV_POINT_9,
    DEV_POINT_10,
    DEV_POINT_11,
    DEV_POINT_12,
    DEV_POINT_13,
    DEV_POINT_14,
    DEV_POINT_15,
    DEV_POINT_16,
    DEV_POINT_17,
    DEV_POINT_18,
    DEV_POINT_19,
    DEV_POINT_20,
    DEV_POINT_21,
    DEV_POINT_22,
    DEV_POINT_23,
    DEV_POINT_24,
    DEV_POINT_25,
    DEV_POINT_26,
    DEV_POINT_27,
    DEV_POINT_28,
    DEV_POINT_29,
    DEV_POINT_30,
    DEV_POINT_31,
    DEV_POINT_32
};

enum//通用的点控制每命令
{
    DEV_POINT_CTRL_ON = 1,//点开启
    DEV_POINT_CTRL_OFF = 2//点关闭
};

enum
{
    DEV_FALUT_NOMAL = 0,
    DEV_FALUT_HAPPEN = 1,
};

typedef enum
{
    OnlineType_OFFLINE =0,
    OnlineType_ONLINE
}OnlineType;

typedef enum
{
//	BaseCtrlType_NONE=0,
    BaseCtrlType_STARTUP=1,
    BaseCtrlType_STOP=0
}BaseCtrlType;

typedef enum
{
    BaseRunType_STOP =0,
    BaseRunType_RUN
}BaseRunType;

/**煤机运行方向**/
enum
{
    MINE_RUN_LITTLE_TO_BIG = 1,
    MINE_RUN_BIG_TO_LITTLE = 2,

    MINE_RUN_DIR_INVALID = 3
};

/***驱动上报的设备状态***/
typedef struct
{
    u_int16_t Status;//设备在线状态//(0离线1在线)
    u_int16_t Value;
    u_int16_t AppVersionHigh;
    u_int16_t AppVersionLow;
    u_int16_t BootVersionHigh;
    u_int16_t BootVersionLow;
}DriverExtDevStatusType;

/***定义驱动类型与程序编号及硬件设备号的关系****/
typedef struct
{
    uchar ProgramId;//程序编号
    uchar DeviceId_HardWareNumber;//设备号中的高半字节
 }DevDriveIdTransferType;

/***接收到的应用数据的类型***/
class HysDevRecvShmClass
{
public:
    uchar driverid;     //驱动号
    uchar devid;        //设备号
    uchar childdevid;   //子设备号
    ushort point;       //点号
    ushort frametype;   //包类型
    uchar data[50];    //数据
    int len;            //长度
};

/***文件更新时的存储路径***/
typedef struct
{
    u_int16_t DownPrgType;//程序更新类型
    u_int16_t DownDevType;//设备类型，与UI选择的范围一致
    QString DownFilePath;//文件的存储路径
    QString DownFileName;//文件的存储名称
    u_int16_t DriverTypeId;//所对应的驱动的ID号
    u_int16_t DevTypeId;//所对应的设备的类型号
}AppUpdateFilePathType;

/**消息的处理***/
typedef  int(*HysMsgProcFuncType)(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len);


/**共享内存的处理***/
typedef  int(*HysDevShmProcFuncType)(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);
typedef  int(*HysDevFrameProcFuncType)(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len);

/*!
 * \brief DevGetUpdateFilePath
 * \param DownPrgType,更新程序类型
 * \param DownDevType，设备类型，与UI选择的范围一致
 * \param path，文件的存储路径
 * \param DriverTypeId,所对应的驱动的ID号
 * \param DevTypeId,所对应的设备的类型号
 * \return 0正常-1异常
 */
int DevGeneralGetUpdateFilePath(u_int16_t DownPrgType , u_int16_t DownDevType, QString &filepath, QString &filename, u_int16_t &DriverTypeId, u_int16_t &DevTypeId);
/*!
 * \brief DevGeneralIsUpdateFilePathAvalid,判断文件地址的有效性
 * \param pSrcAdd,源地址
 * \param SrcAddrSize,源地址大小
 * \param pDestAddr,目的地址
 * \param DestAddrSize,目的地址大小
 * \return true正确，false异常
 */
bool DevGeneralIsUpdateFilePathAvalid(char *pSrcAdd,int SrcAddrSize,char *pDestAddr,int DestAddrSize);

/*!
 * \brief DevGeneralGetUpdateFilePathRelatedInf,获取此目的地址，所对应的设备类型驱动类型等相关信息
 * \param pDestAddr,目的地址
 * \param DestAddrSize,目的地址大小
 * \param DownPrgType,下载程序类型
 * \param DownDevType，下载设备类型
 * \param DriverTypeId，驱动类型
 * \param DevTypeId，所对应的设备的类型号
 * \return
 */
bool  DevGeneralGetUpdateFilePathRelatedInf(char *pDestAddr,int DestAddrSize,\
                                            u_int16_t &DownPrgType,
                                            u_int16_t &DownDevType,
                                            u_int16_t &DriverTypeId,
                                            u_int16_t &DevTypeId);

/******/
int DevFrameDriverOriginValueReportGeneralProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len);
int DevFrameDriverExtdevStatusReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len);

/******/
int DevGeneralInit(void);
int DevGeneralFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/***参数修改时，运行参数的同步***/
int DevGeneralMsgRunParamInit(void);

#endif // DEVGLOBALTYPE_H
