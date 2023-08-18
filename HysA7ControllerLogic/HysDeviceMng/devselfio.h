#ifndef DEVSELFIO_H
#define DEVSELFIO_H

#include "devglobaltype.h"

/********本架I/O*********/
enum
{
    /***输入相关点***/
    SELF_IO_POINT_STOP = 1,//急停
    SELF_IO_POINT_LOCK = 2,//闭锁
    SELF_IO_POINT_STATUS_CHECK = 3,//状态识别,对应用来说无意义
    SELF_IO_POINT_B12V  =   4,//B12V输出点控制
    /***输出相关点***/
    SELF_IO_POINT_LED_COMM_TRS = 11,//指示灯,通信发送
    SELF_IO_POINT_LED_COMM_RECV = 12,//指示灯,通信接收
    SELF_IO_POINT_LED_STOP = 13,//指示灯，急停闭锁
    SELF_IO_POINT_LED_MATCH_WIRELESS = 14,//指示灯，对码无线已成功
    SELF_IO_POINT_LED_MATCH_IR = 15,//指示灯，红外对码成功
    SELF_IO_POINT_LED_COMM_WIRELESS = 16,//蓝色指示灯,无线通信
    SELF_IO_POINT_LED_7 = 17,//指示灯
    SELF_IO_POINT_LED_8 = 18,//指示灯
    SELF_IO_POINT_LED_9 = 19,//指示灯
    SELF_IO_POINT_LED_10 = 20,//指示灯
    SELF_IO_POINT_BUZZER = 21,//蜂鸣器
};

enum//蜂鸣器工作模式的枚举
{
    SELF_IO_POINT_BUZZER_MODE_NONE = 0,//无任何声音
    SELF_IO_POINT_BUZZER_KEY_PRESSED = 1,//按键音
    SELF_IO_POINT_BUZZER_NEAR_SELECTED = 2,//邻隔架选中
    SELF_IO_POINT_BUZZER_AUTO_ACTION = 3,//自动动作
    SELF_IO_POINT_BUZZER_LOCKED = 4,//急停闭锁
    SELF_IO_POINT_BUZZER_UNLOCKED = 5,//急停闭锁解除
    SELF_IO_POINT_BUZZER_END_PRESSED = 6//停止键
};

/******/
int DevSelfIoInit(void);
int DevSelfIoFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/***参数修改时，运行参数的同步***/
int DevSelfIoMsgRunParamInit(void);

#endif // DEVSELFIO_H
