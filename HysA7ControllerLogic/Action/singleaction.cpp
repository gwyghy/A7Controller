#include "string.h"
#include <QDebug>
#include "singleaction.h"
#include "combaction.h"
#include "alarm.h"
#include "libhysa7parammng.h"
#include "./HysSystem/hyssystem.h"
#include "actionctrl.h"
#include "./Sensor/sensor.h"

/****/
#define     SINGLE_MANUACTRLL_HERATBEAT_TIMEOUT_SET       300//手动控制时，心跳维持的时间

/*****/
SingleActionModuleType SigActionModule[SINGLE_MODULE_MAX];
static TiggerTimerType		TiggerTimerMng[TRIGGER_TIMER_MAX];
static	u8	TiggerTimerCnt=0;


/*******************************************************************************************
*功能描述：单动作触发定时器初始化
*入口参数：无
*返回值：  无
*******************************************************************************************/
void SigActionTrggerTimerInit(void)
{
    memset(TiggerTimerMng,0,sizeof(TiggerTimerMng));
    TiggerTimerCnt=0;
}

/*******************************************************************************************
*功能描述：单动作模型初始化
*入口参数：无
*返回值：  无
*******************************************************************************************/
void SigActionModuleInit(void)
{
    SigActionTrggerTimerInit();
    memset(SigActionModule,0,sizeof(SigActionModule));
}

/*******************************************************************************************
*功能描述：传感器模型初始化
*入口参数：moduleID取值：
*            单动作模型id，从0开始。注意
*          pParam取值：
*             单动作参数指针
*          pParam取值：
*             单动作状态指针
*返回值：  无
*******************************************************************************************/
void SigActionModuleLoad(u8 moduleID,SingleActionParamType *pParam,SingleActionStateType *pState)
{
    u8 i;
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();

    if((pParam == NULL)||(moduleID >=SINGLE_MODULE_MAX))
        return;

    SigActionModule[moduleID].pParam = pParam;
    if(pState!=NULL)
    {
        SigActionModule[moduleID].pState = pState;
        SigActionModule[moduleID].apiFunc[SIGACTIONAPI_NONE] = NULL;
        SigActionModule[moduleID].apiFunc[SIGACTIONAPI_COMB_STOP] = sigActionApi_CombStopFunc;//停止组合动作所有单动作
    }
    else
    {
        SingleActionParamType *pSingleParam;
        pSingleParam = pParamMng->GetRunValueSingleActParam(moduleID+1);
        if(pSingleParam != NULL)//从一开始,备注
            SigActionModule[moduleID].pParam->baseParam.pointID = pSingleParam->baseParam.pointID;
    }
}

/*******************************************************************************************
*功能描述：传感器模型初始化
*入口参数：actionID取值：
*            单动作模型id
*          combID取值：
*             组合动作id
*          pdata取值：
*             动作控制器类型指针
*返回值：  无
*******************************************************************************************/
int sigActionApi_CombStopFunc (u8 actionID,u8 combID,void *pdata)
{
    ActionCtrlType *pCtrl =(ActionCtrlType *)pdata;
    ActionCtrlType ctrl;
    int ret;

    if((combID == 0)||(combID >COMBACTION_NUM_MAX))
        return 0;

    ctrl.CtrlCmd = ActionCtrl_CmdType_STOP;
    ctrl.CtrlMode = pCtrl->CtrlMode;
    ctrl.CtrlSourceType = pCtrl->CtrlSourceType ;//ActionCtrl_SourceType_CONTROLLER;
    ctrl.CtrlSourceAddr = SigActionModule[actionID].pState->ActionCtrl.CtrlSourceAddr;
    ctrl.UserID =0;

    CombActionCtrlFunc(ctrl,combID-1);

    return 1;
}


/*******************************************************************************************
*功能描述：触发条件查找
*入口参数：ptrigger取值：
*            触发条件指针
*返回值：  无
*******************************************************************************************/
static int TiggerCondFound(SingleAction_TriggerCondType *ptrigger)
{
    SensorApiType apiFunc;
    int ret=0;

    if((ptrigger == NULL)||(ptrigger->TriggerEnable == EnableType_DISABLE))
        return -1;

    if(ptrigger->sensorID ==0)
        return 1;

    apiFunc = getSensorApi(ptrigger->methodID);
    if(apiFunc==NULL)
        return -1;

    ret = apiFunc(ptrigger->sensorID,ptrigger->sensorID2,ptrigger->sensorParam1,ptrigger->sensorParam2,0);
    return ret;
}

static u8 TiggerActionCtrl_alarm(u8 id)
{
    //todo
    return 1;
}

/*******************************************************************************************
*功能描述：can接收超时处理函数
*入口参数：u16Way取值：
*             can帧来源（can1、can2）
*          RevBufTemp取值：
*             can帧
*返回值：  无
*******************************************************************************************/
static u8 TiggerActionCtrl_self(u8 id,u8 moduleID,u8 cmd,ActionCtrlType *pCtrl,u8 *startupFlag)
{
    ActionCtrlType ctrl;
    SingleActionParamType *pactionParam;
    bool ret;
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();

    if/*((cmd ==ActionCtrl_CmdType_NONE)||*/(cmd >= ActionCtrl_CmdTypeMAX)
        return 0;

    if(*startupFlag ==1)
    {
        if((id == (moduleID+1))&&(cmd == ActionCtrl_CmdType_STOP))
        {
            *startupFlag =2;
            return 0;
        }
    }

    ctrl.CtrlCmd = cmd; //ActionCtrl_CmdType_WAITSTARTUP;
    ctrl.CtrlMode = pCtrl->CtrlMode;
    ctrl.CtrlSourceType = pCtrl->CtrlSourceType ;//ActionCtrl_SourceType_CONTROLLER;
    ctrl.CtrlSourceAddr = SigActionModule[moduleID].pState->ActionCtrl.CtrlSourceAddr;
    ctrl.UserID =0;

    if(SigActionModule[moduleID].combAction_num)
    {
        pactionParam = pParamMng->GetRunValueCommbSingleActParam(SigActionModule[moduleID].combAction_num,id);
        if(pactionParam != NULL)
        {
            SigActionModuleLoad(id-1,pactionParam,0);
        }
        else
        {
            LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
            pactionParam = pParam->GetRunValueSingleActParam(SigActionModule[moduleID].pParam->baseParam.ID);
            if(pactionParam == NULL)
                return 0;

            SigActionModuleLoad(id-1,pactionParam,0);
            SigActionCtrlFunc(ctrl,id-1,0);
            qDebug("single ctrl self id: %d cmd: %d",id,cmd);
            return 1;
        }
    }

    SigActionCtrlFunc(ctrl,id-1,SigActionModule[moduleID].combAction_num);
    qDebug("single ctrl self id: %d cmd: %d,id:",id ,cmd);

    return 1;
}


