#ifndef DEVSELFAD_H
#define DEVSELFAD_H

#include "devglobaltype.h"

/********本架模拟量*********/
enum
{
    SELF_AD_POINT_SENSOR_1 = 1,
    SELF_AD_POINT_SENSOR_2 = 2,
    SELF_AD_POINT_SENSOR_3 = 3,
    SELF_AD_POINT_SENSOR_4 = 4,
    SELF_AD_POINT_BOARD_VOLTAGE = 5,
    SELF_AD_POINT_BOARD_CURRENT = 6
};

/******/
int DevSelfAdInit(void);
int DevSelfAdFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/***参数修改时，运行参数的同步***/
int DevSelfAdMsgRunParamInit(void);

#endif // DEVSELFAD_H
