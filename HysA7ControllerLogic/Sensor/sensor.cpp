#include "unistd.h"
#include "sensor.h"
#include "./Action/alarm.h"
#include "./Action/part.h"
#include "./HysSystem/hyssystem.h"
#include "libhysa7parammng.h"
#include "sys/time.h"

/*******/
#define     POWER_ON_TIME_DELAY     2000//ms

/*******/
SensorModuleType 	sensorModule[SENSOR_MAX];
static u16 statefail[SENSOR_MAX]={0};
static u16 sensorReportCount[SENSOR_MAX] = {0};
static timeval  LastRecvTime[SENSOR_MAX] = {{0,0}};//上一次收到传感器数据的时间


/*******************************************************************************************
*功能描述：传感器判断方法（大于设定值）
*入口参数：sensorID取值：
*            传感器id号
*          sensorID2取值：
*             传感器id2号
*          param1取值：
*             设定值1
*          param2取值：
*             设定值2
*返回值：  满足：1     不满足：-1
*******************************************************************************************/
static int SensorApi_MoreThan(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata)
{
    if((sensorID > SENSOR_MAX)||(sensorID==0))
        return -1;
    if(sensorModule[sensorID-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID-1].pParam == nullptr)
        return -1;

    if((sensorModule[sensorID-1].pState->state_online == OnlineType_OFFLINE)||
      (sensorModule[sensorID-1].pState->state_fail == SensorFailType_FAIL)||
//		(sensorModule[sensorID-1].pState->state_fail == SensorFailType_MORELIMIT)||
      (sensorModule[sensorID-1].pParam->enable == (ParamUnit)EnableType_DISABLE))
    {
        if(param2 >0)
            return 1;
        else
            return -1;
    }

    if(sensorModule[sensorID-1].pState->calculateValue > param1)
    {
//       qDebug()<<"logic sensor, SensorApi_MoreThan :"<<sensorID<<sensorModule[sensorID-1].pState->calculateValue,param1;
        return 1;
    }
    else
        return 0;
}

/*******************************************************************************************
*功能描述：传感器判断方法（小于设定值）
*入口参数：sensorID取值：
*            传感器id号
*          sensorID2取值：
*             传感器id2号
*          param1取值：
*             设定值1
*          param2取值：
*             设定值2
*返回值：  满足：1     不满足：-1
*******************************************************************************************/
static int SensorApi_LessThan(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata)
{
    if((sensorID > SENSOR_MAX)||(sensorID==0))
        return -1;
    if(sensorModule[sensorID-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID-1].pParam == nullptr)
        return -1;

    if((sensorModule[sensorID-1].pState->state_online == OnlineType_OFFLINE)||
      (sensorModule[sensorID-1].pState->state_fail == SensorFailType_FAIL)||
      (sensorModule[sensorID-1].pParam->enable == (ParamUnit)EnableType_DISABLE))
    {
        if(param2 >0)
            return 1;
        else
            return -1;
    }

    if(sensorModule[sensorID-1].pState->calculateValue < param1)
    {
//        qDebug()<<"logic sensor, SensorApi_LessThan :"<<sensorID<<sensorModule[sensorID-1].pState->calculateValue<<param1;
        return 1;
    }
    else
        return 0;
}

/*******************************************************************************************
*功能描述：传感器判断方法（大于变化率值）
*入口参数：sensorID取值：
*            传感器id号
*          sensorID2取值：
*             传感器id2号
*          param1取值：
*             设定值1
*          param2取值：
*             设定值2
*返回值：  满足：1     不满足：-1
*******************************************************************************************/
static int SensorApi_MoreThan_rate(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata)
{
    if((sensorID > SENSOR_MAX)||(sensorID==0))
        return -1;
    if(sensorModule[sensorID-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID-1].pParam == nullptr)
        return -1;

    if((sensorModule[sensorID-1].pState->state_online == OnlineType_OFFLINE)||
      (sensorModule[sensorID-1].pState->state_fail == SensorFailType_FAIL)||
      (sensorModule[sensorID-1].pParam->enable == (ParamUnit)EnableType_DISABLE))
    {
        if(param2 >0)
            return 1;
        else
            return -1;
    }

    if(sensorModule[sensorID-1].pState->rate > param1)
    {
//        qDebug()<<"logic sensor ,SensorApi_MoreThan_rate:"<<sensorID<<sensorModule[sensorID-1].pState->rate<<param1;
        return 1;
    }
    else
        return 0;
}


