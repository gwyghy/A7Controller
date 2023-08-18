#ifndef UARTDATATYPE_H
#define UARTDATATYPE_H
#include <stdint.h>

//应答定义
enum
{
    NOACK=0,
    ACK
};

//接收处理函数指针
typedef  char (*RecvProcFunc)(UartFrame *pframe,void*arg);
//超时处理函数指针
typedef  void (*OverTimeProcFunc)(void *pframe);

//接收处理类型
typedef struct
{
    unsigned char  id;                      //帧类别
    unsigned short OverTime;                //超时事件
    RecvProcFunc RecvFuncCb;          //接收处理回调
    OverTimeProcFunc OverTimeFuncCb;  //超时处理回调
}DataProcType;

//程序更新结果上报类型
typedef struct
{
    uint16_t FileType;
    uint16_t DestDriveType;
    uint16_t DestDriveNUm;
    uint16_t Result;
}UpdataResType;

#endif // UARTDATATYPE_H
