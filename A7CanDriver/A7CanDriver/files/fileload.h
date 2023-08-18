#ifndef FILELOAD_H
#define FILELOAD_H

#include "zprint.h"




#define		FLASH_BULK_ERASER_TIME				(3)//块擦除时间
#define THE_DEV_PRG_BULK_SIZE		    	0x10000//块定义。64K
#define PROGRAM_INFO_SIZE  256

enum
{
    GUI_TYPE_SC = 0,
    GUI_TYPE_EMVD = 1,
    GUI_TYPE_ANGLE = 2,
    GUI_TYPE_PERSON = 3,
    GUI_TYPE_ALARM = 4,
    GUI_TYPE_HIGH = 5,
    GUI_TYPE_IR = 6,
    GUI_TYPE_KEYBOARD = 7,
    GUI_TYPE_WIRELESS = 8,
    GUI_TYPE_PROGRAM = 9,
    GUI_TYPE_PERSONBL = 10,
    GUI_TYPE_ALARMBL = 11,
    GUI_TYPE_HIGHBL = 12,
    GUI_TYPE_ALARM_UWB_ANGLE = 13,  //ghy 增加多功能声光显示类型
    GUI_TYPE_HUB =14 //ghy 增加HUB显示类型
};

//通讯协议中，版本信息的结构体定义。总共占用256个字节，64个字。低字节在前，高字节在后。
typedef struct
{
    uint32_t u32PrgDevType;//程序版本的设备类型
    uint32_t u32TargetBoardType;//目标板类型
    uint32_t u32PrgVer;//应用程序的版本号
    uint32_t u32PrgSize;//应用程序的大小
    uint32_t u32PrgDate;//应用程序的完成日期
    uint32_t u32PrgEncryptType;//应用程序的加密算法，0x00为无加密
    uint32_t u32PrgWriteBaseaddr;//程序写入的基地址
    uint32_t u32PrgCrc16;//应用程序的CRC校验值
    uint32_t reserved[55];//预留信息，52个字
    uint32_t u32VerInfCrc16;//整个版本信息程序的CRC校验，为CRC16，只占用其中的两个字节
}PROGRAM_VERSION_TYPE;


typedef struct
{
    PROGRAM_VERSION_TYPE programinfo;
    char fileName[128];
    uint32_t packetNum;
}
CAN_PROGRAM_UPDATE_DATA;

#endif // FILELOAD_H
