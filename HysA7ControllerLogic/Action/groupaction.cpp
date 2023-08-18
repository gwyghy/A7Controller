#include "groupaction.h"
#include "combaction.h"
#include "alarm.h"
#include "string.h"
#include "./HysSystem/hyssystem.h"
#include "./Sensor/sensor.h"

/********/
groupActionModuleType 	groupActionModule[GROUPACTION_NUM_MAX];
static int	TiggerTicks[GROUPACTION_NUM_MAX][GROUP_ACTION_TRIGGER_MAX];
static u8	TiggerTicksFlag[GROUPACTION_NUM_MAX][GROUP_ACTION_TRIGGER_MAX];

/********/
void groupActionModuleInit(void)
{
    memset(groupActionModule,0,sizeof(groupActionModule));
    memset(TiggerTicks,0,sizeof(TiggerTicks));
    memset(TiggerTicksFlag,0,sizeof(TiggerTicksFlag));
}

void groupActionModuleLoad(u8 moduleID,groupActionParamType *pParam,groupActionStateType *pState)
{
    if((pState == NULL)||(pParam == NULL)||(moduleID >=GROUPACTION_NUM_MAX))
        return;

    groupActionModule[moduleID].pParam = pParam;
    groupActionModule[moduleID].pState = pState;
}

static u8 groupActionAutoCtrlFunc(ActionCtrlType ctrl,groupActionParamType *pParam,groupActionStateType *pState)
{
    memcpy((u_int8_t *)&pState->ActionCtrl.CtrlCmd,\
           (u_int8_t *)&ctrl.CtrlCmd,sizeof(ActionCtrlType));
    switch(ctrl.CtrlCmd)
    {
        case ActionCtrl_CmdType_STARTUP:
        {
//			if(pState->RunState != RunStateType_IDLE)
//				return 0;

            pState->RunState = RunStateType_STARTUP;
            //add log
        }
        break;

        case ActionCtrl_CmdType_WAITSTARTUP:
        {
            if(pState->RunState != RunStateType_IDLE)
                return 0;

            pState->RunState = RunStateType_ALARM;
            //add log
        }
        break;

        case ActionCtrl_CmdType_STOP:
        {
            if(pState->RunState == RunStateType_IDLE)
                return 0;

            pState->RunState = RunStateType_STOP;
            //add log
        }
        break;

        case ActionCtrl_CmdType_SUSPEND:
        {
            if(pState->RunState != RunStateType_IDLE)
                return 0;

            pState->RunStateBak = pState->RunState;
            pState->RunState = RunStateType_SUSPEND;
            //add log
        }
        break;

        case ActionCtrl_CmdType_RESUME:
        {
            if(pState->RunState != RunStateType_SUSPEND)
                return 0;

            pState->RunState = pState->RunStateBak;
            //add log
        }
        break;

        default:
            return 0;
    }

    return 1;
}

static u8 getGroupActionScope(u8 *startid,u8 *endid,u8 *groupendid,groupActionParamType *pParam,u8 moduleID)
{
    u8 startaddr,endaddr,groupEndaddr,i,scope;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t currentid = pParamMng->GetRunValueGlobalCurrentSupportID();
    u_int16_t SupportDirect = pParamMng->GetRunValueGlobalSupportDirect();

    if((groupActionModule[moduleID].startID!=0)&&(groupActionModule[moduleID].endID!=0)
        &&(groupActionModule[moduleID].groupEndID!=0))
    {
        *startid = groupActionModule[moduleID].startID;
        *endid = groupActionModule[moduleID].endID;
        *groupendid = groupActionModule[moduleID].groupEndID;
        return 1;
    }

    startaddr = groupActionModule[moduleID].startID;
    endaddr = groupActionModule[moduleID].endID;

    if(((groupActionModule[moduleID].direct == TriggerDirectType_LEFT)&&(SupportDirect>0))
      ||((groupActionModule[moduleID].direct == TriggerDirectType_RIGHT)&&(SupportDirect==0)))
    {
        if((startaddr -endaddr+1)>=pParam->actionParam.groupCnt)
            groupEndaddr = startaddr -pParam->actionParam.groupCnt+1;
        else
            groupEndaddr = endaddr;
    }
    else
    {
        if((startaddr +pParam->actionParam.groupCnt-1)<=endaddr)
            groupEndaddr = startaddr +pParam->actionParam.groupCnt-1;
        else
            groupEndaddr = endaddr;
    }

    *startid = startaddr;
    *endid = endaddr;
    *groupendid = groupEndaddr;
    groupActionModule[moduleID].groupEndID =groupEndaddr;
    //groupActionModule[moduleID].direct = pParam->actionParam.groupDirct;
    return 1;
}

