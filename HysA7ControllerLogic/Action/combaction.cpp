#include "combaction.h"
#include "singleaction.h"
#include "alarm.h"
#include <QDebug>
#include "libhysa7parammng.h"
#include "./HysSystem/hyssystem.h"
#include "actionctrl.h"

/******/
CombActionModuleType 	CombActionModule[COMBACTION_NUM_MAX];

void CombActionModuleInit(void)
{
    memset(CombActionModule,0,sizeof(CombActionModule));
}

void CombActionModuleLoad(u8 moduleID,CombActionParamType *pParam,CombActionStateType *pState)
{
    if((pState == NULL)||(pParam == NULL)||(moduleID >=COMBACTION_NUM_MAX))
        return;

    CombActionModule[moduleID].pParam = pParam;
    CombActionModule[moduleID].pState = pState;
}

static u8 CombActionAutoCtrlFunc(ActionCtrlType ctrl,CombActionParamType *pParam,CombActionStateType *pState)
{
    u8 i,id;

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
            if(pState->RunState == RunStateType_IDLE)
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

            for(i=0; i<COMBACTION_SINGLE_NUM;i++)
            {
                if(pParam->actionParam[i].SigParam.baseParam.enable == EnableType_ENABLE)
                {
                    ctrl.CtrlCmd = ActionCtrl_CmdType_RESUME;
                    ctrl.CtrlMode = pState->ActionCtrl.CtrlMode; //ActionCtrl_ModeType_AUTO;
                    ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType; //ActionCtrl_SourceType_CONTROLLER;
                    ctrl.CtrlSourceAddr = pState->ActionCtrl.CtrlSourceAddr;
                    ctrl.UserID =0;

                    id = pParam->actionParam[i].SigParam.baseParam.ID;
                    if(id ==0)
                    {
                        continue;
                    }
                    SigActionCtrlFunc(ctrl,id-1,pParam->baseParam.ID);
                }
            }
            //add log
        }
        break;

        default:
            return 0;
    }

    memcpy(&pState->ActionCtrl,&ctrl,sizeof(ActionCtrlType));
    return 1;
}

u8 getCombActionFin(u8 combNum,u8 actionID,u8 *finFlag)
{
    if((combNum >COMBACTION_NUM_MAX)||(actionID > SINGLEACTION_NUM_MAX))
        return 0;

    *finFlag = CombActionModule[combNum -1].runFin[actionID-1];
    return 1;
}

static u8 serverReportAction_comb(u8 moduleID)
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
    temp[1] = ENUM_COMBINATION_ACTION;
    temp[2] = CombActionModule[moduleID].pParam->baseParam.ID;
    length =sizeof(CombActionStateType);
    memcpy(&temp[3],&length,2);
    memcpy(&temp[5],CombActionModule[moduleID].pState,length);

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    serverID = pParamMng->GetRunValueGlobalSysServerAddr();
    UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                   temp,5+length);
    return 1;
}

