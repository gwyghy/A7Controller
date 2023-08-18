#ifndef DEVWIRELESSMODULE_H
#define DEVWIRELESSMODULE_H

#include "devglobaltype.h"

/********无线模块********/
enum
{
    WIRELESS_MODULE_POINT_SINGLE_ACT = 0,//单动作
    WIRELESS_MODULE_POINT_ASSEMBLE_ACT = 1,//组合动作
    WIRELESS_MODULE_POINT_GRP_ACT = 2,//成组动作
    WIRELESS_MODULE_POINT_ALARM = 3,//报警
    WIRELESS_MODULE_POINT_FUNCTION_KEY = 99,//功能键
    /*********/
    WIRELESS_MODULE_POINT_CODE_MATCH = 11,//对码状态
    WIRELESS_MODULE_POINT_WIRELESS_BUS = 12,//无线总线
    WIRELESS_MODULE_POINT_WIRELESS_SIG_STRENGTH = 13,//无线信号轻度
    WIRELESS_MODULE_POINT_BLUE_SIG_STRENGTH = 14,//蓝牙信号强度
};

enum//对码状态的枚举
{
    WIRELESS_MODULE_CODE_MATCH_NONE = 0,//无
    WIRELESS_MODULE_CODE_MATCH_IR = 1,//红外对码
    WIRELESS_MODULE_CODE_MATCH_WIRELESS = 2,//无线对码成功
    WIRELESS_MODULE_CODE_MATCH_UNMATCED = 3,//解除对码
    /**下述与控制方式有关**/
    WIRELESS_MODULE_CTRL_SELF_SC = 4,//本架控制过程中
    WIRELESS_MODULE_CTRL_NEIGBOUR_SC = 5,//本架控制过程中
};

enum//功能键ID的枚举
{
    WIRELESS_MODULE_FUNCTION_KEY_ID_END = 1,
};

enum//功能键状态的枚举
{
    WIRELESS_MODULE_FUNCTION_KEY_STATUS_PREEEED = 1,
    WIRELESS_MODULE_FUNCTION_KEY_STATUS_UP = 2
};

/*****功能键状态的枚举***/
typedef struct
{
     u_int16_t KeyNumber;
     u_int16_t KeyStatus;
}WirelessModuleFuncKey_DataType;

/******/
int DevWirelessModuleInit(void);
int DevWirelessModuleFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/***通讯状态指示灯相关的处理***/
int DevWirelessModuleGetCommLedStatus(void);

/***通讯状态指示灯相关的处理***/
int DevWirelessModuleSetCommLedStatus(int ledstatus);

/***参数修改时，运行参数的同步***/
int DevWirelessModuleMsgRunParamInit(void);
#endif // DEVWIRELESSMODULE_H