/*******************************************************************************************
*功能描述：传感器判断方法（小于变化率值）
*入口参数：sensorID取值：
*            传感器id号
*          sensorID2取值：
*             传感器id2号
*          param1取值：
*             设定值1
*          param2取值：
*             设定值2
*返回值：  满足：1     不满足：-1
*******************************************************************************************/
static int SensorApi_LessThan_rate(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata)
{
    if((sensorID > SENSOR_MAX)||(sensorID==0))
        return -1;
    if(sensorModule[sensorID-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID-1].pParam == nullptr)
        return -1;

    if((sensorModule[sensorID-1].pState->state_online == OnlineType_OFFLINE)||
      (sensorModule[sensorID-1].pState->state_fail == SensorFailType_FAIL)||
      (sensorModule[sensorID-1].pParam->enable == (ParamUnit)EnableType_DISABLE))
    {
        if(param2 >0)
            return 1;
        else
            return -1;
    }

    if(sensorModule[sensorID-1].pState->rate < param1)
    {
//        qDebug()<<"logic sensor,SensorApi_LessThan_rate :"<<sensorID<<sensorModule[sensorID-1].pState->rate<<param1;
        return 1;
    }
    else
        return 0;
}


/*******************************************************************************************
*功能描述：传感器判断方法（大于角度值）
*入口参数：sensorID取值：
*            传感器id号
*          sensorID2取值：
*             传感器id2号
*          param1取值：
*             设定值1
*          param2取值：
*             设定值2
*返回值：  满足：1     不满足：-1
*******************************************************************************************/
static int SensorApi_MoreThan_angle(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata)
{
    if((sensorID > SENSOR_MAX)||(sensorID==0))
        return -1;
    if(sensorModule[sensorID-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID-1].pParam == nullptr)
        return -1;

    if((sensorModule[sensorID-1].pState->state_online == OnlineType_OFFLINE)||
      (sensorModule[sensorID-1].pState->state_fail == SensorFailType_FAIL)||
      (sensorModule[sensorID-1].pParam->enable == (ParamUnit)EnableType_DISABLE))
    {
        if(param2 >0)
            return 1;
        else
            return -1;
    }

    if((int16_t)(sensorModule[sensorID-1].pState->calculateValue) > (int16_t)param1)
    {
//        qDebug()<<"logic sensor, SensorApi_MoreThan_angle:"<<sensorID<<sensorModule[sensorID-1].pState->calculateValue<<param1;
        return 1;
    }
    else
        return 0;
}

/*******************************************************************************************
*功能描述：传感器判断方法（小于角度值）
*入口参数：sensorID取值：
*            传感器id号
*          sensorID2取值：
*             传感器id2号
*          param1取值：
*             设定值1
*          param2取值：
*             设定值2
*返回值：  满足：1     不满足：-1
*******************************************************************************************/
static int SensorApi_LessThan_angle(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata)
{
    if((sensorID > SENSOR_MAX)||(sensorID==0))
        return -1;
    if(sensorModule[sensorID-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID-1].pParam == nullptr)
        return -1;

    if((sensorModule[sensorID-1].pState->state_online == OnlineType_OFFLINE)||
      (sensorModule[sensorID-1].pState->state_fail == SensorFailType_FAIL)||
      (sensorModule[sensorID-1].pParam->enable == (ParamUnit)EnableType_DISABLE))
    {
        if(param2 >0)
            return 1;
        else
            return -1;
    }

    if((int16_t)(sensorModule[sensorID-1].pState->calculateValue) < (int16_t)param1)
    {
//        qDebug()<<"logic sensor, SensorApi_LessThan_angle:"<<sensorID<<sensorModule[sensorID-1].pState->calculateValue<<param1;
        return 1;
    }
    else
        return 0;
}