/*******************************************************************************************
*功能描述：单动作触发其他架动作（助推）
*入口参数：id取值：
*             单动作id
*          ptrigger取值：
*             触发动作指针
*          cmd取值：
*             触发命令
*          pCtrl取值：
*             控制结构体
*返回值：  无
*******************************************************************************************/
static u8 TiggerActionCtrl_other(u8 id,SingleAction_TriggerActionType *ptrigger,u8 cmd,ActionCtrlType *pCtrl)
{
    u8 temp[9],startaddr,endaddr,i;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();
    u_int16_t SupportDirect = pParamMng->GetRunValueGlobalSupportDirect();

    if(ptrigger == NULL)
        return 0;

    if/*((cmd ==ActionCtrl_CmdType_NONE)||*/(cmd >= ActionCtrl_CmdTypeMAX)
        return 0;

    //u8 Type+u8 Id+u8 ControlCommand+u8 ControlType+u8 ControlParamLength+(u8 SourceAddr+u8 SourceType+u16 UserId+u8 ControlParamData[])
    memset(temp,0,sizeof(temp));
    temp[0] = ENUM_SINGLE_ACTION;
    temp[1] = id;
    temp[2] = cmd; //ActionCtrl_CmdType_WAITSTARTUP;
    temp[3] = pCtrl->CtrlMode;  //ActionCtrl_ModeType_AUTO;
    temp[4] = 0;
    temp[5] = pCtrl->CtrlSourceAddr;
    temp[6] = pCtrl->CtrlSourceType ;//ActionCtrl_SourceType_CONTROLLER;
    temp[7] = 0;//用户ID

    if(SupportDirect>0)
    {
        if((ptrigger->TriggerDirection == TriggerDirectType_LEFT)||(ptrigger->TriggerDirection == TriggerDirectType_BOTH))
        {
            if(ScSelfAddr <= ptrigger->TriggerGap)
                return 0;

            endaddr = startaddr = ScSelfAddr - ptrigger->TriggerGap;
            for(i=0; i<ptrigger->TriggerSupportNum;i++)
            {
                if(endaddr >SUPPORT_ADDR_MIN)
                    endaddr--;
                else
                    break;
            }

            for(i=startaddr;i>=endaddr;i--)
            {
                if(i == ScSelfAddr)
                    continue;
                UdpAppProcCtrl(V4APP_DEV_TYPE_SC,i,V4APP_STATUS_CTRL_DEV,temp,8);
            }
        }

        if((ptrigger->TriggerDirection == TriggerDirectType_RIGHT)||(ptrigger->TriggerDirection == TriggerDirectType_BOTH))
        {
            if((ScSelfAddr + ptrigger->TriggerGap)>SUPPORT_ADDR_MAX)
                return 0;

            endaddr = startaddr = ScSelfAddr + ptrigger->TriggerGap;
            for(i=0; i<ptrigger->TriggerSupportNum;i++)
            {
                if(endaddr <SUPPORT_ADDR_MAX)
                    endaddr++;
                else
                    break;
            }

            for(i=startaddr;i<=endaddr;i++)
            {
                if(i == ScSelfAddr)
                    continue;
                UdpAppProcCtrl(V4APP_DEV_TYPE_SC,i,V4APP_STATUS_CTRL_DEV,temp,8);
            }
        }
    }
    else
    {
        if((ptrigger->TriggerDirection == TriggerDirectType_RIGHT)||(ptrigger->TriggerDirection == TriggerDirectType_BOTH))
        {
            if(ScSelfAddr <= ptrigger->TriggerGap)
                return 0;

            endaddr = startaddr = ScSelfAddr - ptrigger->TriggerGap;
            for(i=0; i<ptrigger->TriggerSupportNum;i++)
            {
                if(endaddr >SUPPORT_ADDR_MIN)
                    endaddr--;
                else
                    break;
            }

            for(i=startaddr;i>=endaddr;i--)
            {
                if(i == ScSelfAddr)
                    continue;
                UdpAppProcCtrl(V4APP_DEV_TYPE_SC,i,V4APP_STATUS_CTRL_DEV,temp,8);
            }
        }

        if((ptrigger->TriggerDirection == TriggerDirectType_LEFT)||(ptrigger->TriggerDirection == TriggerDirectType_BOTH))
        {
            if((ScSelfAddr + ptrigger->TriggerGap)>SUPPORT_ADDR_MAX)
                return 0;

            endaddr = startaddr = ScSelfAddr + ptrigger->TriggerGap;
            for(i=0; i<ptrigger->TriggerSupportNum;i++)
            {
                if(endaddr <SUPPORT_ADDR_MAX)
                    endaddr++;
                else
                    break;
            }

            for(i=startaddr;i<=endaddr;i++)
            {
                if(i == ScSelfAddr)
                    continue;
                UdpAppProcCtrl(V4APP_DEV_TYPE_SC,i,V4APP_STATUS_CTRL_DEV,temp,8);
            }
        }
    }

    return 1;
}


/*******************************************************************************************
*功能描述：插入延时触发队列
*入口参数：id取值：
*             单动作id
*          ptrigger取值：
*             触发动作指针
*          cmd取值：
*             触发命令
*          pCtrl取值：
*             控制结构体
*返回值：  无
*******************************************************************************************/
static void InsertTriggerDelayBuf(SingleAction_TriggerActionType *ptrigger,u8 moduleID,ActionCtrlType *pCtrl,u8 combID)
{
    u8 i;

    if(TiggerTimerCnt < TRIGGER_TIMER_MAX)
    {
        for(i=0;i<TRIGGER_TIMER_MAX;i++)
        {
            if(TiggerTimerMng[i].enbale == 0)
            {
                TiggerTimerMng[i].enbale =1;
                TiggerTimerMng[i].pTigger = ptrigger;
                TiggerTimerMng[i].pCtrl = pCtrl;
                TiggerTimerMng[i].ticks = 0;
                TiggerTimerMng[i].moduleID = moduleID;
                TiggerTimerMng[i].combID = combID;
                TiggerTimerCnt++;
                break;
            }
        }
    }
}

