#include "part.h"
#include "alarm.h"
#include "./HysSystem/hyssystem.h"
#include "./Sensor/sensor.h"

/*********/
#if (APPLICATION_SCENARIO_SELECTION == LIVE_CHICKEN_AND_RABBIT_WORKING_FACE)
    u16 lastMeterValue = 0;
    u16 SensorFailState = 0;
#endif

/*********/
partModuleType 	partModule[PART_NUM_MAX];
/*********/
void partModuleInit(void)
{
    memset(partModule,0,sizeof(partModule));
}

/*******************************************************************************************
*功能描述：部件模型数据加载
*入口参数：moduleID取值：
*            模型ID
*          pParam取值：
*             部件参数指针
*          pState取值：
*             部件状态指针
*返回值：  无
*******************************************************************************************/
void partModuleLoad(u8 moduleID,PartParamType *pParam,PartStateType *pState)
{
    if((pState == NULL)||(pParam == NULL)||(moduleID >=PART_NUM_MAX))
        return;

    partModule[moduleID].pParam = pParam;
    partModule[moduleID].pState = pState;
}

static u8 TiggerCondFound(PartTriggerCondType *ptrigger)
{
    SensorApiType apiFunc;
    int ret=0;

    if((ptrigger == nullptr)||(ptrigger->TriggerEnable == static_cast<u_int16_t>(EnableType_DISABLE)))
        return 0;

    if(ptrigger->sensorID == 0x0000)
        return 0;

    apiFunc = getSensorApi(ptrigger->methodID);
    if(apiFunc==NULL)
        return 0;

    ret = apiFunc(ptrigger->sensorID,0,ptrigger->sensorParam1,ptrigger->sensorParam2,nullptr);
    return ((ret<0)?0:ret);
}

/*******************************************************************************************
*功能描述：部件模型上报传感器状态
*入口参数：sensorID取值：
*            传感器模型ID
*          value取值：
*             传感器参数指针
*返回值：  无
*******************************************************************************************/
static u8 partReport_sensor(u_int16_t sensorID,HysScSensorType *value)
{
    u8 temp[20];
    u8 serverID;
    u16 length;

    u16 ServerEnabled = 0;
    u16 ServerCommErr = 0;

    HysSystem *pSystem = HysSystem::getInston();

    pSystem->GetRunScSelfServerEnabled(ServerEnabled);
    pSystem->GetRunScSelfServerCommErr(ServerCommErr);
    if((ServerEnabled == 0)||(ServerCommErr == 1))
        return 0;

    memset(temp,0,sizeof(temp));

    temp[0] = REPORTENUM_SENSOR;
    temp[1] = ENUM_SENSOR;
    temp[2] = sensorID;
    length =sizeof(HysScSensorType)-0x02;
    memcpy(&temp[3],&length,2);
    memcpy(&temp[5],&value->OriginalValue,length);
    #if (APPLICATION_SCENARIO_SELECTION == LIVE_CHICKEN_AND_RABBIT_WORKING_FACE)
    if(sensorID == 2)
    {
        if(abs(lastMeterValue-value->Value)>100)
        {
            memcpy(&temp[9],&lastMeterValue,2);
            SensorFailState = SensorFailType_MORELIMIT;
            memcpy(&temp[13],&SensorFailState,2);
        }
    }
    #endif

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    serverID = pParamMng->GetRunValueGlobalSysServerAddr();
    UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                   temp,5+length);
    #if (APPLICATION_SCENARIO_SELECTION == LIVE_CHICKEN_AND_RABBIT_WORKING_FACE)
        if(sensorID == 2)
            lastMeterValue = value->Value;
    #endif
    return 1;
}