/*******************************************************************************************
*功能描述：传感器判断方法（传感器1大于传感器2）
*入口参数：sensorID取值：
*            传感器id号
*          sensorID2取值：
*             传感器id2号
*          param1取值：
*             设定值1
*          param2取值：
*             设定值2
*返回值：  满足：1     不满足：-1
*******************************************************************************************/
static int SensorApi_Sensor1MoreSensor2(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata)
{
    u16 presiondefault[4] = {1,10,100,1000};
    u16 presinonnow = 1;

    if((sensorID > SENSOR_MAX)||(sensorID==0))
        return -1;

    if((sensorID2 > SENSOR_MAX)||(sensorID2==0))
        return -1;

    if(sensorModule[sensorID-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID-1].pParam == nullptr)
        return -1;
    if(sensorModule[sensorID2-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID2-1].pParam == nullptr)
        return -1;

    if((sensorModule[sensorID2-1].pState->state_online == OnlineType_OFFLINE)||
      (sensorModule[sensorID2-1].pState->state_fail == SensorFailType_FAIL)||
      (sensorModule[sensorID2-1].pParam->enable == (ParamUnit)EnableType_DISABLE))
    {
        if(param2 >0)
            return 1;
        else
            return -1;
    }

    if(sensorModule[sensorID-1].pParam->precision_logic < 4)
        presinonnow = presiondefault[sensorModule[sensorID-1].pParam->precision_logic];


//	if((int16_t)(sensorModule[sensorID-1].pState->calculateValue)>(int16_t)(sensorModule[sensorID2-1].pState->calculateValue))
    {
        if((int16_t)(sensorModule[sensorID-1].pState->calculateValue)>((int16_t)sensorModule[sensorID2-1].pState->calculateValue+(int16_t)param1*presinonnow))
        {
//            qDebug()<<"logic sensor,SensorApi_Sensor1MoreSensor2 :"<<sensorID<<sensorModule[sensorID-1].pState->calculateValue<<param1;
            return 1;
        }
        else
            return 0;
    }
}

/*******************************************************************************************
*功能描述：传感器判断方法（传感器1小于传感器2）
*入口参数：sensorID取值：
*            传感器id号
*          sensorID2取值：
*             传感器id2号
*          param1取值：
*             设定值1
*          param2取值：
*             设定值2
*返回值：  满足：1     不满足：-1
*******************************************************************************************/
//sensor1<(sensor2+x)
static int SensorApi_Sensor1LessSensor2(u8 sensorID,u8 sensorID2,ParamUnit param1,ParamUnit param2,void *pdata)
{
    u16 presiondefault[4] = {1,10,100,1000};
    u16 presinonnow = 1;

    if((sensorID > SENSOR_MAX)||(sensorID==0))
        return -1;

    if((sensorID2 > SENSOR_MAX)||(sensorID2==0))
        return -1;

    if(sensorModule[sensorID-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID-1].pParam == nullptr)
        return -1;
    if(sensorModule[sensorID2-1].pState == nullptr)
        return -1;
    if(sensorModule[sensorID2-1].pParam == nullptr)
        return -1;

    if((sensorModule[sensorID2-1].pState->state_online == OnlineType_OFFLINE)||
      (sensorModule[sensorID2-1].pState->state_fail == SensorFailType_FAIL)||
      (sensorModule[sensorID2-1].pParam->enable == (ParamUnit)EnableType_DISABLE))
    {
        if(param2 >0)
            return 1;
        else
            return -1;
    }

    if(sensorModule[sensorID-1].pParam->precision_logic < 4)
        presinonnow = presiondefault[sensorModule[sensorID-1].pParam->precision_logic];

//	if((int16_t)(sensorModule[sensorID-1].pState->calculateValue)>(int16_t)(sensorModule[sensorID2-1].pState->calculateValue))
    {
        if((int16_t)(sensorModule[sensorID-1].pState->calculateValue)<((int16_t)sensorModule[sensorID2-1].pState->calculateValue+(int16_t)presinonnow))
        {
//            qDebug()<<"logic sensor , SensorApi_Sensor1LessSensor2:"<<sensorID<<sensorModule[sensorID-1].pState->calculateValue<<param1;
            return 1;
        }
        else
            return 0;
    }
}

