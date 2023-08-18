#ifndef DEVBLUETEETH_H
#define DEVBLUETEETH_H

/******/
#include "devglobaltype.h"

/******/
int DevBlueTeethInit(void);
int DevBlueTeethFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/***参数修改时，运行参数的同步***/
int DevBlueTeethMsgRunParamInit(void);
#endif // DEVBLUETEETH_H