static u8 groupAction_CtrlAlarm(u8 alarmType ,u16 alarmTime, u8 addr,BaseCtrlType cmd)
{
    u8 temp[12];

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();

    memset(temp,0,sizeof(temp));
    temp[0] = ENUM_ALARM;
    temp[1] = 1;
    temp[2] = (cmd==BaseCtrlType_STARTUP)? ActionCtrl_CmdType_STARTUP:ActionCtrl_CmdType_STOP;
    temp[3] = ActionCtrl_ModeType_AUTO;
    temp[4] = 3;
    temp[5] = (u8)(ScSelfAddr&0xFF);
    temp[6] = ActionCtrl_SourceType_CONTROLLER;
    temp[7] = 0;//用户ID
    temp[9] = alarmType;                      //模型参数：报警类型
    memcpy(&temp[10],&alarmTime,2);           //模型参数：报警时间（2bytes）

    UdpAppProcCtrl(V4APP_DEV_TYPE_SC,addr,V4APP_STATUS_CTRL_DEV,temp,12);
    qDebug("group ctrl %d alarm \r\n", addr);
    return 1;
}

static u8 TiggerCondFound(groupAction_TriggerCondType *ptrigger,groupActionParamType *pParam)
{
    SensorApiType apiFunc;
    int ret=0;
    u8 runFin=0;

    if((ptrigger == NULL)||(ptrigger->TriggerEnable == EnableType_DISABLE))
        return 0;

    if((ptrigger->sensorID ==0)&&(ptrigger->TriggerType == groupActionEnum_triggerType_RUN))
        return 1;
//	else if((ptrigger->sensorID ==0)&&(ptrigger->TriggerType == groupActionEnum_triggerType_NEXTRUN))
//		return 0;

    if(ptrigger->TriggerActionID !=0)
    {
        if(pParam->actionParam.actionType == groupActionEnum_ActionType_SINGLE)
            return 0;
        else
        {
            getCombActionFin(pParam->actionParam.actionID,ptrigger->TriggerActionID,&runFin);
            if(runFin!=1)
                return 0;
            else if((ptrigger->sensorID ==0)&&(ptrigger->TriggerType == groupActionEnum_triggerType_NEXTRUN))
                return 1;
        }
    }

    apiFunc = getSensorApi(ptrigger->methodID);
    if(apiFunc==NULL)
        return 0;

    ret = apiFunc(ptrigger->sensorID,0,ptrigger->sensorParam1,ptrigger->sensorParam2,0);
    return ((ret<0)?0:ret);
}

static u8 groupActionTiggerCond(TriggerActionPointType type,groupActionEnum_triggerType triggerType,groupActionParamType *pParam,u8 moduleID,u16 delay)
{
    u8 i;
    u8 ret=0;
    u8 condret=0;

    if(triggerType == groupActionEnum_triggerType_RUN)
    {
        for(i=0; i<GROUP_ACTION_TRIGGER_MAX;i++)
        {
            if(pParam->tiggerParam[i].TriggerEnable == EnableType_DISABLE)
                continue;
            ret=1;
        }
        if(ret==0)
            return  1;
    }

    for(i=0; i<GROUP_ACTION_TRIGGER_MAX;i++)
    {
        if(pParam->tiggerParam[i].TriggerEnable == EnableType_DISABLE)
            continue;

        if(pParam->tiggerParam[i].TriggerActionPoint != type)
            continue;

        condret = TiggerCondFound(&(pParam->tiggerParam[i]),pParam);
        if(condret)
        {
            if(TiggerTicksFlag[moduleID][i]==0)
            {
                TiggerTicksFlag[moduleID][i] =1;
                TiggerTicks[moduleID][i]=0;
            }
        }

        if(TiggerTicksFlag[moduleID][i])
        {
            TiggerTicks[moduleID][i]+=delay;
        }
        else
            TiggerTicks[moduleID][i]=0;

        if(((condret)&&(pParam->tiggerParam[i].TriggerDelayTime==0))
           ||((TiggerTicksFlag[moduleID][i])&&(TiggerTicks[moduleID][i] >= pParam->tiggerParam[i].TriggerDelayTime*LOGIC_TIME_MS)))
        {
            ret=1;
        }
    }

    return ret;
}