u8 checkSigActionTriggerFlag_comb(u8 moduleID, u8 actionID)
{
//    CombActionParamType *pParam = CombActionModule[moduleID].pParam;

//    if(moduleID >= COMBACTION_NUM_MAX)
//        return 0;
//    if(actionID >= COMBACTION_SINGLE_NUM)
//        return 0;
//    if(pParam == NULL)
//        return 0;

//        qDebug()<<("checkSigActionTriggerFlag_comb,")<<moduleID<<actionID;
//         for(int j = 0; j < 10;j++)
//         {
//            state = 0;
//         }
//    for(number = 0;number<SINGLE_ACTION_TRIGGER_MAX;number++)
//    {
//        qDebug()<<("checkSigActionTriggerFlag_comb,111+1+111")<<number<<moduleID<<actionID;
//        for(list = 0;list<TRIGGER_ACTION_MAX;list++)
//        {
//            qDebug()<<("ccheckSigActionTriggerFlag_comb,222+1+222")<<list<<moduleID<<actionID;;
//            if(pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionID == 0)
//                continue;
//            if(pParam->actionParam[actionID].SigParam.baseParam.enable == EnableType_DISABLE)
//                continue;
//            if(pParam->actionParam[actionID].SigParam.baseParam.ID<pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].ExtTriggerFuncID)
//                continue;
//            if(pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionCmd == ActionCtrl_CmdType_STARTUP ||\
//                pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionCmd == ActionCtrl_CmdType_WAITSTARTUP)
//            {
//                qDebug()<<("checkSigActionTriggerFlag_comb,333+1+333")<<list<<moduleID<<actionID;;
//                if(pParam->actionParam[actionID].SigParam.tiggerParam[number].cond.TriggerEnable == EnableType_ENABLE&&\
//                    pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionEnable == EnableType_ENABLE&&\
//                    pParam->actionParam[actionID].SigParam.baseParam.enable == EnableType_ENABLE)
//                {
//                    qDebug()<<("checkSigActionTriggerFlag_comb,444+1+444")<<list;
//                   if(pParam->actionParam[actionID].zeroDelayEnable == EnableType_ENABLE)
//                    {
//                       qDebug()<<("checkSigActionTriggerFlag_comb,555+1+555")<<list;
//                        setSigActionTriggerFlag_comb(pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionID-1,1);
//                        qDebug()<<("checkSigActionTriggerFlag_comb,555+1+555,end")<<list;
//                    }
//                    else
//                    {
//                        qDebug()<<("checkSigActionTriggerFlag_comb,666+1+666")<<list<<pParam->actionParam[actionID].SigParam.baseParam.ID;
//                        getSigActionTriggerFlag_comb(pParam->actionParam[actionID].SigParam.baseParam.ID-1,&state);
//                        qDebug()<<("checkSigActionTriggerFlag_comb,777+1+777")<<list;
//                        if(state == 1)
//                        {
//                            qDebug()<<("checkSigActionTriggerFlag_comb,888+1+888")<<list;
//                            setSigActionTriggerFlag_comb(pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionID-1,1);
//                            qDebug()<<("checkSigActionTriggerFlag_comb,999+1+999")<<list;
//                        }
//                    }
//                }
//            }
//        }
//    }
}

u8 CombActionIdleProcess(u16 delay,u8 moduleID)
{
    return 1;
}

u8 CombActionAlaramProcess(u16 delay,u8 moduleID)
{
    CombActionParamType *pParam = CombActionModule[moduleID].pParam;
    CombActionStateType *pState = CombActionModule[moduleID].pState;

    if(pState->RunStateBak != pState->RunState)
    {
        qDebug("comb %d alarm",pState->Id);
        Alarm *pAlarm = Alarm::getInston();
        /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
        u_int8_t alarmModetemp[2];
        alarmModetemp[0] = 0;
        alarmModetemp[1] = 0;
        pAlarm->CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime,BaseCtrlType_STARTUP,\
                          alarmModetemp,ENUM_COMBINATION_ACTION,moduleID+1,ALARM_MODE_1);//alarmmodetemp处原为数值0
    }

    CombActionModule[moduleID].ticks_alarm +=delay;
    if(CombActionModule[moduleID].ticks_alarm > (int)(pParam->baseParam.alarmTime) *LOGIC_TIME_MS)
    {
        CombActionModule[moduleID].ticks_alarm =0;
        pState->RunState = RunStateType_STARTUP;
        CombActionModule[moduleID].cycle_cnt=0;
        Alarm *pAlarm = Alarm::getInston();
        /***新修改的地方alarmModetemp.23.05.26，alarmmodetemp处原为数值0***/
        u_int8_t alarmModetemp[2];
        alarmModetemp[0] = 0;
        alarmModetemp[1] = 0;
        pAlarm->CtrlAlarm((AlarmType)(pParam->baseParam.alarmType ),pParam->baseParam.alarmTime,BaseCtrlType_STOP,\
                          alarmModetemp,ENUM_COMBINATION_ACTION,moduleID+1,ALARM_MODE_NONE);
    }
    return 0;
}

