#ifndef DEVPERSON_H
#define DEVPERSON_H

/******/
#include <sys/types.h>
#include "devglobaltype.h"


/**人员定位标签**/
#define    	PERSONLABLE_PROC_TIMER_MIN      1000//最少每1秒上报一次数据

#define     PERSON_LABLE_NUMB_MAX           11//修改为11个。V1.0.1

/*****/
#define     PERSONLABLE_ID_MIN              301//人员标签的最小值
/*****/
#define     UWB_IR_SEND_NUMB_MIN            291
#define     UWB_IR_SEND_NUMB_MAX            299

/**人员标签状态**/
enum
{
    PERSON_LABLE_STATUS_INVALID = 0,//无效
    PERSON_LABLE_STATUS_ENTER = 1,//进入
    PERSON_LABLE_STATUS_EXIT = 2,//离开
    PERSON_LABLE_STATUS_FAULT = 3//故障
};

/**人员标签状态,是否处于呼救状态,mry,2023.03.15**/
enum
{
    PERSON_LABLE_STATUS_HELP_MASK = 0x8000
};

/**人员操作权限**/
enum
{
    PERSON_GRADES_0 = 0,
    PERSON_GRADES_1 = 1,
    PERSON_GRADES_2 = 2,
    PERSON_GRADES_3 = 3,
    PERSON_GRADES_15 = 15,
};

/**人员检测方式**/
enum
{
    PERSON_CAL_STRANGHT_LINE = 0,
    PERSON_CAL_HORIZONTAL_DISTANCE
};

/**人员定位发送功率**/
enum
{
    PERSON_OUTPUT_POWER_0DB = 0,
    PERSON_OUTPUT_POWER_3DB,
    PERSON_OUTPUT_POWER_6DB,
    PERSON_OUTPUT_POWER_9DB,
    PERSON_OUTPUT_POWER_12DB,
    PERSON_OUTPUT_POWER_15DB,
    PERSON_OUTPUT_POWER_18DB,
    PERSON_OUTPUT_POWER_21DB,
    PERSON_OUTPUT_POWER_24DB,
    PERSON_OUTPUT_POWER_27DB,
    PERSON_OUTPUT_POWER_30DB,
    PERSON_OUTPUT_POWER_33DB,
};

/****/
enum
{
    PERSON_DANGER_RATE_INVALID = 0,
    PERSON_DANGER_RATE_SAFE_1,//范围内无动作
    PERSON_DANGER_RATE_ALRAM_2,//范围内正在执行非危险动作
    PERSON_DANGER_RATE_DANGEROUS_3,//范围内正在执行危险动作
    PERSON_DANGER_RATE_MAX = PERSON_DANGER_RATE_DANGEROUS_3
};

/**呼救标志**/
enum
{
    PERSON_STATUS_HELP_FLAG = 0x8000
};

/**人员定位标签的具体上报数据**/
typedef struct
{
    u_int16_t PersonLableId;//ID
    u_int16_t PersonLableDistance;//距离
    u_int16_t PersonLableAuthority;//权限
    u_int16_t PersonLableStatus;//状态。低8位标签状态1进入2离开3故障，高8位bit7呼救0否1是/bit0~bit6电池电量百分比。mry,2023.03.15
}PersonLableDevType;

/***********************0级标签*************************/
typedef struct
{
    u_int16_t ZeroPersonLockState;//闭锁状态
    u_int16_t LastZeroPersonLockState;//上一次闭锁状态
    u_int16_t ZeroPersonLockNum;//闭锁架号
}stZeroPersonLock;

/******/
int DevPersonInit(void);
int DevPersonFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len);

/***参数修改时，运行参数的同步***/
int DevPersonMsgRunParamInit(void);

#endif // DEVPERSON_H