static u8 groupAction_CtrlAction(u8 addr,groupActionParamType *pParam,ActionCtrl_CmdType cmd)
{
    u8 temp[9];

    memset(temp,0,sizeof(temp));

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();

    temp[0] = ENUM_GROUP_ACTION_KZQ;
//	temp[1] = pParam->actionParam.actionID;
    temp[1] = pParam->baseParam.ID;
    temp[2] = cmd;
    temp[3] = ActionCtrl_ModeType_AUTO;
    temp[4] = 0;
    temp[5] = (u8)(ScSelfAddr&0xFF);
    temp[6] = ActionCtrl_SourceType_CONTROLLER;
    temp[7] = 0;//用户ID
    temp[8] = 0;//用户ID

    UdpAppProcCtrl(V4APP_DEV_TYPE_SC,addr,V4APP_STATUS_CTRL_DEV,temp,9);
    qDebug("group ctrl addr: %d ,actiontype: %d ,actionid: %d", addr,temp[0],temp[1]);
    return 1;
}

static u8 groupAction_CtrlNextGroupAction(u8 addr,u8 actionID)
{
    u8 temp[12];

    memset(temp,0,sizeof(temp));

    temp[0] = ENUM_GROUP_ACTION;
    temp[1] = actionID+1;
    temp[2] = groupActionModule[actionID].pState->ActionCtrl.CtrlCmd;
    temp[3] = groupActionModule[actionID].pState->ActionCtrl.CtrlMode;
    temp[4] = 3;
    temp[5] = groupActionModule[actionID].pState->ActionCtrl.CtrlSourceAddr;
    temp[6] = groupActionModule[actionID].pState->ActionCtrl.CtrlSourceType;
    temp[7] = 0;//用户ID
    temp[9] = addr ;
    temp[10] = groupActionModule[actionID].endID;
    temp[11] = groupActionModule[actionID].direct;

    UdpAppProcCtrl(V4APP_DEV_TYPE_SC,addr,V4APP_STATUS_CTRL_DEV,temp,12);
    return 1;
}

