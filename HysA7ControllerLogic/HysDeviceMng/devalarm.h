#ifndef DEVALARM_H
#define DEVALARM_H

/******/
#include "devglobaltype.h"

/**声光报警器**/
/**点号定义**/
enum
{
    ALARM_POINT_NO = 0,
    ALARM_POINT_BEEP = 1,
    ALARM_POINT_LED = 2,
    ALARM_POINT_BEEP_LED = 3
};

/***声报警方式***/
enum
{
    ALARM_BEEP_NEVER  = 0,
    ALARM_BEEP_ALWAYS = 1,
    ALARM_BEEP_200400 = 2,
    ALARM_BEEP_100200 = 3,
};

/***光报警方式****/
enum
{
    ALARM_BLUE_RED_NEVER  = 0,
    ALARM_BLUE_RED_ALWAYS = 1,
    ALARM_BLUE_ALWAYS     = 2,
    ALARM_RED_ALWAYS      = 3,
    ALARM_RED20_BLUE20    = 4,
};

/******/
int DevAlaramInit(void);
int DevAlaramFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

#endif // DEVSOUND_H