/*******************************************************************************************
*功能描述：传感器模型初始化
*入口参数：pParam取值：
*            传感器参数指针
*          pState取值：
*             传感器状态指针
*          manualpState取值：
*             手动控制（被控架）传感器状态指针
*          sensorId取值：
*             传感器id号
*返回值：  无
*******************************************************************************************/
void SensorModuleInit(SensorParamType *pParam,SensorStateType *pState,u_int16_t sensorId)
{
    if((sensorId >= SENSOR_MAX)||(pParam == nullptr)||(pState == nullptr))
        return;

    memset(&sensorModule[sensorId],0,sizeof(SensorModuleType));
    sensorModule[sensorId].pParam = pParam;
    sensorModule[sensorId].pState = pState;

    sensorModule[sensorId].apiFunc[SENSORAPI_NONE]= NULL;
    sensorModule[sensorId].apiFunc[SENSORAPI_MoreThan]= SensorApi_MoreThan;//传感器方法判断函数
    sensorModule[sensorId].apiFunc[SENSORAPI_LessThan]= SensorApi_LessThan;
    sensorModule[sensorId].apiFunc[SENSORAPI_MoreThan_rate]= SensorApi_MoreThan_rate;
    sensorModule[sensorId].apiFunc[SENSORAPI_LessThan_rate]= SensorApi_LessThan_rate;
    sensorModule[sensorId].apiFunc[SENSORAPI_MoreThan_angle]= SensorApi_MoreThan_angle;
    sensorModule[sensorId].apiFunc[SENSORAPI_LessThan_angle]= SensorApi_LessThan_angle;
    sensorModule[sensorId].apiFunc[SENSORAPI_Sensor1MoreSensor2] = SensorApi_Sensor1MoreSensor2;
    sensorModule[sensorId].apiFunc[SENSORAPI_Sensor1LessSensor2] = SensorApi_Sensor1LessSensor2;

    /*****/
    for(int i = 0; i < SENSORAPI_MAX;i++)
    {
        gettimeofday(&LastRecvTime[i],NULL);//直接获取此时的系统时间
    }
}


//static void JudgeSensorMoreLimit(SensorModuleType *psensor,ParamUnit value)
//{
//	if(psensor->pState->state_fail == SensorFailType_FAIL)
//		return;
//	if(psensor->pParam->moreLimitTimes == 0)
//		return;
//	if(psensor->pParam->moreLimitValue == 0)
//		return;
//	if(abs(psensor->CalculateValue_last-value)> psensor->pParam->moreLimitValue)
//	{
//		psensor->pState->state_fail = SensorFailType_MORELIMIT;
//		psensor->MoreLimitTimes++;
//	}
//	else
//	{
//		psensor->MoreLimitTimes = 0;
//		psensor->CalculateValue_last = value;
//		psensor->pState->state_fail = SensorFailType_NORMAL;
//	}
//	if(psensor->MoreLimitTimes>= psensor->pParam->moreLimitTimes)
//	{
//		psensor->MoreLimitTimes = 0;
//		psensor->CalculateValue_last = value;
//		psensor->pState->state_fail = SensorFailType_NORMAL;
//	}

//}