//u16 tmpsensor[6] = {0};
/*******************************************************************************************
*功能描述：部件周期性处理函数
*入口参数：sensorID取值：
*            传感器模型ID
*          delay取值：
*             延时时间
*返回值：  无
*******************************************************************************************/
u8 partCycleProcess(u8 moduleID,u16 delay)   //1000ms
{
    PartParamType *pParam ;
    PartStateType *pState ;
    u8 i,ret;
    ParamUnit value;
    int value_abs;
    u8 alarmMode[2];
    u8 loginfo;
    HysScSensorType sensorTmp;
    static u8 overflow[SENSOR_MAX] = {0};
    if(moduleID >=PART_NUM_MAX)
        return 0;

    pParam = partModule[moduleID].pParam;
    pState = partModule[moduleID].pState;
    if((pState == NULL)||(pParam == NULL))
        return 0;

    for(i=0; i<PART_TRIGGER_NUM_MAX;i++)
    {
        if(pParam->tiggerParam[i].cond.TriggerEnable == static_cast<u_int16_t>(EnableType_DISABLE))
            continue;

        ret = TiggerCondFound(&(pParam->tiggerParam[i].cond));
        if(ret)
        {
            if(pParam->tiggerParam[i].action.alarmEnable == EnableType_ENABLE)
            {
                if(partModule[moduleID].alarm_flag[i] ==0)
                {
                    alarmMode[0] = pParam->tiggerParam[i].action.alarmMode_sound;
                    alarmMode[1] = pParam->tiggerParam[i].action.alarmMode_light;
                    Alarm *pAlarm = Alarm::getInston();
                    pAlarm->CtrlAlarm((AlarmType)(pParam->tiggerParam[i].action.alarmType),0,BaseCtrlType_STARTUP,\
                                      alarmMode,ENUM_PART,moduleID+1,ALARM_MODE_1);
                    partModule[moduleID].alarm_flag[i] =1;

                    loginfo =moduleID+1;
                    HysSystem *pSystem = HysSystem::getInston();
                    pSystem->WriteLogApi(pParam->tiggerParam[i].cond.sensorID,LOG_TYPE_ALARM,LOG_STATUS_SENSOR_ALARM,&loginfo,1);
                }
            }
            else
                partModule[moduleID].alarm_flag[i] =0;
        }
        else
        {
            if(pParam->tiggerParam[i].action.alarmEnable == static_cast<u_int16_t>(EnableType_ENABLE) )
            {
                if(partModule[moduleID].alarm_flag[i] !=0)
                {
                    Alarm *pAlarm = Alarm::getInston();                    
                    /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
                    u_int8_t alarmModetemp[2];
                    alarmModetemp[0] = 0;
                    alarmModetemp[1] = 0;
                    pAlarm->CtrlAlarm((AlarmType)(pParam->tiggerParam[i].action.alarmType),0,BaseCtrlType_STOP,\
                                      alarmModetemp,ENUM_PART,moduleID+1,ALARM_MODE_NONE);
                    partModule[moduleID].alarm_flag[i] =0;

                    loginfo =moduleID+1;
                    HysSystem *pSystem = HysSystem::getInston();
                    pSystem->WriteLogApi(pParam->tiggerParam[i].cond.sensorID,LOG_TYPE_ALARM,LOG_STATUS_SENSOR_NORMAL,&loginfo,1);
                }
            }
            else
                partModule[moduleID].alarm_flag[i] =0;
        }

        if((pParam->tiggerParam[i].cond.sensorID == 0x0000)||(pParam->tiggerParam[i].cond.sensorID > static_cast<u_int16_t>(SENSOR_MAX)))
            continue;

        HysSystem *pSystem = HysSystem::getInston();
        pSystem->GetSensorState(pParam->tiggerParam[i].cond.sensorID,sensorTmp);

        value = sensorTmp.Value;
        partModule[moduleID].ticks_delay[i] +=delay;
        if(partModule[moduleID].ticks_delay[i] >= (int)(pParam->tiggerParam[i].action.reportInterval)*100)   //1000  jhy 220625    more data test
        {
            partModule[moduleID].ticks_delay[i] =0;
            partReport_sensor(pParam->tiggerParam[i].cond.sensorID,&sensorTmp);
            partModule[moduleID].value_bak[i] =value;
            continue;
        }

        value_abs =abs(value -partModule[moduleID].value_bak[i] );
        if(value_abs > pParam->tiggerParam[i].action.reportScope)
        {
            partModule[moduleID].ticks_delay[i] =0;
            partReport_sensor(pParam->tiggerParam[i].cond.sensorID,&sensorTmp);
            partModule[moduleID].value_bak[i] =value;
            continue;
        }
        if((value > pParam->tiggerParam[i].action.reportMax)||(value < pParam->tiggerParam[i].action.reportMin))
        {
            if(overflow[moduleID] == 0)
            {
                overflow[moduleID] = 1;
                partModule[moduleID].ticks_delay[i] =0;
                partReport_sensor(pParam->tiggerParam[i].cond.sensorID,&sensorTmp);
                partModule[moduleID].value_bak[i] =value;
                continue;
            }
        }
        else
        {
            if(value_abs > pParam->tiggerParam[i].action.reportScope)
            {
                overflow[moduleID] = 0;
            }
        }
        partModule[moduleID].value_bak[i] =value;

    }

    return 1;
}

/*******************************************************************************************
*功能描述：获取部件中单动作所属的传感器ID
*入口参数：singleActionId取值：
*            单动作模型ID
*返回值：  0：无传感器     >0 :传感器id
*******************************************************************************************/
uint8_t getPartInSingleActionOfSensorId(u_int16_t singleActionId)
{
    u8 i;
    u8 sensorId = 0;
    for(i = 0 ;i<PART_NUM_MAX;i++)
    {
        if(i>=2)
            break;
        if(static_cast<u_int16_t>(partModule[i].pParam->baseParam.ActionID1) == singleActionId \
                || static_cast<u_int16_t>(partModule[i].pParam->baseParam.ActionID2) == singleActionId)
        {
            if(partModule[i].pParam->tiggerParam[0].cond.sensorID != 0)
            {
                    sensorId = partModule[i].pParam->tiggerParam[0].cond.sensorID;
                    break;
            }
        }
    }
    return sensorId;
}