static u8 groupActionOperate(u8 moduleID,ActionCtrl_CmdType cmd)
{
    u8 startid,endid,groupEndid,tempid;
    int i;
    groupActionParamType *pParam = groupActionModule[moduleID].pParam;
    groupActionStateType *pState = groupActionModule[moduleID].pState;
    u8 ret;
    ActionCtrlType ctrl;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();
    u_int16_t SupportDirect = pParamMng->GetRunValueGlobalSupportDirect();

    ret = getGroupActionScope(&startid,&endid,&groupEndid,pParam,moduleID);
    if(ret==0)
        return 0;

    if(groupActionModule[moduleID].groupFinish)
        return 0;

    if(groupActionModule[moduleID].currentOperateId ==0)
        groupActionModule[moduleID].currentOperateId =startid;

    if(groupActionModule[moduleID].currentOperateId == 0)//对异常进行处理。23.07.08
        return 0;

    if(((groupActionModule[moduleID].direct == TriggerDirectType_LEFT)&&(SupportDirect>0))
      ||((groupActionModule[moduleID].direct == TriggerDirectType_RIGHT)&&(SupportDirect==0)))
    {
        if((groupActionModule[moduleID].currentOperateId -endid+1)>=pParam->actionParam.groupCnt)
            tempid = groupActionModule[moduleID].currentOperateId -pParam->actionParam.groupCnt+1;
        else
            tempid = endid;

        if(tempid == 0)//对异常进行处理。23.07.08
            return 0;

        //groupActionModule[moduleID].groupEndID = tempid;
        for(i=groupActionModule[moduleID].currentOperateId; i>=tempid;i=i-pParam->actionParam.actionGap-1)
        {
            if((i<groupActionModule[moduleID].groupEndID)&&(i>=endid))
            {
                groupAction_CtrlNextGroupAction(i,moduleID);
                groupActionModule[moduleID].groupFinish =1;
                break;
            }
            else
            {
                if(i == ScSelfAddr)
                {
                    ctrl.CtrlCmd = cmd;
                    ctrl.CtrlMode = pState->ActionCtrl.CtrlMode;
                    ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType;
                    ctrl.CtrlSourceAddr = (u8)(ScSelfAddr&0xFF);
                    ctrl.UserID =0;

                    if(pParam->baseParam.enable == EnableType_DISABLE)
                        continue;

                    if(pParam->actionParam.actionID ==0)
                    {
                        return 0;
                    }
                    if(pParam->actionParam.actionType == groupActionEnum_ActionType_SINGLE)
                    {
                        SigActionCtrlFunc(ctrl,pParam->actionParam.actionID-1,0);//,pParam->baseParam.ID);
                    }
                    else
                        CombActionCtrlFunc(ctrl,pParam->actionParam.actionID-1);
                }
                else
                {
                    groupAction_CtrlAction(i,pParam, cmd);
                }
            }
        }

        groupActionModule[moduleID].currentOperateId = tempid -1;
        if(groupActionModule[moduleID].currentOperateId>=pParam->actionParam.actionGap)
            groupActionModule[moduleID].currentOperateId =groupActionModule[moduleID].currentOperateId -pParam->actionParam.actionGap;
        else
            groupActionModule[moduleID].currentOperateId =0;

        if(groupActionModule[moduleID].currentOperateId ==0)
        {
            groupActionModule[moduleID].groupFinish =1;
        }
    }
    else
    {
        if((groupActionModule[moduleID].currentOperateId +pParam->actionParam.groupCnt-1)<=endid)
            tempid = groupActionModule[moduleID].currentOperateId +pParam->actionParam.groupCnt-1;
        else
            tempid = endid;

        //groupActionModule[moduleID].groupEndID = tempid;
        for(i=groupActionModule[moduleID].currentOperateId; i<=tempid;i=i+pParam->actionParam.actionGap+1)
        {
            if((i>groupActionModule[moduleID].groupEndID)&&(i<=endid))
            {
                groupAction_CtrlNextGroupAction(i,moduleID);
                groupActionModule[moduleID].groupFinish =1;
                break;
            }
            else
            {
                if(i==ScSelfAddr)
                {
                    ctrl.CtrlCmd = cmd;
                    ctrl.CtrlMode = pState->ActionCtrl.CtrlMode;
                    ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType;
                    ctrl.CtrlSourceAddr = (u8)(ScSelfAddr&0xFF);
                    ctrl.UserID =0;

                    if(pParam->baseParam.enable == EnableType_DISABLE)
                        continue;

                    if(pParam->actionParam.actionID ==0)
                    {
                        return 0;
                    }
                    if(pParam->actionParam.actionType == groupActionEnum_ActionType_SINGLE)
                        SigActionCtrlFunc(ctrl,pParam->actionParam.actionID-1,0);//pParam->baseParam.ID);
                    else
                        CombActionCtrlFunc(ctrl,pParam->actionParam.actionID-1);
                }
                else
                {
                    groupAction_CtrlAction(i,pParam, cmd);
                }
            }
        }

        groupActionModule[moduleID].currentOperateId = tempid+1;
        if(((u16)groupActionModule[moduleID].currentOperateId +pParam->actionParam.actionGap)<=255)
            groupActionModule[moduleID].currentOperateId =groupActionModule[moduleID].currentOperateId +pParam->actionParam.actionGap;
        else
            groupActionModule[moduleID].currentOperateId =255;

        if(groupActionModule[moduleID].currentOperateId >endid)
        {
            groupActionModule[moduleID].groupFinish =1;
        }
    }
    return 1;
}


u8 stopGroupAction(u8 startId,u8 endId,u8 moduleID,ActionCtrl_CmdType cmd)
{;
    int i;
    u8 start;
    groupActionParamType *pParam = groupActionModule[moduleID].pParam;
    groupActionStateType *pState = groupActionModule[moduleID].pState;
    u8 ret;
    ActionCtrlType ctrl;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();

    if(endId>startId)
    {
        start = endId;
        i = startId;
    }
    else
    {
        start = startId;
        i = endId;
    }
    for(;i<=start;i++)
    {
        if(i == ScSelfAddr)
        {
            ctrl.CtrlCmd = cmd;
            ctrl.CtrlMode = pState->ActionCtrl.CtrlMode;
            ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType;
            ctrl.CtrlSourceAddr = (u8)(ScSelfAddr&0xFF);
            ctrl.UserID =0;

            if(pParam->actionParam.actionID ==0)
            {
                return 0;
            }
            if(pParam->baseParam.enable == EnableType_DISABLE)
                continue;
            if(pParam->actionParam.actionType == groupActionEnum_ActionType_SINGLE)
            {
                SigActionCtrlFunc(ctrl,pParam->actionParam.actionID-1,0);//,pParam->baseParam.ID);
            }
            else
                CombActionCtrlFunc(ctrl,pParam->actionParam.actionID-1);
        }
        else
        {
            groupAction_CtrlAction(i,pParam, cmd);
        }
    }
}


