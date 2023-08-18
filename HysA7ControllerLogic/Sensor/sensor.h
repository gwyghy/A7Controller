#ifndef SENSOR_H
#define SENSOR_H

#include "libhysa7parammng.h"

/*************/
#define FREQ_DEAL_OFFSET    10 //10%

/*------------------------state define----------------------*/

typedef enum
{
    SensorFailType_NORMAL=0,
    SensorFailType_FAIL ,
    SensorFailType_MORELIMIT
}FailType;

typedef struct
{
    ParamUnit	Id;             //编号。mry新增加
    ParamUnit	originalValue;	//原始值
    ParamUnit   calculateValue;	//计算值
    ParamUnit   rate;			//变化率
    ParamUnit	state_online;	//在线状态
    ParamUnit	state_fail;		//故障状态
}SensorStateType;


/*------------------------sensor module define----------------------*/
typedef int (*SensorApiType )(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata);

enum
{
    SENSORAPI_NONE =0,
    SENSORAPI_MoreThan,    //x>y
    SENSORAPI_LessThan,    //x<y
    SENSORAPI_MoreThan_rate,    //rate x>y
    SENSORAPI_LessThan_rate,    //rate x<y
    SENSORAPI_MoreThan_angle,   //angle x>y
    SENSORAPI_LessThan_angle,   //angle x<y
    SENSORAPI_Sensor1MoreSensor2,
    SENSORAPI_Sensor1LessSensor2,
    SENSORAPI_MAX
};

typedef struct
{
    SensorParamType		*pParam;
    SensorStateType		*pState;
    SensorApiType		apiFunc[SENSORAPI_MAX];

    ParamUnit			CalculateValue_old;
    ParamUnit     CalculateValue_last;
    ParamUnit     MoreLimitTimes;
    u16                 tick;
}SensorModuleType;

/*------------------------other define----------------------*/
void SensorModuleInit(SensorParamType *pParam,SensorStateType *pState,u_int16_t sensorId);
SensorApiType getSensorApi(u8 apiID);

/***设置收到的外围设备的离线状态***/
int SensorSetExtDevOffLineApi(u_int16_t driverid, u_int16_t devid, u_int16_t childdevid, u_int16_t onlinestate);

/***插入收到的点数据***/
int SensorInsertStateApi(u_int16_t driverid, u_int16_t devid, u_int16_t childdevid, u_int16_t pointid, u_int16_t value);

#endif // SENSOR_H