static void putTriggerDelayBuf(u8 id)
{
    if(id < TRIGGER_TIMER_MAX)
    {
        TiggerTimerMng[id].enbale =0;
        TiggerTimerMng[id].pTigger = NULL;
        TiggerTimerMng[id].pCtrl = NULL;
        TiggerTimerMng[id].ticks = 0;
        TiggerTimerMng[id].moduleID =0;
        TiggerTimerMng[id].combID =0;
        if(TiggerTimerCnt >0)
            TiggerTimerCnt--;
    }
}

static void deleteTriggerDelayBuf(u8 moduleID,u8 combID)
{
    u8 i;

    if(TiggerTimerCnt ==0)
        return ;

    for(i=0;i<TRIGGER_TIMER_MAX;i++)
    {
        if((TiggerTimerMng[i].enbale )&&(TiggerTimerMng[i].moduleID==moduleID)
          &&(TiggerTimerMng[i].combID==combID))
        {
            TiggerTimerMng[i].enbale =0;
            TiggerTimerMng[i].pTigger = NULL;
            TiggerTimerMng[i].pCtrl = NULL;
            TiggerTimerMng[i].ticks = 0;
            TiggerTimerMng[i].moduleID =0;
            TiggerTimerMng[i].combID =0;
            if(TiggerTimerCnt >0)
                TiggerTimerCnt--;
        }
    }
}

static u8 TiggerActionProcess(SingleAction_TriggerActionType *ptrigger,u8 timersel,u8 moduleID,u8 sel,ActionCtrlType *pCtrl,u8 *startupFlag)
{
    if((ptrigger == NULL)||(ptrigger->TriggerActionEnable == EnableType_DISABLE))
        return 0;

    if((timersel >0)||(ptrigger->TriggerDelayTime == 0))
    {
        if(ptrigger->TriggerEventType == TriggerEventType_ACTION)
        {
            if(ptrigger->TriggerType == TriggerType_SELF)
            {
                //动作到位
                if((sel)&&((moduleID +1)== ptrigger->TriggerActionID)&&(ptrigger->TriggerActionCmd == ActionCtrl_CmdType_STOP))
                {
                    HysSystem *pSystem = HysSystem::getInston();
                    pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_SINGLE,LOG_STATUS_INPLACE,0,0);
                }

                if((ptrigger->ExtTriggerFuncID>0)&&(ptrigger->ExtTriggerFuncID<SIGACTIONAPI_MAX))
                {
                    SigActionModule[moduleID].apiFunc[ptrigger->ExtTriggerFuncID](moduleID,SigActionModule[moduleID].combAction_num,(void *)pCtrl);
                    if(*startupFlag ==1)
                    {
                        *startupFlag =2;
                    }
                }
                else
                    TiggerActionCtrl_self(ptrigger->TriggerActionID,moduleID,ptrigger->TriggerActionCmd,pCtrl,startupFlag);
            }
            else
            {
                TiggerActionCtrl_other(ptrigger->TriggerActionID,ptrigger,ptrigger->TriggerActionCmd,pCtrl);
            }
        }
        else if(ptrigger->TriggerEventType == TriggerEventType_ALARM)
        {
            TiggerActionCtrl_alarm(ptrigger->TriggerActionID);
        }
        return 1;
    }
    else if(ptrigger->TriggerDelayTime > 0)
    {
        InsertTriggerDelayBuf(ptrigger,moduleID,pCtrl,SigActionModule[moduleID].combAction_num);
    }

    return 0;
}

static u8 TiggerActionProcess_stop(SingleAction_TriggerActionType *ptrigger,u8 moduleID,ActionCtrlType *pCtrl)
{
    if((ptrigger == NULL)||(ptrigger->TriggerActionEnable == EnableType_DISABLE))
        return 0;

    if(ptrigger->TriggerEventType == TriggerEventType_ACTION)
    {
        if(ptrigger->TriggerType == TriggerType_SELF)
        {
            //动作到位
            if(((moduleID +1)== ptrigger->TriggerActionID)&&(ptrigger->TriggerActionCmd == ActionCtrl_CmdType_STOP))
            {
                return 1;
            }
        }
    }
    return 0;
}

static u8 SigActionTiggerProcess(TriggerActionPointType type,SingleActionParamType *pParam,u8 moduleID,ActionCtrlType *pCtrl)
{
    u8 i,j;
    u8 ret=0;
    u8 flag;
    int tiggercond;

    for(i=0; i<SINGLE_ACTION_TRIGGER_MAX;i++)
    {
        if(pParam->tiggerParam[i].cond.TriggerEnable == EnableType_DISABLE)
            continue;

        if(pParam->tiggerParam[i].cond.TriggerActionPoint != type)
            continue;

        tiggercond = TiggerCondFound(&(pParam->tiggerParam[i].cond));
        if(tiggercond <1)
            continue;
//		if((tiggercond ==0)||((tiggercond <0)&&(type != TriggerActionPoint_START)))
//			continue;

//		if(((tiggercond <0)&&(type == TriggerActionPoint_START)))
//		{
//			for(j=0; j<TRIGGER_ACTION_MAX;j++)
//			{
//				flag = TiggerActionProcess_stop(&(pParam->tiggerParam[i].action[j]),moduleID,pCtrl);
//				if(flag)
//					ret=1;
//			}
//		}
//		else
//		{
            for(j=0; j<TRIGGER_ACTION_MAX;j++)
            {
                if(type == TriggerActionPoint_START)
                    flag =1;
                else
                    flag =0;
                TiggerActionProcess(&(pParam->tiggerParam[i].action[j]),0,moduleID,(tiggercond==2)? 1:0,pCtrl,&flag);
                if(flag==2)
                    ret=1;
            }
//		}
    }

    return ret;
}