u8 CombActionStartupProcess(u16 delay,u8 moduleID)
{
    CombActionParamType *pParam = CombActionModule[moduleID].pParam;
    CombActionStateType *pState = CombActionModule[moduleID].pState;
    u8 id = 0;
    u8 idtemp = 0;
    ActionCtrlType ctrl;
    u8 loginfo[6];

    CombActionModule[moduleID].cycle_cnt=0;
    ctrl.CtrlCmd = ActionCtrl_CmdType_WAITSTARTUP;
    ctrl.CtrlMode = pState->ActionCtrl.CtrlMode; //ActionCtrl_ModeType_AUTO;
    ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType; //ActionCtrl_SourceType_CONTROLLER;
    ctrl.CtrlSourceAddr = pState->ActionCtrl.CtrlSourceAddr;
    ctrl.UserID =0;

    id = pParam->actionParam[0].SigParam.baseParam.ID;
    if(id ==0)
    {
        pState->RunState = RunStateType_IDLE;
        return -1;
    }
    qDebug("comb %d startup",pState->Id);
    ActionCtrl *pActCtrl = ActionCtrl::getInston();
    pActCtrl->AddActionInfo(ENUM_COMBINATION_ACTION,moduleID+1,RunStateType_RUN);

    /*********/
    for(int i=0; i<COMBACTION_SINGLE_NUM;i++)
    {
        idtemp = pParam->actionParam[i].SigParam.baseParam.ID;
        if(idtemp ==0)
        {
            continue;
        }
        setSigActionTriggerFlag_comb(pParam->actionParam[i].SigParam.baseParam.ID-1,0);
        if(pParam->actionParam[i].zeroDelayEnable == EnableType_ENABLE &&\
            pParam->actionParam[i].SigParam.baseParam.enable == EnableType_ENABLE)
        {
            setSigActionTriggerFlag_comb(pParam->actionParam[i].SigParam.baseParam.ID-1,1);
        }
    }

    /*********/
    for(int actionID=0; actionID<COMBACTION_SINGLE_NUM;actionID++)//查找组合动作在中，所有包含的单动作
    {
        idtemp = pParam->actionParam[actionID].SigParam.baseParam.ID;
        if(idtemp ==0)
        {
            continue;
        }
        clearSigActionFin_comb(pParam->baseParam.ID,idtemp);
        for(int number = 0;number<SINGLE_ACTION_TRIGGER_MAX;number++)
        {
            for(int list = 0;list<TRIGGER_ACTION_MAX;list++)
            {
                if(pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionID == 0)
                    continue;
                if(pParam->actionParam[actionID].SigParam.baseParam.enable == EnableType_DISABLE)
                    continue;
                if(pParam->actionParam[actionID].SigParam.baseParam.ID<pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].ExtTriggerFuncID)
                    continue;
                if(pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionCmd == ActionCtrl_CmdType_STARTUP ||\
                    pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionCmd == ActionCtrl_CmdType_WAITSTARTUP)
                {
                    if(pParam->actionParam[actionID].SigParam.tiggerParam[number].cond.TriggerEnable == EnableType_ENABLE&&\
                        pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionEnable == EnableType_ENABLE&&\
                        pParam->actionParam[actionID].SigParam.baseParam.enable == EnableType_ENABLE)
                    {
                       if(pParam->actionParam[actionID].zeroDelayEnable == EnableType_ENABLE)
                        {
                            setSigActionTriggerFlag_comb(pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionID-1,1);
                        }
                        else
                        {
                            u8 state = 0 ;
                            getSigActionTriggerFlag_comb(pParam->actionParam[actionID].SigParam.baseParam.ID-1,&state);
                            if(state == 1)
                            {
                                setSigActionTriggerFlag_comb(pParam->actionParam[actionID].SigParam.tiggerParam[number].action[list].TriggerActionID-1,1);
                            }
                        }
                    }
                }
            }
        }
    }

    /*********/
    SigActionModuleLoad(id-1,&(pParam->actionParam[0].SigParam),0);
    SigActionCtrlFunc(ctrl,id-1,pParam->baseParam.ID);
    pState->RunState = RunStateType_RUN;
    memset(CombActionModule[moduleID].ticks_delay,0,sizeof(CombActionModule[moduleID].ticks_delay));
    memset(CombActionModule[moduleID].runFin,0,sizeof(CombActionModule[moduleID].runFin));
    memset(CombActionModule[moduleID].delayFin,0,sizeof(CombActionModule[moduleID].delayFin));
    CombActionModule[moduleID].cycle_start = pParam->baseParam.cycleStart1;
    CombActionModule[moduleID].cycle_end = pParam->baseParam.cycleEnd1;
    loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
    loginfo[1]=pState->ActionCtrl.CtrlSourceType;
    memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
    loginfo[4]=pState->ActionCtrl.CtrlMode;

    HysSystem *pSystem = HysSystem::getInston();
    pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_COMB,LOG_STATUS_START,loginfo,5);

    return 1;
}