/*******************************************************************************************
*功能描述：模拟量类型的传感器数值处理
*入口参数：pParam取值：
*            传感器参数指针
*          pState取值：
*             传感器状态指针
*          delay取值：
*             延时时间
*          sensorId取值：
*             传感器id号
*返回值：  无
*******************************************************************************************/
static uint SensorTypeFreqValueProc(SensorParamType *pParam,SensorStateType *pState,u16 delay, u_int16_t sensorId, u_int16_t value)
{
    PointIDType point;
    float freqMinTemp,freqMaxTemp;
    float caltemp,factorTemp,calvalue;
    int16_t valuetemp;
    float orignalTemp;
    u8 alarmMode[2];
    u16 presiondefault[4] = {1,10,100,1000};
    u16 presinonnow = 1;

//    qDebug()<<"**************SensorTypeFreqValueProc******************"<<sensorId<<value;
    if((pParam == nullptr)||(pState == nullptr))
        return 0;

    point = pParam->pointID;
    if(point.point == 0)
        return 0;

    if(pParam->precision < 4)
        presinonnow = presiondefault[pParam->precision];

    //ad模拟量传感器采集,或HUB123模拟量的上报(mry,23.06.09修改)
    if((point.driver == DRIVER_AD)\
        ||(((point.driver == DRIVER_CAN1)||(point.driver == DRIVER_CAN2))\
            &&(point.device >= EXTDEV_TYPE_HUB_1)&&((point.device <= EXTDEV_TYPE_HUB_3)\
            &&(point.subDevice == DEV_CHILDDEV_IGNOED)\
            &&(point.point >= DEV_POINT_1)&&(point.point <= DEV_POINT_4))))
    {
        orignalTemp = ((float)value/1000)*((float)presinonnow);  //AD??
        pState->originalValue = orignalTemp;

        freqMinTemp = (float)pParam->freqMin*(100 - FREQ_DEAL_OFFSET)/100;    //????????95%
        freqMaxTemp = (float)pParam->freqMax*(100 + FREQ_DEAL_OFFSET)/100;    //????????105%
        if((orignalTemp > freqMaxTemp)||(orignalTemp < freqMinTemp))	//??
        {
            if(pState->state_fail != SensorFailType_FAIL)
            {
                /**传感器故障，无需写入***/
                //WriteFault_API(FAULT_HOST_SENSOR,sensorId+1,FAULT_EVENT_FAULT,SENSOR_FAULT_STATUS_FAULT);
                if(pParam->alarmEnable == (ParamUnit)EnableType_ENABLE)
                {
                    if(pParam->alarmType < (ParamUnit)AlarmType_Phone)
                    {
                        alarmMode[0] = pParam->alarmMode_sound;
                        alarmMode[1] = pParam->alarmMode_light;
                        Alarm *pAlaram = Alarm::getInston();
                        pAlaram->CtrlAlarm((AlarmType)(pParam->alarmType),pParam->alarmTime,BaseCtrlType_STARTUP,\
                                           alarmMode,ENUM_SENSOR,sensorId+1,ALARM_MODE_1);
                    }
                }
            }

            pState->state_fail = SensorFailType_FAIL;
//			pState->calculateValue=SENSOR_FAIL_VALUE;
            pState->rate=0;

            return 0;
        }

        if((pState->originalValue>= freqMinTemp)&&(pState->originalValue< (float)pParam->freqMin))//???????95%~100% ??????
            pState->originalValue = pParam->freqMin;
        else if((pState->originalValue<= freqMaxTemp)&&(pState->originalValue> (float)pParam->freqMax))//???????100%~105% ??????
            pState->originalValue = pParam->freqMax;

        caltemp = orignalTemp;//*(float)pParam->factor/100+pParam->offset;   //y = kx+b     jhy  220708
        factorTemp = (float)(pParam->freqMax-pParam->freqMin)/(float)(pParam->scopeMax - pParam->scopeMin);

        if(pParam->extParam ==SensorExtParamType_Reverse)//????---????
        {
            calvalue = pParam->scopeMax -(float)(caltemp-pParam->freqMin)/factorTemp;
            if(calvalue<0)
                calvalue =0;
        }
        else
            calvalue = (float)(caltemp-pParam->freqMin)/factorTemp + pParam->scopeMin;//???

        pState->calculateValue = (ParamUnit)(calvalue);   // jhy  220708
//		JudgeSensorMoreLimit(&sensorModule[sensorId],pState->calculateValue*(float)pParam->factor/100+pParam->offset);
        pState->calculateValue = pState->calculateValue*(float)pParam->factor/100+pParam->offset;  // jhy  220708

//        qDebug()<<"**************SensorTypeFreqValueProc***ad********result*******"<<sensorId<<pState->calculateValue<<pState->state_online<<pState->state_fail;

//		if(pState->state_fail != SensorFailType_MORELIMIT)
        {
            if(pState->state_fail != SensorFailType_NORMAL)
            {
                /**传感器故障，无需写入***/
                //WriteFault_API(FAULT_HOST_SENSOR,sensorId+1,FAULT_EVENT_FAULT,FAULT_STATUS_NORMAL);
            }
            pState->state_fail = SensorFailType_NORMAL;
        }
    }
    //can传感器采集
    else  if((point.driver == DRIVER_CAN1)||(point.driver == DRIVER_CAN2))
    {
        //高度
        if(point.device == EXTDEV_TYPE_HIGH)//(mry,23.06.09修改)
        {
            orignalTemp = ((float)value/10)*((float)presinonnow);   //CAN??
            valuetemp = pState->originalValue =orignalTemp;

            freqMinTemp = (float)pParam->freqMin*(100 - FREQ_DEAL_OFFSET)/100;
            freqMaxTemp = (float)pParam->freqMax*(100 + FREQ_DEAL_OFFSET)/100;
            if((orignalTemp > freqMaxTemp)||(orignalTemp < freqMinTemp))
            {
                if(pState->state_fail != SensorFailType_FAIL)
                {
                    /**传感器故障，无需写入***/
                    //WriteFault_API(FAULT_HOST_SENSOR,sensorId+1,FAULT_EVENT_FAULT,SENSOR_FAULT_STATUS_FAULT);
                    if(pParam->alarmEnable == (ParamUnit)EnableType_ENABLE)
                    {
                        if(pParam->alarmType < (ParamUnit)AlarmType_Phone)
                        {
                            alarmMode[0] = pParam->alarmMode_sound;
                            alarmMode[1] = pParam->alarmMode_light;
                            Alarm *pAlaram = Alarm::getInston();
                            pAlaram->CtrlAlarm((AlarmType)(pParam->alarmType),pParam->alarmTime,BaseCtrlType_STARTUP,\
                                               alarmMode,ENUM_SENSOR,sensorId+1,ALARM_MODE_1);
                        }
                    }
                }

                pState->state_fail = SensorFailType_FAIL;
//				pState->calculateValue=SENSOR_FAIL_VALUE;
                pState->rate=0;
                return 0;
            }
            else
            {
                if(pState->state_fail != SensorFailType_NORMAL)
                {
                    /**传感器故障，无需写入***/
                    //WriteFault_API(FAULT_HOST_SENSOR,sensorId+1,FAULT_EVENT_FAULT,FAULT_STATUS_NORMAL);
                }
                pState->state_fail = SensorFailType_NORMAL;
            }

            if((pState->originalValue>= freqMinTemp)&&(pState->originalValue< (float)pParam->freqMin))
                pState->originalValue = pParam->freqMin;
            else if((pState->originalValue<= freqMaxTemp)&&(pState->originalValue> (float)pParam->freqMax))
                pState->originalValue = pParam->freqMax;

            caltemp = orignalTemp;//*(float)pParam->factor/100+pParam->offset;    // jhy  220708
            factorTemp = (float)(pParam->freqMax-pParam->freqMin)/(float)(pParam->scopeMax - pParam->scopeMin);
            if(pParam->extParam ==SensorExtParamType_Reverse)
            {
                calvalue = pParam->scopeMax -(float)(caltemp-pParam->freqMin)/factorTemp;
                if(calvalue<0)
                    calvalue =0;
            }
            else
                calvalue = (float)(caltemp-pParam->freqMin)/factorTemp + pParam->scopeMin;

            pState->calculateValue = (ParamUnit)(calvalue);			// jhy  220708
            pState->calculateValue = pState->calculateValue*(float)pParam->factor/100+pParam->offset;   // jhy  220708
        }
        //角度(mry,23.06.09修改)
        else if((point.device == EXTDEV_TYPE_ANGLE)\
                ||(point.device == EXTDEV_TYPE_ANGLE_HEIGHT)\
                ||(point.device == EXTDEV_TYPE_MULTI_SOUND)\
                ||((point.device >= EXTDEV_TYPE_HUB_1)&&((point.device <= EXTDEV_TYPE_HUB_3)\
                   &&(point.subDevice != DEV_CHILDDEV_IGNOED)&&(point.subDevice <= DEV_CHILDDEV_4)\
                   &&(point.point >= DEV_POINT_1)&&(point.point <= DEV_POINT_3))))
        {
            valuetemp = (int16_t)value;
            if((valuetemp> ((int16_t)pParam->freqMax*100))||(valuetemp< (int16_t)pParam->freqMin*100))
            {
                if(pState->state_fail != SensorFailType_FAIL)
                {
                    /**传感器故障，无需写入***/
                    //WriteFault_API(FAULT_HOST_SENSOR,sensorId+1,FAULT_EVENT_FAULT,SENSOR_FAULT_STATUS_FAULT);
                    if(pParam->alarmEnable == (ParamUnit)EnableType_ENABLE)
                    {
                        if(pParam->alarmType < (ParamUnit)AlarmType_Phone)
                        {
                            alarmMode[0] = pParam->alarmMode_sound;
                            alarmMode[1] = pParam->alarmMode_light;
                            Alarm *pAlaram = Alarm::getInston();
                            pAlaram->CtrlAlarm((AlarmType)(pParam->alarmType),pParam->alarmTime,BaseCtrlType_STARTUP,\
                                               alarmMode,ENUM_SENSOR,sensorId+1,ALARM_MODE_1);
                        }
                    }
                }

                pState->state_fail = SensorFailType_FAIL;
                pState->calculateValue=0;
                pState->rate=0;
                return 0;
            }
            else
            {
                if(pState->state_fail != SensorFailType_NORMAL)
                {
                    /**传感器故障，无需写入***/
                    //WriteFault_API(FAULT_HOST_SENSOR,sensorId+1,FAULT_EVENT_FAULT,FAULT_STATUS_NORMAL);
                }
                pState->state_fail = SensorFailType_NORMAL;
            }

            pState->originalValue =value;
            pState->calculateValue =(u16)(valuetemp/10);
        }
//        qDebug()<<"**************SensorTypeFreqValueProc***********result*******"<<sensorId<<pState->calculateValue<<pState->state_online<<pState->state_fail;;

    }
    else
    {
        pState->originalValue =0;
        pState->calculateValue=value;
        pState->state_fail = SensorFailType_NORMAL;
    }

    return 1;
}