static u8 SigActionAutoCtrlFunc(ActionCtrlType ctrl,SingleActionParamType *pParam,SingleActionStateType *pState,u8 actionID)
{
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
            deleteTriggerDelayBuf(actionID,SigActionModule[actionID].combAction_num);
            if(pState->RunState == RunStateType_IDLE)
                return 0;

            pState->RunState = RunStateType_STOP;
            //add log
        }
        break;

        case ActionCtrl_CmdType_SUSPEND:
        {
            if(pState->RunState == RunStateType_IDLE)
                return 0;

            pState->RunStateBak = pState->RunState;
            pState->RunState = RunStateType_SUSPEND;
            //add log
        }
        break;

        case ActionCtrl_CmdType_RESUME:
        {
            if((pState->RunState == RunStateType_SUSPEND)||(pState->RunState == RunStateType_SUSPEND_ACTION))
            {
                pState->RunState = pState->RunStateBak;
                pState->RunStateBak = ActionCtrl_CmdType_RESUME;
            }
            else
                return 0;

            //add log
        }
        break;

        case ActionCtrl_CmdType_SUSPEND_ACTION:
        {
            if(pState->RunState != RunStateType_RUN)
                return 0;

            pState->RunStateBak = pState->RunState;
            pState->RunState = RunStateType_SUSPEND_ACTION;
            //add log
        }
        break;

        default:
            return 0;
    }

    memcpy(&(pState->ActionCtrl),&ctrl,sizeof(ActionCtrlType));
    return 1;
}

u8 singleActionReportState[150];
static u8 serverReportAction_single(u8 moduleID)
{
    u8 serverID;
    u16 length;
    uint sensorLen;

    u16 ServerEnabled = 0;
    u16 ServerCommErr = 0;

    HysSystem *pSystem = HysSystem::getInston();

    pSystem->GetRunScSelfServerEnabled(ServerEnabled);
    pSystem->GetRunScSelfServerCommErr(ServerCommErr);
    if((ServerEnabled == 0)||(ServerCommErr == 1))
        return 0;

    memset(singleActionReportState,0,sizeof(singleActionReportState));

    singleActionReportState[0] = REPORTENUM_ACTION;
    singleActionReportState[1] = ENUM_SINGLE_ACTION;
    singleActionReportState[2] = SigActionModule[moduleID].pParam->baseParam.ID;
    length =sizeof(SingleActionStateType);
    memcpy(&singleActionReportState[3],&length,2);
    memcpy(&singleActionReportState[5],SigActionModule[moduleID].pState,length);

    //jhy 20221008 report singleAction state and sonsor state
    sensorLen = pSystem->GetAllSensorStateDataToReport(&singleActionReportState[7+length]);
    memcpy(&singleActionReportState[5+length],&sensorLen,2);

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    serverID = pParamMng->GetRunValueGlobalSysServerAddr();
    UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                   singleActionReportState,5+length+sensorLen+2);
    return 1;
}

u8 getSingleActionState(u8 moduleID, Action_RunStateType* type)
{
    u16 runState;
    SingleActionParamType *pParam = SigActionModule[moduleID].pParam;
    SingleActionStateType *pState = SigActionModule[moduleID].pState;
    if((pState == NULL)||(pParam == NULL))
        return 0;
//	if(pParam->baseParam.enable == EnableType_DISABLE)
//		return 0;
    runState = pState->RunState;
    *type = static_cast<Action_RunStateType>(runState) ;
    return 1;
}