u8 CombActionRunProcess(u16 delay,u8 moduleID)
{
    CombActionParamType *pParam = CombActionModule[moduleID].pParam;
    CombActionStateType *pState = CombActionModule[moduleID].pState;
    u8 id = 0;
    u8 idtemp = 0;
    ActionCtrlType ctrl;
    Action_RunStateType typetmp;
    u8 finFlag,finFlagTemp,finFlagTotal,cycleStepFlag,trggerFlag,triggerState;

    for(int i=0; i<COMBACTION_SINGLE_NUM;i++)
    {
        if((pParam->actionParam[i].zeroDelayEnable)&&(CombActionModule[moduleID].delayFin[i]==0))
        {
            CombActionModule[moduleID].ticks_delay[i] +=delay;
            if(CombActionModule[moduleID].ticks_delay[i] >= (int)(pParam->actionParam[i].zeroDelayTime)*LOGIC_TIME_MS)
            {
                CombActionModule[moduleID].ticks_delay[i]=0;
                CombActionModule[moduleID].delayFin[i]=1;

                ctrl.CtrlCmd = ActionCtrl_CmdType_WAITSTARTUP;
                ctrl.CtrlMode = pState->ActionCtrl.CtrlMode; //ActionCtrl_ModeType_AUTO;
                ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType; //ActionCtrl_SourceType_CONTROLLER;
                ctrl.CtrlSourceAddr = pState->ActionCtrl.CtrlSourceAddr;
                ctrl.UserID =0;

                id = pParam->actionParam[i].SigParam.baseParam.ID;
                if(id ==0)
                {
                    break;
                }
                SigActionModuleLoad(id-1,&(pParam->actionParam[i].SigParam),0);
                SigActionCtrlFunc(ctrl,id-1,pParam->baseParam.ID);
            }
        }
    }

    finFlag =0;
    finFlagTotal=0;
    cycleStepFlag=0;
    if((CombActionModule[moduleID].cycle_end >= CombActionModule[moduleID].cycle_start)
       &&(pParam->baseParam.cycleCnt !=0)&&(CombActionModule[moduleID].cycle_start!=0))
    {
        trggerFlag = 0;
        for(int i=CombActionModule[moduleID].cycle_start; i<=CombActionModule[moduleID].cycle_end;i++)
        {
            id = pParam->actionParam[i-1].SigParam.baseParam.ID;
            if(id ==0)
                continue;

            getSigActionTriggerFlag_comb(id-1,&triggerState);
            if(triggerState == 1)
                trggerFlag++;
            if(getSingleActionState(id-1,&typetmp))
            {
                if(typetmp == RunStateType_IDLE)
                {
                    getSigActionFin_comb(pParam->baseParam.ID,id,&finFlagTemp);
                    if(finFlagTemp>1 && triggerState == 1)
                    {
                        finFlag++;
                    }
                }
            }
        }

        if(finFlag == trggerFlag)  //第一轮循环结束
        {
            CombActionModule[moduleID].cycle_cnt ++;
            if(CombActionModule[moduleID].cycle_cnt >= pParam->baseParam.cycleCnt)
                cycleStepFlag=1;
            else
            {
                for(int i=CombActionModule[moduleID].cycle_start; i<=CombActionModule[moduleID].cycle_end;i++)
                {
                    id = pParam->actionParam[i-1].SigParam.baseParam.ID;
                    if(id ==0)
                    {
                        continue;
                    }
                    clearSigActionFin_comb(pParam->baseParam.ID,id);
                    CombActionModule[moduleID].delayFin[i-1]=0;
                }

                ctrl.CtrlCmd = ActionCtrl_CmdType_WAITSTARTUP;
                ctrl.CtrlMode = pState->ActionCtrl.CtrlMode; //ActionCtrl_ModeType_AUTO;
                ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType; //ActionCtrl_SourceType_CONTROLLER;
                ctrl.CtrlSourceAddr = pState->ActionCtrl.CtrlSourceAddr;
                ctrl.UserID =0;

                id = pParam->actionParam[CombActionModule[moduleID].cycle_start-1].SigParam.baseParam.ID;
                if(id ==0)
                {
                    pState->RunState = RunStateType_IDLE;
                    return -1;
                }
                SigActionModuleLoad(id-1,&(pParam->actionParam[CombActionModule[moduleID].cycle_start-1].SigParam),0);
                SigActionCtrlFunc(ctrl,id-1,pParam->baseParam.ID);
            }

            if(cycleStepFlag)  //第一轮循环结束
            {
                ctrl.CtrlCmd = ActionCtrl_CmdType_WAITSTARTUP;
                ctrl.CtrlMode = pState->ActionCtrl.CtrlMode; //ActionCtrl_ModeType_AUTO;
                ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType; //ActionCtrl_SourceType_CONTROLLER;
                ctrl.CtrlSourceAddr = pState->ActionCtrl.CtrlSourceAddr;
                ctrl.UserID =0;

                idtemp =CombActionModule[moduleID].cycle_end;
                if((pParam->baseParam.cycleEnd2 >= pParam->baseParam.cycleStart2)
                    &&(pParam->baseParam.cycleCnt !=0)&&(pParam->baseParam.cycleStart2!=0)) //存在第二轮循环
                {
                    CombActionModule[moduleID].cycle_start = pParam->baseParam.cycleStart2;
                    CombActionModule[moduleID].cycle_end = pParam->baseParam.cycleEnd2;
                    CombActionModule[moduleID].cycle_cnt = 0;
                    cycleStepFlag = 0;
                }
                else
                {
                    CombActionModule[moduleID].cycle_start = 0;
                    CombActionModule[moduleID].cycle_end = 0;
                }

                if(idtemp >= COMBACTION_SINGLE_NUM)
                {
                    pState->RunState = RunStateType_STOP;
                    return 0;
                }

                id = pParam->actionParam[idtemp].SigParam.baseParam.ID;
                if(id ==0)
                {
                    pState->RunState = RunStateType_STOP;
                    return 0;
                }
                SigActionModuleLoad(id-1,&(pParam->actionParam[idtemp].SigParam),0);
                SigActionCtrlFunc(ctrl,id-1,pParam->baseParam.ID);
            }
        }
    }
    else  //无循环或循环从第一个开始整体循环
    {
        for(int i=0; i<COMBACTION_SINGLE_NUM;i++)
        {
            finFlag = finFlagTotal;
            id = pParam->actionParam[i].SigParam.baseParam.ID;
            if(id ==0)
                continue;
            if(getSingleActionState(id-1,&typetmp))
            {
                if(typetmp == RunStateType_IDLE)
                {
                    continue;
                }
                else
                {
                    finFlag = finFlagTotal+1;
                    break;
                }
            }
        }

        if(finFlag == finFlagTotal)
        {
            CombActionModule[moduleID].cycle_cnt ++;
            if(CombActionModule[moduleID].cycle_cnt >= pParam->baseParam.cycleCnt)
                pState->RunState = RunStateType_STOP;
            else
            {
                pState->RunState = RunStateType_STARTUP;
            }

            for(int i=0; i<COMBACTION_SINGLE_NUM;i++)
            {
                id = pParam->actionParam[i].SigParam.baseParam.ID;
                if(id ==0)
                {
                    continue;
                }
                clearSigActionFin_comb(pParam->baseParam.ID,id);
                CombActionModule[moduleID].delayFin[i]=0;
            }
        }
    }

    for(int i=0; i<COMBACTION_SINGLE_NUM;i++)
    {
        id = pParam->actionParam[i].SigParam.baseParam.ID;
        if(id ==0)
            continue;

        getSigActionFin_comb(pParam->baseParam.ID,id-1,&CombActionModule[moduleID].runFin[i]);
    }

    return 1;
}