/*******************************************************************************************
*功能描述：传感器采集变化率函数（例如：移架速度）
*入口参数：pParam取值：
*            传感器参数指针
*          pState取值：
*             传感器状态指针
*          delay取值：
*             延时时间
*          sensorId取值：
*             传感器id号
*返回值：  无
*******************************************************************************************/
static u8 SensorTypeSwitchValueProc(SensorParamType *pParam,SensorStateType *pState,u_int16_t delay,u_int16_t sensorId, u_int16_t value)
{
    return 1;
}

static u8 SensorRateProc(SensorModuleType *psensor,u16 delay)
{
    ParamUnit value = 0;
    ParamUnit value_old = 0;
    ParamUnit temp = 0;

    psensor->tick += delay;
    if(psensor->tick >= 1000)
    {
        psensor->tick =0;
        value = psensor->pState->calculateValue;
        value_old = psensor->CalculateValue_old;

        if((value==0)&&(value_old==0))
            psensor->pState->rate =0;
        else if(value_old==0)
        {
            psensor->CalculateValue_old = psensor->pState->calculateValue;
        }
        else if(value >= value_old)
        {
            psensor->pState->rate = (ParamUnit)((float)(value-value_old)/(float)value_old*100);
            psensor->CalculateValue_old = psensor->pState->calculateValue;
        }
        else if(value < value_old)
        {
            temp = (ParamUnit)((float)(value_old-value)/(float)value_old*100);
            psensor->pState->rate = static_cast<ParamUnit>((temp&0x7FFF)|(1<<16));
            psensor->CalculateValue_old = psensor->pState->calculateValue;
        }
    }
    return 1;
}