u8 SigActionStepProcess(u16 delay,u8 moduleID)
{
    SingleActionParamType *pParam = SigActionModule[moduleID].pParam;
    SingleActionStateType *pState = SigActionModule[moduleID].pState;
    u8 ret,i;
    u16 runState;
    u16 value = 0;
    u8 alarmMode[2];
    u8 loginfo[6];

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();

    if((pState == NULL)||(pParam == NULL))
        return 0;
    if(pParam->baseParam.enable == static_cast<u_int16_t>(EnableType_DISABLE))
        return 0;

    if((SigActionModule[moduleID].suspend_flag)&&(pState->RunState==RunStateType_SUSPEND))
        return 0;
    else if(pState->RunState !=RunStateType_SUSPEND_ACTION)
        SigActionModule[moduleID].suspend_flag =0;

    runState = pState->RunState;
    if(pState->RunStateBak != pState->RunState)
    {
        if(pState->RunStateBak == RunStateType_RUN_MANUAL && pState->RunState == RunStateType_IDLE)
        {
            pState->RunState = RunStateType_STOP;
            serverReportAction_single(moduleID);
            pState->RunState = RunStateType_IDLE;
            serverReportAction_single(moduleID);
        }
        else
            serverReportAction_single(moduleID);
        pState->RunTime = 0;
    }

    switch(pState->RunState)
    {
        case RunStateType_IDLE:
        {
            SigActionModule[moduleID].suspend_action_flag =0;
        }
        break;

        case RunStateType_ALARM:
        {
            if(pState->RunStateBak != pState->RunState)
            {
                qDebug()<<"single alarm ,id:"<<pState->Id;;
                alarmMode[0] = pParam->actionParam.alarmMode_sound;
                alarmMode[1] = pParam->actionParam.alarmMode_light;
                Alarm *pAlarm = Alarm::getInston();
                pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),pParam->actionParam.alarmTime,\
                                  BaseCtrlType_STARTUP,alarmMode,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_1);
            }
            SigActionModule[moduleID].ticks_alarm +=delay;
            if(SigActionModule[moduleID].ticks_alarm > (int)(pParam->actionParam.alarmTime) *LOGIC_TIME_MS)
            {
                SigActionModule[moduleID].ticks_alarm =0;
                pState->RunState = RunStateType_DELAY;
                /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
                u_int8_t alarmModetemp[2];
                alarmModetemp[0] = 0;
                alarmModetemp[1] = 0;
                Alarm *pAlarm = Alarm::getInston();
                pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,\
                                  BaseCtrlType_STOP,alarmModetemp,\
                                  ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_NONE);
            }
        }
        break;

        case RunStateType_DELAY:
        {
            if(pState->RunStateBak != pState->RunState)
            {
                qDebug()<<"single delay start ,id:"<<pState->Id;;
                alarmMode[0] = pParam->actionParam.alarmMode_sound;
                alarmMode[1] = pParam->actionParam.alarmMode_light;
                Alarm *pAlarm = Alarm::getInston();
                pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STARTUP,\
                                  alarmMode,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_1);
            }
            SigActionModule[moduleID].ticks_alarm +=delay;
            if(SigActionModule[moduleID].ticks_alarm > (int)(pParam->actionParam.delayTime) *LOGIC_TIME_MS)
            {
                SigActionModule[moduleID].ticks_alarm =0;
                pState->RunState = RunStateType_STARTUP;

                /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
                u_int8_t alarmModetemp[2];
                alarmModetemp[0] = 0;
                alarmModetemp[1] = 0;
                Alarm *pAlarm = Alarm::getInston();
                pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STOP,\
                                  alarmModetemp,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_NONE);
            }
        }
        break;

        case RunStateType_STARTUP:
        {
            ret = SigActionTiggerProcess(TriggerActionPoint_START,pParam,moduleID,&(pState->ActionCtrl));
            if(ret==1)
            {
                pState->RunState = RunStateType_STOP;
                break;
            }

            if(SigActionModule[moduleID].combAction_num >0)
                SigActionModule[moduleID].runFin[SigActionModule[moduleID].combAction_num -1] =0;

            runState = pState->RunState;
            if(pState->RunState == RunStateType_STARTUP)
            {
                qDebug()<<"single startup ,id:"<<pState->Id;;

                SigActionModule[moduleID].ticks_alarm =0;
                alarmMode[0] = SINGLEACTION_ALARM_MODE_SOUND_DEFAULT;
                alarmMode[1] = SINGLEACTION_ALARM_MODE_LIGHT_DEFAULT;
                Alarm *pAlarm = Alarm::getInston();
                pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STARTUP,\
                                  alarmMode,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_2);
                pState->RunState = RunStateType_RUN;
                value = BaseCtrlType_STARTUP;
                if(pParam->actionParam.execTime != 0)
                {
                    ActionCtrl *pActCtrl = ActionCtrl::getInston();
                    pActCtrl->AddActionInfo(ENUM_SINGLE_ACTION,moduleID+1,RunStateType_RUN);
                    DevCtrlInquire(pParam->baseParam.pointID.driver,\
                                   pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                                   pParam->baseParam.pointID.point,(u8 *)&value,0x02);
                }
                else
                {
                    SigActionModule[moduleID].ticks_run =0;
                    pState->RunState = RunStateType_STOP;
                }
            }
        }
        break;

        case RunStateType_RUN_MANUAL:
        {
            for(i=0;i<MANUALACTIONINFO_LEN;i++)
            {
                ActionCtrl *pActCtrl = ActionCtrl::getInston();
                uint ClearData = 0;
                if(pActCtrl->GetManualActionClearBuf(i,ClearData) == false)
                    break;

                if(static_cast<u8>(ClearData) == (moduleID+1))
                {
                    pActCtrl->SetManualActionClearBuf(i,0);
                    SigActionModule[moduleID].ticks_alarm =0;
                }
            }

            SigActionModule[moduleID].ticks_run +=delay;
            SigActionModule[moduleID].pState->RunTime = SigActionModule[moduleID].ticks_run/100;

            SigActionModule[moduleID].ticks_alarm +=delay;
            if(SigActionModule[moduleID].ticks_alarm > SINGLE_MANUACTRLL_HERATBEAT_TIMEOUT_SET)//电磁阀要求时间300ms
                //&& (SigActionModule[moduleID].pState->ActionCtrl.CtrlSourceAddr !=ScSelfAddr))//mry,23.06.09,自身控制时，也需要对心跳进行超时判断
            {
                qDebug()<<"*******single act manual stop, time out";
                SigActionModule[moduleID].ticks_alarm =0;
                ActionCtrl *pActCtrl = ActionCtrl::getInston();
                pActCtrl->AddActionInfo(ENUM_SINGLE_ACTION,moduleID+1,RunStateType_IDLE);

                alarmMode[0] = SINGLEACTION_ALARM_MODE_SOUND_DEFAULT;
                alarmMode[1] = SINGLEACTION_ALARM_MODE_LIGHT_DEFAULT;
                Alarm *pAlarm = Alarm::getInston();
                pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STOP,\
                                  alarmMode,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_NONE);
                pState->RunState = RunStateType_IDLE;
                SigActionModule[moduleID].ticks_run =0;
                value = DEV_POINT_CTRL_OFF;//BaseCtrlType_STOP;
                DevCtrlInquire(pParam->baseParam.pointID.driver,\
                               pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                               pParam->baseParam.pointID.point,(u8 *)&value,0x02);
                //add log
            }
        }
        break;

        case RunStateType_RUN:
        {
            if(pState->RunStateBak != pState->RunState)
            {
                ActionCtrl *pActCtrl = ActionCtrl::getInston();
                pActCtrl->AddActionInfo(ENUM_SINGLE_ACTION,moduleID+1,RunStateType_RUN);
                if(SigActionModule[moduleID].suspend_action_flag ==1)
                {
                    SigActionModule[moduleID].suspend_action_flag =0;
                }
                if(pParam->actionParam.execTime != 0)
                {
                    alarmMode[0] = SINGLEACTION_ALARM_MODE_SOUND_DEFAULT;
                    alarmMode[1] = SINGLEACTION_ALARM_MODE_LIGHT_DEFAULT;
                    Alarm *pAlarm = Alarm::getInston();
                    pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STARTUP,\
                                      alarmMode,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_2);
                    value = BaseCtrlType_STARTUP;
                    DevCtrlInquire(pParam->baseParam.pointID.driver,\
                                   pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                                   pParam->baseParam.pointID.point,(u8 *)&value,0x02);

                    loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
                    loginfo[1]=pState->ActionCtrl.CtrlSourceType;
                    memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
                    loginfo[4]=pState->ActionCtrl.CtrlMode;
                    HysSystem *pSystem = HysSystem::getInston();
                    pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_SINGLE,LOG_STATUS_START,loginfo,5);

                }
                else
                {
                    SigActionModule[moduleID].ticks_run =0;
                    pState->RunState = RunStateType_STOP;
                    pState->RunTime =0;
                }
            }
            SigActionModule[moduleID].ticks_run +=delay;
            SigActionModule[moduleID].pState->RunTime = SigActionModule[moduleID].ticks_run/100;

            if(SigActionModule[moduleID].ticks_run > (int)(pParam->actionParam.execTime ) *LOGIC_TIME_MS)
            {
                SigActionModule[moduleID].ticks_run =0;
                pState->RunState = RunStateType_STOP;
            }
            else
            {
                SigActionTiggerProcess(TriggerActionPoint_RUN,pParam,moduleID,&(pState->ActionCtrl));
                if(SigActionModule[moduleID].combAction_num >0)
                    SigActionModule[moduleID].runFin[SigActionModule[moduleID].combAction_num -1] =1;
            }
        }
        break;

        case RunStateType_SUSPEND_ACTION:
        {
            if(SigActionModule[moduleID].suspend_action_flag ==0)
            {
                ActionCtrl *pActCtrl = ActionCtrl::getInston();
                pActCtrl->AddActionInfo(ENUM_SINGLE_ACTION,moduleID+1,RunStateType_IDLE);

                /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
                u_int8_t alarmModetemp[2];
                alarmModetemp[0] = 0;
                alarmModetemp[1] = 0;
                Alarm *pAlarm = Alarm::getInston();
                pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STOP,\
                                  alarmModetemp,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_NONE);
                value = BaseCtrlType_STOP;
                DevCtrlInquire(pParam->baseParam.pointID.driver,\
                               pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                               pParam->baseParam.pointID.point,(u8 *)&value,0x02);
            }
            SigActionModule[moduleID].suspend_action_flag =1;
            SigActionModule[moduleID].suspend_flag =1;

            SigActionModule[moduleID].ticks_run +=delay;
            SigActionModule[moduleID].pState->RunTime = SigActionModule[moduleID].ticks_run/100;

            if(SigActionModule[moduleID].ticks_run > (int)(pParam->actionParam.execTime ) *LOGIC_TIME_MS)
            {
                SigActionModule[moduleID].ticks_run =0;
                pState->RunState = RunStateType_STOP;
            }
        }
        break;

        case RunStateType_SUSPEND:
        {
            SigActionModule[moduleID].suspend_flag =1;
            qDebug()<<"single suspend ,id:"<<pState->Id;;

            /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
            u_int8_t alarmModetemp[2];
            alarmModetemp[0] = 0;
            alarmModetemp[1] = 0;
            Alarm *pAlarm = Alarm::getInston();
            pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STOP,\
                              alarmModetemp,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_NONE);
            value = BaseCtrlType_STOP;
            DevCtrlInquire(pParam->baseParam.pointID.driver,\
                           pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                           pParam->baseParam.pointID.point,(u8 *)&value,0x02);

            loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
            loginfo[1]=pState->ActionCtrl.CtrlSourceType;
            memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
            loginfo[4]=pState->ActionCtrl.CtrlMode;
            HysSystem *pSystem = HysSystem::getInston();
            pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_SINGLE,LOG_STATUS_SUSPEND,loginfo,5);
        }
        break;

        case RunStateType_STOP:
        {
            qDebug()<<"single stop ,id:"<<pState->Id;;
            ActionCtrl *pActCtrl = ActionCtrl::getInston();
            pActCtrl->AddActionInfo(ENUM_SINGLE_ACTION,moduleID+1,RunStateType_IDLE);

            /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
            u_int8_t alarmModetemp[2];
            alarmModetemp[0] = 0;
            alarmModetemp[1] = 0;
            Alarm *pAlarm = Alarm::getInston();
            pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STOP,\
                              alarmModetemp,ENUM_SINGLE_ACTION,moduleID+1,ALARM_MODE_NONE);
            pState->RunState = RunStateType_IDLE;

            SigActionModule[moduleID].ticks_run =0;//按下停止按钮，清除计时

            value = DEV_POINT_CTRL_OFF;
            DevCtrlInquire(pParam->baseParam.pointID.driver,\
                           pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                           pParam->baseParam.pointID.point,(u8 *)&value,0x02);

            loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
            loginfo[1]=pState->ActionCtrl.CtrlSourceType;
            memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
            loginfo[4]=pState->ActionCtrl.CtrlMode;
            HysSystem *pSystem = HysSystem::getInston();
            pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_SINGLE,LOG_STATUS_STOP,loginfo,5);

            SigActionTiggerProcess(TriggerActionPoint_STOP,pParam,moduleID,&(pState->ActionCtrl));
            if(SigActionModule[moduleID].combAction_num >0)
                SigActionModule[moduleID].runFin[SigActionModule[moduleID].combAction_num -1] =2;
        }
        break;

        default:
            break;
    }

    if(SigActionModule[moduleID].suspend_flag ==0)
        pState->RunStateBak = runState;

    return 1;
}


