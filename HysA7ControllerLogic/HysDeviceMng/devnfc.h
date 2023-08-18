#ifndef DEVNFC_H
#define DEVNFC_H

/******/
#include "devglobaltype.h"

/******/
int DevNfcInit(void);
int DevNfcFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/***参数修改时，运行参数的同步***/
int DevNfcMsgRunParamInit(void);

#endif // DEVNFC_H