u8 CombActionSuspendProcess(u16 delay,u8 moduleID)
{
    CombActionParamType *pParam = CombActionModule[moduleID].pParam;
    CombActionStateType *pState = CombActionModule[moduleID].pState;
    u8 loginfo[6];
    u8 id = 0;
    ActionCtrlType ctrl;

    qDebug("comb %d suspend",pState->Id);
    CombActionModule[moduleID].suspend_flag =1;

    for(int i=0; i<COMBACTION_SINGLE_NUM;i++)
    {
        if(pParam->actionParam[i].SigParam.baseParam.enable == EnableType_ENABLE)
        {
            ctrl.CtrlCmd = ActionCtrl_CmdType_SUSPEND;
            ctrl.CtrlMode = pState->ActionCtrl.CtrlMode; //ActionCtrl_ModeType_AUTO;
            ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType; //ActionCtrl_SourceType_CONTROLLER;
            ctrl.CtrlSourceAddr = pState->ActionCtrl.CtrlSourceAddr;
            ctrl.UserID =0;

            id = pParam->actionParam[i].SigParam.baseParam.ID;
            if(id ==0)
            {
                continue;
            }
            SigActionModuleLoad(id-1,&(pParam->actionParam[i].SigParam),0);
            SigActionCtrlFunc(ctrl,id-1,pParam->baseParam.ID);
        }
    }

    loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
    loginfo[1]=pState->ActionCtrl.CtrlSourceType;
    memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
    loginfo[4]=pState->ActionCtrl.CtrlMode;
    HysSystem *pSystem = HysSystem::getInston();
    pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_COMB,LOG_STATUS_SUSPEND,loginfo,5);

    return 1;
}