/*******************************************************************************************
*功能描述：传感器周期上报（服务器）函数
*入口参数：sensorID取值：
*            传感器id号
*返回值：  无
*******************************************************************************************/
static u8 sensorReport_sensor(u_int16_t sensorID)
{
    u8 temp[40];
    u_int16_t serverID;
    u16 length;

    u16 ServerEnabled = 0;
    u16 ServerCommErr = 0;

    HysSystem *pSystem = HysSystem::getInston();
    pSystem->GetRunScSelfServerEnabled(ServerEnabled);
    pSystem->GetRunScSelfServerCommErr(ServerCommErr);

    if((ServerEnabled == EnableType_DISABLE)\
        ||(ServerCommErr == 1))
        return 1;

    if(sensorModule[sensorID].pParam == nullptr)
        return 1;

    memset(temp,0,sizeof(temp));

    temp[0] = REPORTENUM_SENSOR;
    temp[1] = ENUM_SENSOR;
    temp[2] = sensorModule[sensorID].pParam->sensorID;
    length =sizeof(SensorStateType);
    memcpy(&temp[3],&length,2);
    memcpy(&temp[5],sensorModule[sensorID].pState,length);

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    serverID = pParamMng->GetRunValueGlobalSysServerAddr();
    UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                   temp,5+length);
    return 1;
}


SensorApiType getSensorApi(u8 apiID)
{
    if(apiID >=SENSORAPI_MAX)
        return nullptr;
    else
        return sensorModule[0].apiFunc[apiID];
}