u8 getSigActionFin_comb(u8 combNum,u8 actionID,u8 *finFlag)
{
    if((combNum >COMBACTION_NUM_MAX)||(actionID > SINGLEACTION_NUM_MAX))
        return 0;

    *finFlag = SigActionModule[actionID -1].runFin[combNum-1];
    return 1;
}

u8 getSigActionTriggerFlag_comb(u8 actionID,u8 *state)
{
    u8 statetmp = 0;
    statetmp = SigActionModule[actionID].trigger_flag;
    *state = statetmp;
    return 1;
}


u8 setSigActionTriggerFlag_comb(u8 actionID, u8 state)
{
    SigActionModule[actionID].trigger_flag=state;
    return 1;
}


u8 clearSigActionFin_comb(u8 combNum,u8 actionID)
{
    if((combNum >COMBACTION_NUM_MAX)||(actionID > SINGLEACTION_NUM_MAX))
        return 0;

    SigActionModule[actionID -1].runFin[combNum-1]=0;
    return 1;
}



void TiggerDelayCycle(u16 delay)
{
    u8 i,flag;

    if(TiggerTimerCnt ==0)
        return;

    for(i=0;i<TRIGGER_TIMER_MAX;i++)
    {
        if(TiggerTimerMng[i].enbale )
        {
            TiggerTimerMng[i].ticks += delay;
            if(TiggerTimerMng[i].ticks >= ((int)(TiggerTimerMng[i].pTigger->TriggerDelayTime) *LOGIC_TIME_MS))
            {
                flag=0;
                TiggerActionProcess(TiggerTimerMng[i].pTigger,1,TiggerTimerMng[i].moduleID,0,TiggerTimerMng[i].pCtrl,&flag);
                putTriggerDelayBuf(i);
            }
        }
    }
}

