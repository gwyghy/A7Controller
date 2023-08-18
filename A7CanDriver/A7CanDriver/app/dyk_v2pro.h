#ifndef __DYK_V2PRO_H__
#define __DYK_V2PRO_H__

#include "can_bus.h"
#include "can_protocol.h"
#include <stdint.h>
#include "dyk_v2callback.h"

typedef int (*SetShmRecvProc)(uint16_t type, uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len);


enum
{
    /********************EMVD********************/
    READ_DRIVER_STATE_REQUEST = 0x51,     //0x51(读驱动状态请求)
    READ_DRIVER_STATE_CONFIRM = 0x52,     //0x52(读驱动状态应答)
    CONTROLL_DRIVER_REQUEST = 0x57,       //0x57(控制驱动)
    CONTROLL_DRIVER_HEART_REQUEST = 0x59, //0x59(控制设备时维持心跳)
    READ_DEVICE_STATE_REQUEST = 0x61,     //0x61(读驱动设备状态)
    REPORT_DEVICE_STATE_REQUEST = 0x65,   //0x65(驱动上报设备状态)
    /********************ANGLE********************/
    REPORT_DEVICE_RAW_VALUE = 0X55,      //0x55(驱动上报原始数值)
};

enum
{
    FILE_CONFIG_SC = 1,
    FILE_VALUE_SC = 2,
    FILE_PROGRAM_APP = 3,
    FILE_PICTURE = 4,
    FILE_AUDIO = 5,
    FILE_PROGRAM_BOOT = 6,
    FILE_CONFIG_GB2312 = 101,
    //1属性文件2数值文件3程序文件4图片5音频101属性文件
};

enum
{
    //253传输超时；254文件错误；255失败，后跟失败的架号(多个)
    FILE_UPDATE_SUCCESS = 100,
    FILE_UPDATE_DEVICE_OFFLINE = 251,
    FILE_UPDATE_FILE_ABSENT = 252,
    FILE_UPDATE_TIMEOUT = 253,
    FILE_UPDATE_FILE_ERROR = 254,
    FILE_UPDATE_FAIL = 255
};


typedef struct
{
    uint8_t  forwardTimes;
    uint8_t  workMode;
    uint16_t interval;
    uint8_t  RxID;
    uint8_t  angleVariation;
    uint8_t  RxSubID;
    uint8_t  reserve;
}
stAngleParam;


typedef struct
{
    uint16_t scNum;//支架编号
    union{
        struct{
            unsigned int  workMode       :1;  //1主动上报
            unsigned int  calcMode       :1;  //1水平距离
            unsigned int	disVariation   :2;	//0：变化距离20cm
            unsigned int	trsPower       :4;	//发送功率

        } WORKPARAM;
        uint8_t u8workparam;
    } u8WORKPARAM;
    uint8_t	     interval;		//上报间隔0.1s
    uint16_t	     range;		  //检测范围cm
    uint16_t	     scHigh;	  //支架高度
}
stPersonParam;


typedef struct
{
    uint8_t device;
    uint8_t subDevice;
    uint8_t point;
    uint16_t fileType;
    uint16_t deviceType;
    uint8_t deviceNum;
    uint8_t filePath[50];
    uint16_t fileCrc;
    uint16_t calcCrc;
    uint32_t fileSize;
    uint32_t verson;
    uint8_t cxb_recv_device;
    uint8_t fileName[40];
}
STR_CAN_RECV_PROGRAM;


uint16_t getSupprotNum(void);
uint16_t getCheckDis_Person(void);
uint16_t getReportInter_Person(void);

uint16_t getWirelessEnable(void);
uint16_t getSupportNumIncDir(void);
uint16_t getMasterControllTimer(void);
uint16_t getNumberShelves(void);

void StopEmvdAction(void);
void StopAlarmAction(void);
void StopEmvdAction_Offline(uint8_t driverId);
void RecvShmCallback_Controll(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len);
void RecvShmCallback_ValueCheck(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len);
void RecvShmCallback_ControllHeart(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len);
void RecvShmCallback_ReadDeviceState(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len);
void RecvShmCallback_ReadAllDeviceState(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len);
void RecvMsgCallback_InitDevice(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len);
void RecvMsgCallback_CXBLoadProgram(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len);
void CanSendDataProc(char* name, uint8_t device,uint8_t fileType);
void RecvMsgCallback_UpdateProgram(uint8_t device, uint8_t subDevice, uint8_t point, uint8_t* data, int len);

class dyk_can
{
public:
     ncan_protocol  *    pro_p;          //使用的can协议指针    ncan_protocol  *    pro_p;          //使用的can协议指针

public:

    dyk_can(ncan_protocol * pro);
    int dyk_driver_config_init(void);

    ~dyk_can();
};

#endif
