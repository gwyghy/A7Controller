#ifndef DEVINFRED_H
#define DEVINFRED_H

#include <sys/types.h>
#include "devglobaltype.h"

/***********/
#define     SELF_INFRED_ID_MAX  1
/**红外接收**/
/**红外接收的具体上报数据**/
typedef struct
{
    u_int16_t InfredId;//ID
    u_int16_t InfredStatus;//状态。
    u_int16_t InfredTimes;//次数。
}INFRED_DATA_TYPE;

/******/
int DevSelfInfredInit(void);
int DevSelfInfredFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/******/
int DevSelfInfredGetCommLedStatus(void);
int DevSelfInfredSetCommLedStatus(int ledstatus);

/***参数修改时，运行参数的同步***/
int DevSelfInfredMsgRunParamInit(void);
#endif // DEVINFRED_H