u8 CombActionStopProcess(u16 delay,u8 moduleID)
{
    CombActionParamType *pParam = CombActionModule[moduleID].pParam;
    CombActionStateType *pState = CombActionModule[moduleID].pState;
    u8 loginfo[6];
    u8 id = 0;
    ActionCtrlType ctrl;

    pState->RunState = RunStateType_IDLE;
    qDebug("comb %d stop",pState->Id);
    ActionCtrl *pActCtrl = ActionCtrl::getInston();
    pActCtrl->AddActionInfo(ENUM_COMBINATION_ACTION,moduleID+1,RunStateType_IDLE);

    for(int i=0; i<COMBACTION_SINGLE_NUM;i++)
    {
        if(pParam->actionParam[i].SigParam.baseParam.enable == EnableType_ENABLE)
        {
            ctrl.CtrlCmd = ActionCtrl_CmdType_STOP;
            ctrl.CtrlMode = pState->ActionCtrl.CtrlMode; //ActionCtrl_ModeType_AUTO;
            ctrl.CtrlSourceType = pState->ActionCtrl.CtrlSourceType; //ActionCtrl_SourceType_CONTROLLER;
            ctrl.CtrlSourceAddr = pState->ActionCtrl.CtrlSourceAddr;
            ctrl.UserID =0;

            id = pParam->actionParam[i].SigParam.baseParam.ID;
            if(id ==0)
            {
                continue;
            }

            clearSigActionFin_comb(pParam->baseParam.ID,id);
            SigActionModuleLoad(id-1,&(pParam->actionParam[i].SigParam),0);
            SigActionCtrlFunc_stop(ctrl,id-1,pParam->baseParam.ID);
        }
    }

    loginfo[0]=pState->ActionCtrl.CtrlSourceAddr;
    loginfo[1]=pState->ActionCtrl.CtrlSourceType;
    memcpy(&loginfo[2],&pState->ActionCtrl.UserID,2);
    loginfo[4]=pState->ActionCtrl.CtrlMode;
    HysSystem *pSystem = HysSystem::getInston();
    pSystem->WriteLogApi(moduleID +1,LOG_TYPE_HANDLE_COMB,LOG_STATUS_STOP,loginfo,5);

    return 1;
}

u8 CombActionStepProcess(u16 delay,u8 moduleID)
{
    CombActionParamType *pParam = CombActionModule[moduleID].pParam;
    CombActionStateType *pState = CombActionModule[moduleID].pState;
    u16 runState;

    if((pState == nullptr)||(pParam == nullptr))
        return 0;
    if(pParam->baseParam.enable == static_cast<u_int16_t>(EnableType_DISABLE) && pState->RunState == static_cast<u_int16_t>(RunStateType_IDLE))
        return 0;

    if((CombActionModule[moduleID].suspend_flag)&&(pState->RunState==RunStateType_SUSPEND))
        return 0;
    else
        CombActionModule[moduleID].suspend_flag =0;

    runState = pState->RunState;
    if(pState->RunStateBak != pState->RunState)
    {
        serverReportAction_comb(moduleID);
    }

    switch(pState->RunState)
    {
        case RunStateType_IDLE:
            CombActionIdleProcess(delay,moduleID);
            break;

        case RunStateType_ALARM:
            CombActionAlaramProcess(delay,moduleID);
            break;

        case RunStateType_STARTUP:
            CombActionStartupProcess(delay,moduleID);
            break;

        case RunStateType_RUN:
            CombActionRunProcess(delay,moduleID);
            break;

        case RunStateType_SUSPEND:
            CombActionSuspendProcess(delay,moduleID);
            break;

        case RunStateType_STOP:
            CombActionStopProcess(delay,moduleID);
        break;

        default:
            break;
    }
    if(CombActionModule[moduleID].suspend_flag ==0)
        pState->RunStateBak = runState;

    return 1;
}

