#ifndef DEVKEY_H
#define DEVKEY_H

#include "devglobaltype.h"

/******/
int DevKeyInit(void);

/******/
int DevKeyFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/***参数修改时，运行参数的同步***/
int DevKeyMsgRunParamInit(void);

#endif // DEVKEY_H