static u8 serverReportAction_group(u8 moduleID)
{
    u8 temp[40];
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

    temp[0] = REPORTENUM_ACTION;
    temp[1] = ENUM_GROUP_ACTION;
    temp[2] = groupActionModule[moduleID].pParam->baseParam.ID;
    length =sizeof(groupActionStateType);
    memcpy(&temp[3],&length,2);
    memcpy(&temp[5],groupActionModule[moduleID].pState,length);

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    serverID = pParamMng->GetRunValueGlobalSysServerAddr();
    UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                   temp,5+length);
    return 1;
}

/*******************************************************************************************
**函数名称：chackBSStateStopAction_group
**函数作用：判断成组范围内是否有闭锁，有闭锁停所有动作
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
static u8 chackBSStateStopAction_group(u8 moduleID)
{
    u_int16_t i;
    u_int16_t start,end;
    u_int16_t startId,endId;
    u8 ret = 0;

    startId = groupActionModule[moduleID].startID;
    endId = groupActionModule[moduleID].endID;

    if(startId>endId)
    {
        start = startId;
        end = endId;
    }
    else
    {
        start = endId;
        end = startId;
    }

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t MinSupportID = pParamMng->GetRunValueGlobalMinSupportID();
    u_int16_t MaxSupportID = pParamMng->GetRunValueGlobalMaxSupportID();

    end = (end == MinSupportID)?MinSupportID:--end;
    start = (start == MaxSupportID)?MinSupportID:++start;
    HysSystem *pSystem = HysSystem::getInston();

    for(i = end;i<=start;i++)
    {
        if(pSystem->GetSysStopStateLockFlag(i) == true)
        {
            stopGroupAction(startId,endId,moduleID,ActionCtrl_CmdType_STOP);
            ret = 1;
            break;
        }
    }
    return ret;
}


u8 getGroupAction_SingleState(u8 moduleID)
{
    groupActionParamType *pParam = groupActionModule[moduleID].pParam;

    Action_RunStateType singleModuletype;
    if(getSingleActionState(pParam->actionParam.actionID-1,&singleModuletype))
    {
        if(singleModuletype == RunStateType_IDLE||singleModuletype == RunStateType_STOP)
            return 1;
    }
    return 0;
}


u8 groupActionStepProcess(u16 delay,u8 moduleID)
{
    groupActionParamType *pParam = groupActionModule[moduleID].pParam;
    groupActionStateType *pState = groupActionModule[moduleID].pState;
    u16 runState;
    u8 i,ret;
    u8 startid,endid,groupEndid,currentid;
    u8 alarmMode[2];
    u8 loginfo[6];

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();
    u_int16_t SupportDirect = pParamMng->GetRunValueGlobalSupportDirect();

    if((pState == nullptr)||(pParam == nullptr))
        return 0;
//	if(pParam->baseParam.enable == EnableType_DISABLE)
//		return 0;

    if((groupActionModule[moduleID].suspend_flag)&&(pState->RunState==RunStateType_SUSPEND))
        return 0;
    else
        groupActionModule[moduleID].suspend_flag =0;

    runState = pState->RunState;
    if(pState->RunStateBak != pState->RunState)
    {
        serverReportAction_group(moduleID);
    }

    switch(pState->RunState)
    {
        case RunStateType_IDLE:
        {
            groupActionModule[moduleID].currentOperateId=0;
            groupActionModule[moduleID].ticks_delay=0;
            groupActionModule[moduleID].groupFinish=0;
            groupActionModule[moduleID].groupEndID =0;
            memset(TiggerTicksFlag[moduleID],0,GROUP_ACTION_TRIGGER_MAX);
            memset(TiggerTicks[moduleID],0,GROUP_ACTION_TRIGGER_MAX*sizeof(int));
        }
        break;

        case RunStateType_ALARM:
        {
            ret = getGroupActionScope(&startid,&endid,&groupEndid,pParam,moduleID);
            if(ret ==0)
            {
                pState->RunState = RunStateType_IDLE;
                break;
            }
            if(chackBSStateStopAction_group(moduleID))
            {
                pState->RunState = RunStateType_STOP;
                break;
            }
            if(pState->RunStateBak != pState->RunState)
            {
                currentid = ScSelfAddr&0xFF;
                if(((groupActionModule[moduleID].direct == TriggerDirectType_LEFT)&&(SupportDirect>0))
                   ||((groupActionModule[moduleID].direct == TriggerDirectType_RIGHT)&&(SupportDirect==0)))
                {
                    for(i=startid; i>=groupEndid;)
                    {
                        if( i== currentid)
                        {
                            alarmMode[0] = pParam->baseParam.alarmMode_sound;
                            alarmMode[1] = pParam->baseParam.alarmMode_light;
                            Alarm *pAlarm = Alarm::getInston();
                            pAlarm->CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime,BaseCtrlType_STARTUP,\
                                              alarmMode,ENUM_GROUP_ACTION,moduleID+1,ALARM_MODE_1);
                        }
                        else
                        {
                            groupAction_CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime, i,BaseCtrlType_STARTUP);
                        }

                        if(i >= (pParam->actionParam.actionGap+1))
                            i=i-(pParam->actionParam.actionGap+1);
                        else
                            break;
                    }
                }
                else
                {
                    for(i=startid; i<=groupEndid;i=i+pParam->actionParam.actionGap+1)
                    {
                        if( i== currentid)
                        {
                            alarmMode[0] = pParam->baseParam.alarmMode_sound;
                            alarmMode[1] = pParam->baseParam.alarmMode_light;
                            Alarm *pAlarm = Alarm::getInston();
                            pAlarm->CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime,BaseCtrlType_STARTUP,\
                                              alarmMode,ENUM_GROUP_ACTION,moduleID+1,ALARM_MODE_1);
                        }
                        else
                        {
                            groupAction_CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime, i,BaseCtrlType_STARTUP);
                        }
                    }
                }
                qDebug("group %d alarm ",moduleID+1);
            }

            groupActionModule[moduleID].ticks_delay +=delay;
            if(groupActionModule[moduleID].ticks_delay > (int)(pParam->baseParam.alarmTime ) *LOGIC_TIME_MS)
            {
                groupActionModule[moduleID].ticks_delay =0;
                pState->RunState = RunStateType_STARTUP;

                currentid = ScSelfAddr&0xFF;
                if(((groupActionModule[moduleID].direct == TriggerDirectType_LEFT)&&(SupportDirect>0))
                  ||((groupActionModule[moduleID].direct == TriggerDirectType_RIGHT)&&(SupportDirect==0)))
                {
                    for(i=startid; i>=groupEndid;)
                    {
                        if( i== currentid)
                        {
                            alarmMode[0] = pParam->baseParam.alarmMode_sound;
                            alarmMode[1] = pParam->baseParam.alarmMode_light;
                            Alarm *pAlarm = Alarm::getInston();
                            pAlarm->CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime,BaseCtrlType_STOP,\
                                              alarmMode,ENUM_GROUP_ACTION,moduleID+1,ALARM_MODE_NONE);
                        }
                        else
                        {
                            groupAction_CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime, i,BaseCtrlType_STOP);
                        }

                        if(i >= (pParam->actionParam.actionGap+1))
                            i=i-(pParam->actionParam.actionGap+1);
                        else
                            break;
                    }
                }
                else
                {
                    for(i=startid; i<=groupEndid;i=i+pParam->actionParam.actionGap+1)
                    {
                        if( i== currentid)
                        {
                            alarmMode[0] = pParam->baseParam.alarmMode_sound;
                            alarmMode[1] = pParam->baseParam.alarmMode_light;
                            Alarm *pAlarm = Alarm::getInston();
                            pAlarm->CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime,BaseCtrlType_STOP,\
                                              alarmMode,ENUM_GROUP_ACTION,moduleID+1,ALARM_MODE_NONE);
                        }
                        else
                        {
                            groupAction_CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime, i,BaseCtrlType_STOP);
                        }
                    }
                }
            }
        }
        break;

        case RunStateType_STARTUP:
        {
            qDebug("group %d startup ",moduleID+1);
            //memset(&groupActionModule[moduleID].actionSupport,0,GROUP_ACTION_SUPPORT_NUM_MAX);
            ret = groupActionTiggerCond(TriggerActionPoint_START,groupActionEnum_triggerType_RUN,pParam,moduleID,delay);
            if(ret==0)
                pState->RunState = RunStateType_IDLE;

            ret = groupActionOperate(moduleID,ActionCtrl_CmdType_STARTUP);
            if(ret==0)
                pState->RunState = RunStateType_IDLE;
            else
            {
                pState->RunState = RunStateType_RUN;
                groupActionModule[moduleID].ticks_delay =0;
            }

            loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
            loginfo[1]=pState->ActionCtrl.CtrlSourceType;
            memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
            loginfo[4]=pState->ActionCtrl.CtrlMode;
            HysSystem *pSystem = HysSystem::getInston();
            pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_GROUP,LOG_STATUS_START,loginfo,5);
        }
        break;

        case RunStateType_RUN:
        {
            getGroupActionScope(&startid,&endid,&groupEndid,pParam,moduleID);
            chackBSStateStopAction_group(moduleID);
            if(((groupActionModule[moduleID].direct == TriggerDirectType_LEFT)&&(SupportDirect>0)&&(groupActionModule[moduleID].currentOperateId<endid))||
               ((groupActionModule[moduleID].direct == TriggerDirectType_RIGHT)&&(SupportDirect==0)&&(groupActionModule[moduleID].currentOperateId<endid))||
               ((groupActionModule[moduleID].direct == TriggerDirectType_RIGHT)&&(SupportDirect>0)&&(groupActionModule[moduleID].currentOperateId>endid))||
               ((groupActionModule[moduleID].direct == TriggerDirectType_LEFT)&&(SupportDirect==0)&&(groupActionModule[moduleID].currentOperateId>endid)))
            {
                if(getGroupAction_SingleState(moduleID))
                    pState->RunState = RunStateType_STOP;
                break;
            }

            if(pParam->actionParam.delayTime !=0xffff)
            {
                groupActionModule[moduleID].ticks_delay +=delay;
            }
            if(groupActionModule[moduleID].ticks_delay > (int)(pParam->actionParam.delayTime ) *LOGIC_TIME_MS)
            {
                groupActionModule[moduleID].ticks_delay =0;
                ret = groupActionOperate(moduleID,ActionCtrl_CmdType_STARTUP);
//				if(ret==0)
//					pState->RunState = RunStateType_IDLE;
            }
            else
            {
                ret = groupActionTiggerCond(TriggerActionPoint_RUN,groupActionEnum_triggerType_NEXTRUN,pParam,moduleID,delay);
                if(ret)
                {
                    ret = groupActionOperate(moduleID,ActionCtrl_CmdType_STARTUP);
                    if(ret)
                    {
                        groupActionModule[moduleID].ticks_delay =0;
                    }

                    memset(TiggerTicksFlag[moduleID],0,GROUP_ACTION_TRIGGER_MAX);
                    memset(TiggerTicks[moduleID],0,GROUP_ACTION_TRIGGER_MAX*sizeof(int));
                }
            }

            if(groupActionModule[moduleID].groupFinish)
                if(getGroupAction_SingleState(moduleID))
                    pState->RunState = RunStateType_STOP;
        }
        break;

        case RunStateType_SUSPEND:
        {
            groupActionModule[moduleID].suspend_flag =1;
            qDebug("group %d suspend ",moduleID+1);

            loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
            loginfo[1]=pState->ActionCtrl.CtrlSourceType;
            memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
            loginfo[4]=pState->ActionCtrl.CtrlMode;
            HysSystem *pSystem = HysSystem::getInston();
            pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_GROUP,LOG_STATUS_SUSPEND,loginfo,5);
        }
        break;

        case RunStateType_STOP:
        {
            ret = getGroupActionScope(&startid,&endid,&groupEndid,pParam,moduleID);
            if(ret ==0)
            {
                pState->RunState = RunStateType_IDLE;
                break;
            }
            if(pState->RunStateBak == RunStateType_RUN)
            {
                chackBSStateStopAction_group(moduleID);
            }
            if(pState->RunStateBak == RunStateType_ALARM)//首架启动后，停止，闭锁，清除组内声光报警器
            {
                currentid = ScSelfAddr&0xFF;
                if(((groupActionModule[moduleID].direct == TriggerDirectType_LEFT)&&(SupportDirect>0))
                  ||((groupActionModule[moduleID].direct == TriggerDirectType_RIGHT)&&(SupportDirect==0)))
                {
                    for(i=startid; i>=groupEndid;)
                    {
                        if( i!= currentid)
                        {
                            groupAction_CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime, i,BaseCtrlType_STOP);
                        }

                        if(i >= (pParam->actionParam.actionGap+1))
                            i=i-(pParam->actionParam.actionGap+1);
                        else
                            break;
                    }
                }
                else
                {
                    for(i=startid; i<=groupEndid;i=i+pParam->actionParam.actionGap+1)
                    {
                        if( i!= currentid)
                        {
                            groupAction_CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime, i,BaseCtrlType_STOP);
                        }
                    }
                }
            }

            pState->RunState = RunStateType_IDLE;
            qDebug("group %d stop ",moduleID+1);

            loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
            loginfo[1]=pState->ActionCtrl.CtrlSourceType;
            memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
            loginfo[4]=pState->ActionCtrl.CtrlMode;
            HysSystem *pSystem = HysSystem::getInston();
            pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_GROUP,LOG_STATUS_STOP,loginfo,5);
        }
        break;

        default:
            break;
    }

    if(groupActionModule[moduleID].suspend_flag ==0)
        pState->RunStateBak = runState;

    return 1;
}

u8 groupActionCtrlFunc(ActionCtrlType ctrl,u8 actionID,u8 *pctrlParam,u8 length)
{
    groupActionParamType *pParam = groupActionModule[actionID].pParam;
    groupActionStateType *pState = groupActionModule[actionID].pState;

    if((pState == NULL)||(pParam == NULL))
        return 0;
//	if(pParam->baseParam.enable == EnableType_DISABLE)
//		return 0;

    if((length >=GroupModuleParamENUM_MAX)&&(pctrlParam !=NULL))
    {
        groupActionModule[actionID].startID = pctrlParam[0];
        groupActionModule[actionID].endID   = pctrlParam[1];
        groupActionModule[actionID].direct  = pctrlParam[2];
    }
    else
    {
        groupActionModule[actionID].startID = 0;
        groupActionModule[actionID].endID   = 0;
        groupActionModule[actionID].direct  = 0;
    }
    groupActionAutoCtrlFunc(ctrl,pParam,pState);
    return 1;
}

u8 groupActionAllStop(u8 actionID)
{
    groupActionParamType *pParam = groupActionModule[actionID].pParam;
    groupActionStateType *pState = groupActionModule[actionID].pState;

    if((pState == NULL)||(pParam == NULL))
        return 0;
//	if(pParam->baseParam.enable == EnableType_DISABLE)
//		return 0;

    if(pState->RunState != RunStateType_IDLE)
        pState->RunState = RunStateType_STOP;
    return 1;
}


void groupActionJudge(ActionCtrlType ctrl,u8 actionID)
{
    ActionCtrlType ctrlTmp;
    if(actionID > GROUPACTION_NUM_MAX)
        return;

    groupActionParamType *pParam = groupActionModule[actionID].pParam;
    groupActionStateType *pState = groupActionModule[actionID].pState;

    if((pParam == nullptr)||(pState == nullptr))
        return ;

    if(pParam->baseParam.enable == EnableType_DISABLE)
        return;
    if(pParam->actionParam.actionID == 0)
        return;


    ctrlTmp.CtrlCmd = ctrl.CtrlCmd;
    ctrlTmp.CtrlMode =ctrl.CtrlMode;
    ctrlTmp.UserID =0;
    ctrlTmp.CtrlSourceAddr = ctrl.CtrlSourceAddr;
    ctrlTmp.CtrlSourceType = ActionCtrl_SourceType_CONTROLLER;

    if(pParam->actionParam.actionType == groupActionEnum_ActionType_SINGLE)
    {
        SigActionCtrlFunc(ctrlTmp,pParam->actionParam.actionID-1,0);//,pParam->baseParam.ID);
    }
    else
    {
        CombActionCtrlFunc(ctrlTmp,pParam->actionParam.actionID-1);
    }

}