u8 SigActionCtrlFunc(ActionCtrlType ctrl,u8 actionID,u8 combAction_num)
{
    u8 alarmMode[2],i;
    u16 value = 0;
    SingleActionParamType *pParam;
    SingleActionStateType *pState;
    u16 stateTemp;
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();

    if(actionID >=SINGLE_MODULE_MAX)
        return 0;

    if(combAction_num !=0)
    {
        pParam = pParamMng->GetRunValueCommbSingleActParam(combAction_num,actionID+1);
        if(pParam == NULL)
            return 0;
    }
    else
    {
        pParam = pParamMng->GetRunValueSingleActParam(static_cast<u_int16_t>(SigActionModule[actionID].pParam->baseParam.ID));
        if(nullptr == pParam)
            pParam = SigActionModule[actionID].pParam;
    }

    pState = SigActionModule[actionID].pState;
    if((pState == NULL)||(pParam == NULL))
        return 0;
    if(pParam->baseParam.enable == EnableType_DISABLE)
        return 0;

    SigActionModule[actionID].combAction_num = combAction_num;
    pState->CombNum = combAction_num;

    if((pParam->actionParam.mutexActionID1 !=0)&&(pParam->actionParam.mutexActionID1 <=SINGLE_MODULE_MAX))
    {
        stateTemp = SigActionModule[pParam->actionParam.mutexActionID1-1].pState->RunState;
        if((stateTemp != RunStateType_IDLE)&&(stateTemp != RunStateType_SUSPEND)&&(stateTemp != RunStateType_STOP))
        {
            if(SigActionModule[actionID].combAction_num >0)
                SigActionModule[actionID].runFin[SigActionModule[actionID].combAction_num -1] =2;
            HysSystem *pSystem = HysSystem::getInston();
            pSystem->WriteLogApi(actionID +1,LOG_TYPE_HANDLE_SINGLE,LOG_STATUS_DISABLE,0,0);
            return 0;
        }
    }

    if((pParam->actionParam.mutexActionID2 !=0)&&(pParam->actionParam.mutexActionID2 <=SINGLE_MODULE_MAX))
    {
        stateTemp = SigActionModule[pParam->actionParam.mutexActionID2-1].pState->RunState;
        if((stateTemp != RunStateType_IDLE)&&(stateTemp != RunStateType_SUSPEND)&&(stateTemp != RunStateType_STOP))
        {
            if(SigActionModule[actionID].combAction_num >0)
                SigActionModule[actionID].runFin[SigActionModule[actionID].combAction_num -1] =2;
            HysSystem *pSystem = HysSystem::getInston();
            pSystem->WriteLogApi(actionID +1,LOG_TYPE_HANDLE_SINGLE,LOG_STATUS_DISABLE,0,0);
            return 0;
        }
    }

    if(ctrl.CtrlMode == ActionCtrl_ModeType_MANUAL)
    {
        SigActionModule[actionID].pParam = pParam;
        if(ctrl.CtrlCmd == ActionCtrl_CmdType_STARTUP)
        {
            qDebug()<<"*******single act manual start, cmd"<<actionID+1;
            ActionCtrl *pActCtrl = ActionCtrl::getInston();
            pActCtrl->AddActionInfo(ENUM_SINGLE_ACTION,actionID+1,RunStateType_RUN);

            alarmMode[0] = SINGLEACTION_ALARM_MODE_SOUND_DEFAULT;
            alarmMode[1] = SINGLEACTION_ALARM_MODE_LIGHT_DEFAULT;
            Alarm *pAlarm = Alarm::getInston();
            pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STARTUP,\
                              alarmMode,ENUM_SINGLE_ACTION,actionID+1,ALARM_MODE_2);
            pState->RunState = RunStateType_RUN_MANUAL;
            pState->RunTime = 0;
            value = BaseCtrlType_STARTUP;
            DevCtrlInquire(pParam->baseParam.pointID.driver,\
                           pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                           pParam->baseParam.pointID.point,(u_int8_t *)(&value),0x02);
            memcpy(&pState->ActionCtrl,&ctrl,sizeof(ActionCtrlType));
            //add log
            SigActionModule[actionID].ticks_alarm =0;
            SigActionModule[actionID].ticks_run =0;
            for(i=0;i<MANUALACTIONINFO_LEN;i++)
            {
                ActionCtrl *pActCtrl = ActionCtrl::getInston();
                uint ClearData = 0x00;
                if(pActCtrl->GetManualActionClearBuf(i,ClearData) == false)
                    break;

                if(static_cast<u8>(ClearData)  == (actionID+1))
                {
                    pActCtrl->SetManualActionClearBuf(i,0);
                }
            }
        }
        else if(ctrl.CtrlCmd == ActionCtrl_CmdType_STOP)
        {
            ActionCtrl *pActCtrl = ActionCtrl::getInston();
            pActCtrl->AddActionInfo(ENUM_SINGLE_ACTION,actionID+1,RunStateType_IDLE);

            alarmMode[0] = SINGLEACTION_ALARM_MODE_SOUND_DEFAULT;
            alarmMode[1] = SINGLEACTION_ALARM_MODE_LIGHT_DEFAULT;
            Alarm *pAlarm = Alarm::getInston();
            pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STOP,\
                              alarmMode,ENUM_SINGLE_ACTION,actionID+1,ALARM_MODE_NONE);
            pState->RunState = RunStateType_IDLE;
            pState->RunTime = 0;
            value = DEV_POINT_CTRL_OFF;
            DevCtrlInquire(pParam->baseParam.pointID.driver,\
                           pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                           pParam->baseParam.pointID.point,(u_int8_t *)&value,0x02);
            memcpy(&pState->ActionCtrl,&ctrl,sizeof(ActionCtrlType));
            //add log
            SigActionModule[actionID].ticks_alarm =0;
            SigActionModule[actionID].ticks_run =0;
            qDebug()<<"*******single act manual stop, cmd";
        }
    }
    else
    {
        if(pState->RunState == RunStateType_RUN_MANUAL)
            return 0;

        if(ctrl.CtrlCmd == ActionCtrl_CmdType_STARTUP && (pState->RunState == RunStateType_STOP ||pState->RunState == RunStateType_RUN))//jhy 20220617
            return 0;




        SigActionAutoCtrlFunc(ctrl,pParam,pState,actionID);
        SigActionModule[actionID].pParam = pParam;
    }
    return 1;
}