u8 CombActionCtrlFunc(ActionCtrlType ctrl,u8 actionID)
{
    CombActionParamType *pParam = CombActionModule[actionID].pParam;
    CombActionStateType *pState = CombActionModule[actionID].pState;

    if((pState == NULL)||(pParam == NULL))
        return 0;
    if(pParam->baseParam.enable == EnableType_DISABLE)
        return 0;

    if(ctrl.CtrlMode == ActionCtrl_ModeType_MANUAL)
        return 0;

    CombActionAutoCtrlFunc(ctrl,pParam,pState);
    return 1;
}

u8 CombActionAllStop(u8 actionID)
{
    CombActionParamType *pParam = CombActionModule[actionID].pParam;
    CombActionStateType *pState = CombActionModule[actionID].pState;

    if((pState == nullptr)||(pParam == nullptr))
        return 0;
    if(pParam->baseParam.enable == EnableType_DISABLE)
        return 0;

    if(pState->RunState != RunStateType_IDLE)
        pState->RunState = RunStateType_STOP;
    return 1;
}

u8 CombActionPerformJudge(ActionCtrlType ctrl,u8 actionID)
{
    CombActionParamType *pParam = CombActionModule[actionID].pParam;

    if(pParam == NULL)
        return 0;
    if(pParam->baseParam.enable == EnableType_DISABLE)
        return 0;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t ScSelfAddr = pParamMng->GetRunValueGlobalCurrentSupportID();

    if((pParam->baseParam.controlMode ==ControlModeType_LocalRemote)||
      ((ctrl.CtrlSourceType == ActionCtrl_SourceType_CONTROLLER) &&(ctrl.CtrlSourceAddr !=ScSelfAddr ))||
      ((pParam->baseParam.controlMode ==ControlModeType_Local)&&(ctrl.CtrlSourceType == ActionCtrl_SourceType_CONTROLLER)
        &&(ctrl.CtrlSourceAddr ==ScSelfAddr ))||
       ((pParam->baseParam.controlMode ==ControlModeType_Remote)&&(ctrl.CtrlSourceType != ActionCtrl_SourceType_CONTROLLER)))
    {
        return 1;
    }
    return 0;
}


u8 checkCombActionIsMutexSingleAction(u8 moduleID1,u8 moduleID2)
{
    u8 i,j;
    u8 res = 0;
    CombActionParamType *pParam1 = CombActionModule[moduleID1].pParam;
    CombActionParamType *pParam2 = CombActionModule[moduleID2].pParam;

    for(i = 0;i<COMBACTION_SINGLE_NUM;i++)
    {
        for(j = 0;j<COMBACTION_SINGLE_NUM;j++)
        {
            if(pParam1->actionParam[i].SigParam.baseParam.ID == pParam2->actionParam[j].SigParam.actionParam.mutexActionID1 ||
                pParam1->actionParam[i].SigParam.baseParam.ID == pParam2->actionParam[j].SigParam.actionParam.mutexActionID2)
            {
                res = 1;
                break;
            }
        }
    }
}


u8 checkCombActionPriorityState(u8 moduleID)
{
    u8 i;
    u8 res = 1;
    CombActionParamType *pParam = CombActionModule[moduleID].pParam;
    if(pParam->baseParam.actionPriority == ActionPriority_Low)
    {
        for(i = 0;i < COMBACTION_NUM_MAX; i++)
        {
            if(CombActionModule[i].pState->RunState == RunStateType_IDLE || CombActionModule[i].pState->RunState == RunStateType_STOP)
                continue;
            else
            {
                if(checkCombActionIsMutexSingleAction(i,moduleID))
                {
                    res = 0;
                    break;
                }
                else
                {
                    ;
                }
            }
        }
    }
    else if(pParam->baseParam.actionPriority == ActionPriority_Middle)
    {
        for(i = 0;i < COMBACTION_NUM_MAX; i++)
        {
            if(CombActionModule[i].pState == NULL)
                break;
            else if(CombActionModule[i].pState->RunState == RunStateType_IDLE || CombActionModule[i].pState->RunState == RunStateType_STOP)
                continue;
            else
            {
                if(CombActionModule[i].pParam->baseParam.actionPriority == ActionPriority_Low)
                {
                    if(checkCombActionIsMutexSingleAction(i,moduleID))
                        CombActionAllStop(i);
                }
                else if(CombActionModule[i].pParam->baseParam.actionPriority == ActionPriority_Middle)
                {
                    ;
                }
                break;
            }
        }
    }
    return res;
}