/***设置收到的外围设备的离线状态***/
int SensorSetExtDevOffLineApi(u_int16_t driverid, u_int16_t devid, u_int16_t childdevid, u_int16_t onlinestate)
{
    u_int16_t SensorId = 0;
    u_int16_t bufId = 0;

    for(u_int16_t i = 0;i < SENSOR_MAX;i++)
    {
        if(sensorModule[i].pParam == nullptr)
            break;
        if(sensorModule[i].pState == nullptr)
            break;

        if(sensorModule[i].pParam->sensorID == 0)
            break;

        if((driverid == sensorModule[i].pParam->pointID.driver)\
            &&(devid == sensorModule[i].pParam->pointID.device)\
            &&(childdevid = sensorModule[i].pParam->pointID.subDevice))
        {
            SensorId = sensorModule[i].pParam->sensorID;
            bufId = i;
            if(SensorId != 0)//if((SensorId != 0)&&(bufId != 0))
            {
                if((onlinestate !=sensorModule[bufId].pState->state_online)\
                   &&(sensorModule[bufId].pParam->enable == (ParamUnit)EnableType_ENABLE))
                    sensorReport_sensor(SensorId);

                sensorModule[bufId].pState->state_online = onlinestate;
                if(onlinestate == OnlineType_OFFLINE)
                {
                    memset(sensorModule[bufId].pState,0,sizeof(SensorStateType));
                }
            }
        }
    }

    return SensorId;
}

/*******/
int SensorInsertStateDetailProc(u_int16_t SensorId, u_int16_t bufId, u_int16_t value)
{
    if(SensorId == 0)
        return 0;

    if(bufId == 0xFFFF)//未找到
        return 0;

    /*********/
    if(sensorModule[bufId].pParam->enable == (ParamUnit)EnableType_DISABLE)
    {
        sensorModule[bufId].pState->Id = 0;
        sensorModule[bufId].pState->calculateValue = 0;//不能改变设备在线或故障的状态//memset(sensorModule[bufId].pState,0,sizeof(SensorStateType));
        return 0;
    }
    else
    {
        sensorModule[bufId].pState->Id = SensorId;
    }

    if((SensorId != 0)&&(bufId != 0xFFFF))
    {
        timeval nowtime;
        gettimeofday(&nowtime,NULL);//直接获取此时的系统时间
        /***计算此时的延时时间**/
        u_int16_t delay = 1000*(nowtime.tv_sec-LastRecvTime[bufId].tv_sec)+(nowtime.tv_usec-LastRecvTime[bufId].tv_usec)/1000;

        if(sensorModule->pParam->type == (ParamUnit)PointType_FREQ)
        {
            uint ret = SensorTypeFreqValueProc(sensorModule[bufId].pParam,sensorModule[bufId].pState,delay,SensorId,value);
            if(ret)
                SensorRateProc(&sensorModule[bufId],delay);
        }
        else
        {
            SensorTypeSwitchValueProc(sensorModule[bufId].pParam,sensorModule[bufId].pState,delay,SensorId,value);
        }

        if(statefail[bufId] !=sensorModule[bufId].pState->state_fail)
        {
            sensorReportCount[bufId]++;
            if(sensorReportCount[bufId]>2)
            {
                sensorReportCount[bufId] = 0;
                sensorReport_sensor(SensorId);
                statefail[bufId] =sensorModule[bufId].pState->state_fail;
            }
        }

        /***写入到System中**/
        HysScSensorType syssensorstate;
        syssensorstate.Id = SensorId;
        memmove(&syssensorstate.OriginalValue,&sensorModule[bufId].pState->originalValue,sizeof(SensorStateType)-0x02);
        HysSystem *pSystem = HysSystem::getInston();
        pSystem->SetSensorState(SensorId,syssensorstate);
    }

    return 1;
}
/*!
 * \brief InsertSensorStateApi,插入收到的点数据
 * \param driverid,驱动号
 * \param devid，设备号
 * \param childdevid，点号
 * \param value，数值
 * \return 0异常，1正常
 */
int SensorInsertStateApi(u_int16_t driverid, u_int16_t devid,u_int16_t childdevid, u_int16_t pointid,u_int16_t value)
{
    u_int16_t SensorId = 0;
    u_int16_t bufId = 0xFFFF;

    for(int i = 0;i < SENSOR_MAX;i++)
    {
        if(sensorModule[i].pParam == nullptr)//未赋值
            break;
        if(sensorModule[i].pState == nullptr)//未赋值
            break;
        if(sensorModule[i].pParam->sensorID == 0)
            break;

        if((driverid == sensorModule[i].pParam->pointID.driver)\
            &&(devid == sensorModule[i].pParam->pointID.device)\
            &&(childdevid == sensorModule[i].pParam->pointID.subDevice)\
            &&(pointid == sensorModule[i].pParam->pointID.point))
        {
            SensorId = sensorModule[i].pParam->sensorID;
            bufId = i;
            SensorInsertStateDetailProc(SensorId,bufId,value);
        }
    }

    return SensorId;
}