u8 SigActionCtrlFunc_stop(ActionCtrlType ctrl,u8 actionID,u8 combAction_num)
{
    u8 value,alarmMode[2],i;
    SingleActionParamType *pParam;
    SingleActionStateType *pState;
    u16 stateTemp;
    u8 loginfo[6];
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();

    if(actionID >=SINGLE_MODULE_MAX)
        return 0;

    if(combAction_num !=0)
    {
        pParam = pParamMng->GetRunValueCommbSingleActParam(combAction_num,actionID+1);
        if(NULL == pParam)
            return 0;
    }
    else
    {
        pParam = pParamMng->GetRunValueSingleActParam(SigActionModule[actionID].pParam->baseParam.ID);
        if(NULL == pParam)
            pParam = SigActionModule[actionID].pParam;
    }

    pState = SigActionModule[actionID].pState;
    if((pState == NULL)||(pParam == NULL))
        return 0;
    if(pParam->baseParam.enable == EnableType_DISABLE)
        return 0;

    SigActionModule[actionID].combAction_num = combAction_num;
    pState->CombNum = combAction_num;

    if(pState->RunState == RunStateType_RUN_MANUAL)
        return 0;

    if(pState->RunState == RunStateType_IDLE || pState->RunState == RunStateType_STOP)
        return 0;

    deleteTriggerDelayBuf(actionID,SigActionModule[actionID].combAction_num);
    SigActionModule[actionID].pParam = pParam;

    ActionCtrl *pActCtrl = ActionCtrl::getInston();
    pActCtrl->AddActionInfo(ENUM_SINGLE_ACTION,actionID+1,RunStateType_IDLE);

    /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
    u_int8_t alarmModetemp[2];
    alarmModetemp[0] = 0;
    alarmModetemp[1] = 0;
    Alarm *pAlarm = Alarm::getInston();
    pAlarm->CtrlAlarm((AlarmType)(pParam->actionParam.alarmType ),SINGLEACTION_ALARM_ALWAY,BaseCtrlType_STOP,\
                      alarmModetemp,ENUM_SINGLE_ACTION,actionID+1,ALARM_MODE_NONE);
    pState->RunState = RunStateType_IDLE;
    pState->RunTime = 0;
    value = BaseCtrlType_STOP;
    DevCtrlInquire(pParam->baseParam.pointID.driver,\
                   pParam->baseParam.pointID.device,pParam->baseParam.pointID.subDevice,\
                   pParam->baseParam.pointID.point,&value,0x02);

    loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
    loginfo[1]=pState->ActionCtrl.CtrlSourceType;
    memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
    loginfo[4]=pState->ActionCtrl.CtrlMode;
    HysSystem *pSystem = HysSystem::getInston();
    pSystem->WriteLogApi(actionID +1,LOG_TYPE_HANDLE_SINGLE,LOG_STATUS_STOP,loginfo,5);

    SigActionTiggerProcess(TriggerActionPoint_STOP,pParam,actionID,&(pState->ActionCtrl));
    if(SigActionModule[actionID].combAction_num >0)
        SigActionModule[actionID].runFin[SigActionModule[actionID].combAction_num -1] =2;

    serverReportAction_single(actionID);

    return 1;
}

u8 SigActionAllStop(u8 moduleID)
{
    SingleActionParamType *pParam = SigActionModule[moduleID].pParam;
    SingleActionStateType *pState = SigActionModule[moduleID].pState;

    if((pState == NULL)||(pParam == NULL))
        return 0;
    if(pParam->baseParam.enable == EnableType_DISABLE)
        return 0;

    if(pState->RunState != RunStateType_IDLE)
        pState->RunState = RunStateType_STOP;
    return 1;
}


u8 AutoSigActionAllStop(u8 moduleID)
{
    SingleActionParamType *pParam = SigActionModule[moduleID].pParam;
    SingleActionStateType *pState = SigActionModule[moduleID].pState;

    if((pState == nullptr)||(pParam == nullptr))
        return 0;
    if(pParam->baseParam.enable == EnableType_DISABLE)
        return 0;


    if(pState->RunState == RunStateType_RUN_MANUAL)
        return 1;
    if(pState->RunState != RunStateType_IDLE)
        pState->RunState = RunStateType_STOP;
    return 1;
}


void SigActionRunStateJudge(ActionCtrlType ctrl,u8 moduleID)   //jhy 20220725  judge singleAction is run and restart this
{
    u8 res;
    Action_RunStateType type;
    SingleActionParamType *pParam = SigActionModule[moduleID].pParam;
    SingleActionStateType *pState = SigActionModule[moduleID].pState;
    if(ctrl.CtrlCmd == ActionCtrl_CmdType_STARTUP || ctrl.CtrlCmd == ActionCtrl_CmdType_WAITSTARTUP)
    {
        res = getSingleActionState(moduleID,&type);
        if(res == 0)
            return;
        if(type == RunStateType_RUN)
        {
            if((pState == NULL)||(pParam == NULL))
                return;
            if(pParam->baseParam.enable == EnableType_DISABLE)
                return;

            if(pState->RunState != RunStateType_IDLE)
            {
                pState->RunTime = 0;
                SigActionModule[moduleID].ticks_run = 0;
            }
        }
    }
}

u8 SigActionPerformJudge(ActionCtrlType ctrl,u8 actionID,u16 *pmutexAction)
{
    SingleActionParamType *pParam;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();

    if(actionID > SINGLE_MODULE_MAX)
        return 0;

    pParam = pParamMng->GetRunValueSingleActParam(static_cast<u_int16_t>(SigActionModule[actionID].pParam->baseParam.ID));
    if(nullptr == pParam)
        pParam = SigActionModule[actionID].pParam;

    if(pParam == nullptr)
        return 0;

    if(pParam->baseParam.enable == EnableType_DISABLE)
        return 0;

    if((pParam->actionParam.controlMode ==ControlModeType_LocalRemote)||
      ((ctrl.CtrlSourceType == ActionCtrl_SourceType_CONTROLLER) &&(ctrl.CtrlSourceAddr !=ScSelfAddr ))||
      ((pParam->actionParam.controlMode ==ControlModeType_Local)&&(ctrl.CtrlSourceType == ActionCtrl_SourceType_CONTROLLER)
        &&(ctrl.CtrlSourceAddr ==ScSelfAddr ))||
       ((pParam->actionParam.controlMode ==ControlModeType_Remote)&&(ctrl.CtrlSourceType != ActionCtrl_SourceType_CONTROLLER)))
    {
        *pmutexAction = pParam->actionParam.mutexActionID1;
        *(pmutexAction+1) = pParam->actionParam.mutexActionID2;
        return 1;
    }
    return 0;
}
