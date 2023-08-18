#ifndef DEVEMVD_H
#define DEVEMVD_H

/******/
#include "devglobaltype.h"
#include <QList>

/******/
enum
{
    EmvdPointStatusType_Normal = 1,
    EmvdPointStatusType_Cutdown = 2,
    EmvdPointStatusType_Short = 3,
    EmvdPointStatusType_TimeOut = 4,
    EmvdPointStatusType_NotSet = 4,
}enumEmvdPointStatusType;

/******/
int DevEmvdInit(void);
int DevEmvdFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/*****/
int DevEmvdInsertRealTimeActList(u_int16_t pointid);
int DevEmvdDeleteRealTimeActList(u_int16_t pointid);
int DevEmvdClearRealTimeActList(void);
int DevEmvdGetRealTimeActList(QList<u_int16_t> &list);
#endif // DEVEMVD_H
