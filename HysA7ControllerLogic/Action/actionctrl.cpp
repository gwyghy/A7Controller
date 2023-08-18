#include <unistd.h>
#include "actionctrl.h"
#include "singleaction.h"
#include "part.h"
#include "HysSystem/hyssystem.h"

/****/
#define     ACTCTRL_PROC_POWERON_TIME_SET       100

/****/
enum
{
    ACTCTRL_PROC_TIMER_MIN = 10,
    ACTCTRL_PROC_TIMER_MAX	= 1000,
    ACTCTRL_PROC_TIMER_DEFAULT = 100,
    ACTCTRL_PROC_TIMER_CLOSED = 0xFFFFFFFF
};

/****/
ActionCtrl *ActionCtrl::that = nullptr;


/****动作控制****/
ActionCtrl::ActionCtrl()
{
    m_mutex = new QMutex();
    m_ManuClear_Locker = new QReadWriteLock();
    memset(&m_sem,0x00,sizeof (semt));
    SysCtrlBufInit();
    ActionDataInit();
}

ActionCtrl::~ActionCtrl()
{
   delete m_mutex;
   delete m_ManuClear_Locker;
}

ActionCtrl *ActionCtrl::getInston()
{
    if( that == NULL)
    {
        that = new ActionCtrl();
    }
    return that;
}

int ActionCtrl::Init(void)
{
    qDebug()<<"*************ActionCtrl,Init,1,ing**********";
    manualCtrlAddr = 0;//手动控制的架号

    Alarm *pAlarm = Alarm::getInston();
    pAlarm->AlarmModuleInit();

    HysSystem *pSystem = HysSystem::getInston();
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();

    SingleActionParamType *psingleparam;
    CombActionParamType *pcombparam;
    groupActionParamType *pgrpparam;
    PartParamType *ppartparam;

    SingleActionStateType *pSingleActState;
    PartStateType *pPartState;
    CombActionStateType *pCombActState;
    groupActionStateType *pGroupActState;

    //声光模型初始化
    globalParam_DeviceAlarmType alarmparam;
    pParamMng->GetRunValueGlobalDeviceAlarmParam(alarmparam);
    Alarm::getInston()->AlarmModuleEnable((EnableType)alarmparam.DevAlarmEnable);

    //单动作模型初始化
    SigActionModuleInit();
    for(u_int16_t i=1; i<=SINGLEACTION_NUM_MAX;i++)
    {
        psingleparam = NULL;
        psingleparam = pParamMng->GetRunValueSingleActParam(i);
        if(NULL == psingleparam)
            break;
        pSingleActState = NULL;
        pSingleActState = (SingleActionStateType *)pSystem->GetSingleActState(psingleparam->baseParam.ID);
        if(NULL == pSingleActState)
            break;

        SigActionModuleLoad(i-1,psingleparam,pSingleActState);
    }

    //组合动作模型初始化
    CombActionModuleInit();
    for(u_int16_t i=1; i<=COMBACTION_NUM_MAX;i++)
    {
        pcombparam = NULL;
        pcombparam = pParamMng->GetRunValueCombActParam(i);
        if(NULL == pcombparam )
            break;
        pCombActState = NULL;
        pCombActState = pSystem->GetCombActState(i);
        if(NULL == pCombActState)
            break;
        CombActionModuleLoad(i-1,pcombparam,pCombActState);
    }

    //成组动作模型初始化
    groupActionModuleInit();
    for(u_int16_t i=1; i<=GROUPACTION_NUM_MAX;i++)
    {
        pgrpparam = NULL;
        pgrpparam = pParamMng->GetRunValueGrpActParam(i);
        if(NULL == pgrpparam)
            break;
        pGroupActState = NULL;
        pGroupActState = pSystem->GetGrpActState(i);
        if(NULL == pGroupActState)
            break;
        groupActionModuleLoad(i-1,pgrpparam,pGroupActState);
    }

    //部件模型初始化
    partModuleInit();
    for(u_int16_t i=1; i<=PART_NUM_MAX;i++)
    {
        ppartparam = NULL;
        ppartparam = pParamMng->GetRunValuePartParam(i);
        if(NULL == ppartparam)
            break;

        pPartState = (PartStateType *)pSystem->GetPartState(i);
        if(pPartState == NULL)
            break;
        partModuleLoad(i-1,ppartparam,pPartState);
    }
    qDebug()<<"*************ActionCtrl,Init,end*********";
    return 0;
}

bool ActionCtrl::SetProcIntvMs(uint16_t ms_t)
{
    m_mutex->lock();
    if((ms_t < static_cast<u_int16_t>(ACTCTRL_PROC_TIMER_MIN) )\
        ||(ms_t > static_cast<u_int16_t>(ACTCTRL_PROC_TIMER_MAX)))
        ProcSetIntv = static_cast<u_int16_t>(ACTCTRL_PROC_TIMER_DEFAULT);
    else
        ProcSetIntv = ms_t;
    m_mutex->unlock();

    return true;
}

void ActionCtrl::reportManualSingleAction(u16 delay)
{
    u8 i;
    u8 sensorId;
    u8 temp[20];
    u8 length;
    HysScSensorType value;
    HysSystem *pSystem = HysSystem::getInston();

    if(pSystem->GetSystemSelfStateSlaveMode() != SYSMODE_SLAVE_CTRL)
    {
        ManualActionSupportNum = 0;
        return;
    }

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    if(ManualActionSupportNum == pParamMng->GetRunValueGlobalSysServerAddr())
        return;

    for(i=0;i<MANUALACTIONINFO_LEN;i++)
    {
        uint Data = 0x00;
        if(GetManualActionClearBuf(i,Data) == false)
            break;

        if( Data != 0)
        {
            sensorId = getPartInSingleActionOfSensorId(Data);
            if(sensorId!= 0)
            {
                HysSystem *pSystem = HysSystem::getInston();
                pSystem->GetSensorState(sensorId,value);
                memset(temp,0,sizeof(temp));

                temp[0] = REPORTENUM_SENSOR;
                temp[1] = ENUM_SENSOR;
                temp[2] = sensorId;
                length =sizeof(HysScSensorType)-0x02;
                memcpy(&temp[3],&length,2);
                memcpy(&temp[5],&value.Value,length);

                LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
                u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();
                UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT, (uchar)serverID,V4APP_STATUS_REPORT,\
                               temp, 5+length);
            }
        }
    }
}

void ActionCtrl::ActionCtrlPeriodRun(void)//周期性处理的函数
{
    Alarm *pAlarm = Alarm::getInston();
    u8 i = 0;
    uint len = 0;
    u8  manualActionID[MANUALACTIONINFO_LEN];

    usleep(ACTCTRL_PROC_POWERON_TIME_SET*1000);
    while(1)
    {
        usleep(ProcSetIntv*1000);
        pAlarm->AlarmMng(ProcSetIntv);
        TiggerDelayCycle(ProcSetIntv);
        reportManualSingleAction(ProcSetIntv);

        for(i=0; i<SINGLEACTION_NUM_MAX;i++)
        {
            SigActionStepProcess(ProcSetIntv,i);
        }

        for(i=0; i<COMBACTION_NUM_MAX;i++)
        {
            CombActionStepProcess(ProcSetIntv,i);
        }

        for(i=0; i<GROUPACTION_NUM_MAX;i++)
        {
            groupActionStepProcess(ProcSetIntv,i);
        }

        for(i=0; i<PART_NUM_MAX;i++)
        {
            partCycleProcess(i,ProcSetIntv);
        }

        if(getManualActionInfoBufSzie() >0)
        {
            len = CheckManualActionList(manualActionID);
            if(len)
            {
                UdpAppProcCtrl(V4APP_DEV_TYPE_SC,manualCtrlAddr,V4APP_STATUS_MANUAL_ACT_HEARTBEAT,manualActionID,len);
            }
        }
    }
}


/*********************************************************
** 函数名：SysCtrlMngTask
** 输　入：None
** 输　出：None
** 功能描述：控制器系统功能控制下发任务
************************************************************/
void ActionCtrl::SysCtrlBufInit(void)
{
    int i;

    qDebug()<<"*************SysCtrlBufInit,Init,start*********";
    m_mutex->lock();
    memset(ctrlInfoMng.buf, 0 ,sizeof(ctrlInfoMng.buf));
    ctrlInfoMng.Cnt=0;
    ctrlInfoMng.pHead = &ctrlInfoMng.buf[0];
    ctrlInfoMng.pEnd  = &ctrlInfoMng.buf[0];
    for(i=0 ; i<CTRL_BUF_LEN - 1; i++)
    {
        ctrlInfoMng.buf[i].pNext = &ctrlInfoMng.buf[i+1];
    }
    ctrlInfoMng.buf[CTRL_BUF_LEN-1].pNext = &ctrlInfoMng.buf[0];
    m_mutex->unlock();
    qDebug()<<"*************SysCtrlBufInit,Init,end*********";
}

int ActionCtrl::InsCtrlBufQueue(SysCtrlInfoType *pdata)
{
    int CntTemp = 0;

    if(ctrlInfoMng.pHead == nullptr)
        return -1;
    if(ctrlInfoMng.pHead->pNext == nullptr)
        return -1;
    if(ctrlInfoMng.pEnd->pNext == nullptr)
        return -1;
    if(pdata == nullptr)
        return -1;
    /***对写入参数的有效性进行判断*****/
    if((pdata->actionType == ENUM_SINGLE_ACTION)&&((pdata->actionID > SINGLEACTION_NUM_MAX)||(pdata->actionID == 0)))
        return -1;
    if((pdata->actionType == ENUM_COMBINATION_ACTION)&&((pdata->actionID > COMBACTION_NUM_MAX)||(pdata->actionID == 0)))
        return -1;
    if((pdata->actionType == ENUM_GROUP_ACTION)&&((pdata->actionID > GROUPACTION_NUM_MAX)||(pdata->actionID == 0)))
        return -1;
    if((pdata->actionType == ENUM_GROUP_ACTION_KZQ)&&((pdata->actionID > GROUPACTION_NUM_MAX)||(pdata->actionID == 0)))
        return -1;
    if((pdata->actionType == ENUM_ALARM)&&((pdata->actionID >= ALARM_MODE_MAX)||(pdata->actionID == 0)))
        return -1;

    /*******/
    m_mutex->lock();
    CntTemp = ctrlInfoMng.Cnt;
    m_mutex->unlock();

    if(CntTemp <CTRL_BUF_LEN)
    {
        m_mutex->lock();

        ctrlInfoMng.pHead->actionType = pdata->actionType;
        ctrlInfoMng.pHead->actionID = pdata->actionID;
        ctrlInfoMng.pHead->ParamLength = pdata->ParamLength;
        memcpy(ctrlInfoMng.pHead->ParamBuf,pdata->ParamBuf,pdata->ParamLength);

        memcpy(&ctrlInfoMng.pHead->ctrl.CtrlCmd,&pdata->ctrl.CtrlCmd,sizeof(ActionCtrlType));
        ctrlInfoMng.pHead = static_cast<SysCtrlInfoType*>(ctrlInfoMng.pHead->pNext);
        ctrlInfoMng.Cnt++;

        m_mutex->unlock();
        m_sem.post_sem();
        return 0;
    }

    return -1;
}

void ActionCtrl::DelCtrlBufQueue(void)
{
    if(ctrlInfoMng.Cnt)
    {
        ctrlInfoMng.Cnt --;
        ctrlInfoMng.pEnd = static_cast<SysCtrlInfoType*>(ctrlInfoMng.pEnd->pNext);
    }
}

void ActionCtrl::ActionCtrlSemProc(void)//信号量处理的函数
{
    u8	ret,temp[2];
    int err;
    u8 alarmMode[2];
    u16 mutexAction[2];

    usleep(ACTCTRL_PROC_POWERON_TIME_SET*1000);

    SysCtrlInfoType *pctrl;
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    HysSystem *pSystem = HysSystem::getInston();

    while(1)
    {
        err = m_sem.wait_sem();
        if (err == 0)
        {
            m_mutex->lock();
            if(ctrlInfoMng.Cnt)
            {
                pctrl =ctrlInfoMng.pEnd;
                if((pctrl->actionID !=0)&&(pSystem->GetSystemSelfStateSlaveMode() !=SYSMODE_CONFIG)
                    &&(pSystem->IsEmergencyStopAction(pctrl)==0)
                    &&(pSystem->GetSystemSelfStateLockStatus() != SYSSTOP_PRESS)
                    &&(pSystem->GetSystemSelfStateStopStatus() != SYSSTOP_PRESS))
                {
                    if((pParamMng->GetRunValueGlobalManualCtrlFirst() == EnableType_ENABLE && \
                        (pSystem->GetSystemSelfStateSlaveMode() ==SYSMODE_MASTER_CTRL ||\
                        (pSystem->GetSystemSelfStateSlaveMode() == SYSMODE_SLAVE_CTRL && pctrl->ctrl.CtrlSourceType == ActionCtrl_SourceType_SERVER)))
                         ||(pSystem->GetAllZeroPersonLockState() == true))
                    {
                        ;
                    }
                    else
                    {
                        if(pctrl->actionType ==ENUM_SINGLE_ACTION)
                        {
                            ret = SigActionPerformJudge(pctrl->ctrl,pctrl->actionID-1,mutexAction);
                            if(ret)
                            {
                                if(pctrl->actionType == ENUM_SINGLE_ACTION && pctrl->ctrl.CtrlMode ==ActionCtrl_ModeType_MANUAL \
                                   && pParamMng->GetRunValueGlobalManualCtrlFirst() == EnableType_ENABLE)
                                {
                                    for(u8 i=0; i<SINGLEACTION_NUM_MAX;i++)
                                    {
                                        AutoSigActionAllStop(i);
                                    }

                                    for(u8 i=0; i<COMBACTION_NUM_MAX;i++)
                                    {
                                        CombActionAllStop(i);
                                    }

                                    for(u8 i=0; i<GROUPACTION_NUM_MAX;i++)
                                    {
                                        groupActionAllStop(i);
                                    }
                                }
                                SigActionRunStateJudge(pctrl->ctrl,pctrl->actionID-1);
                                SigActionCtrlFunc(pctrl->ctrl,pctrl->actionID-1,0);
                            }
                            else
                            {
                                HysSystem *pSystem = HysSystem::getInston();
                                pSystem->WriteLogApi(pctrl->actionID,LOG_TYPE_HANDLE_SINGLE,LOG_STATUS_DISABLE,0,0);
                            }
                        }
                        else if(pctrl->actionType ==ENUM_COMBINATION_ACTION)
                        {
                            ret= CombActionPerformJudge(pctrl->ctrl,pctrl->actionID-1);
                            if(ret)
                            {
                                ret= checkCombActionPriorityState(pctrl->actionID-1);
                                if(ret)
                                    CombActionCtrlFunc(pctrl->ctrl,pctrl->actionID-1);
                            }
                            else
                            {
                                HysSystem *pSystem = HysSystem::getInston();
                                pSystem->WriteLogApi(pctrl->actionID,LOG_TYPE_HANDLE_COMB,LOG_STATUS_DISABLE,0,0);
                            }
                        }
                        else if(pctrl->actionType ==ENUM_GROUP_ACTION)
                        {
                            groupActionCtrlFunc(pctrl->ctrl,pctrl->actionID-1,pctrl->ParamBuf,pctrl->ParamLength);
                        }
                        else if(pctrl->actionType ==ENUM_GROUP_ACTION_KZQ)
                        {
                            groupActionJudge(pctrl->ctrl,pctrl->actionID-1);
                        }
                        else if(pctrl->actionType ==ENUM_ALARM)
                        {
                            if(pctrl->ParamLength >=3)
                            {
                                memcpy(&mutexAction[0],&(pctrl->ParamBuf[1]),2);
                                temp[1] = pctrl->ParamBuf[0];
                            }
                            else
                            {
                                temp[1]=AlarmType_SoundLight;
                                mutexAction[0]=SINGLEACTION_ALARM_ALWAY;
                            }

                            alarmMode[0] = SINGLEACTION_ALARM_MODE_SOUND_DEFAULT;
                            alarmMode[1] = SINGLEACTION_ALARM_MODE_LIGHT_DEFAULT;
                            if(pctrl->ctrl.CtrlCmd == ActionCtrl_CmdType_STARTUP)
                            {
                                temp[0] = BaseCtrlType_STARTUP;
                                u_int16_t buzzermode = SELF_IO_POINT_BUZZER_NEAR_SELECTED;
                                DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                                               SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
                            }
                            else
                            {
                                temp[0] = BaseCtrlType_STOP;
                                u_int16_t buzzermode = SELF_IO_POINT_BUZZER_MODE_NONE;
                                DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                                               SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
                            }
                            Alarm *pAlarm = Alarm::getInston();
                            pAlarm->CtrlAlarm((AlarmType)(temp[1]),mutexAction[0],(BaseCtrlType)(temp[0]),\
                                    alarmMode,ENUM_ALARM,pctrl->ctrl.CtrlSourceAddr,pctrl->actionID);//ALARM_MODE_1
                        }
                    }
                }
                DelCtrlBufQueue();
            }
           m_mutex->unlock();
        }
    }
}


/*********************************************************
** 函数名：
** 输　入：None
** 输　出：None
** 功能描述：被控信息维护
************************************************************/
void ActionCtrl::ActionDataInit(void)
{
    uint8_t i;

    qDebug()<<"*************ActionDataInit,Init,start*********";
    m_mutex->lock();
    memset(actionInfoMng.buf,0 ,sizeof(actionInfoMng.buf));
    actionInfoMng.pHead = actionInfoMng.pEnd =NULL;

    actionInfoMng.Cnt = 0;
    for(i=0 ; i<ACTIONINFO_BUF_LEN - 1; i++)
    {
        actionInfoMng.buf[i].pNext = &actionInfoMng.buf[i+1];
    }
    actionInfoMng.buf[ACTIONINFO_BUF_LEN-1].pNext = nullptr;
    actionInfoMng.pFree = &actionInfoMng.buf[0];

    manualActionMng.Cnt = 0;
    memset(manualActionMng.buf,0 ,sizeof(manualActionMng.buf));
    manualActionMng.pHead = manualActionMng.pEnd =NULL;

    for(i=0 ; i<ACTIONINFO_BUF_LEN - 1; i++)
    {
        manualActionMng.buf[i].pNext = &manualActionMng.buf[i+1];
    }
    manualActionMng.buf[ACTIONINFO_BUF_LEN-1].pNext = NULL;
    manualActionMng.pFree = &manualActionMng.buf[0];
    m_mutex->unlock();
    qDebug()<<"*************ActionDataInit,Init,end*********";
}

void ActionCtrl::InsActionBufQueue(ActionInfoType *pdata)
{
    ActionInfoType *ptemp;

    //m_mutex->lock();//已进行保护

    if(actionInfoMng.Cnt <ACTIONINFO_BUF_LEN)
    {
        ptemp = actionInfoMng.pFree;
        actionInfoMng.pFree = static_cast<ActionInfoType*>(ptemp->pNext) ;
        ptemp->pNext = NULL;

        ptemp->actionType = pdata->actionType;
        ptemp->actionID = pdata->actionID;
        ptemp->runstate = pdata->runstate;

        if (actionInfoMng.pEnd != NULL)
        {
            actionInfoMng.pEnd->pNext = ptemp;
        }

        actionInfoMng.pEnd = ptemp;
        if(actionInfoMng.pHead == NULL)
        {
            actionInfoMng.pHead = ptemp;
        }
        actionInfoMng.Cnt ++;
    }

    //m_mutex->unlock();//已进行保护
}

int ActionCtrl::compareActionBuf(ActionInfoType *pdata,ActionInfoType *pNode)
{
    if((pdata ==NULL)||(pNode ==NULL))
        return -1;

    if((pNode->actionType == pdata->actionType)&&(pNode->actionID == pdata->actionID))
        return 0;

    return -1;
}

void ActionCtrl::DelActionBufQueue(ActionInfoType *pdata)
{
    ActionInfoType *pPrevious;
    ActionInfoType *pNode,*pDeleted;
    int nCompareRes;

    //m_mutex->lock();//已进行保护

    if(actionInfoMng.Cnt >0)
    {
        pPrevious = NULL;
        pNode = actionInfoMng.pHead;
        while (pNode != NULL)
        {
            nCompareRes = compareActionBuf(pdata, pNode);
            if (nCompareRes == 0)
            {
                pDeleted = pNode;
                pNode = static_cast<ActionInfoType *>(pNode->pNext) ;

                if (pDeleted == actionInfoMng.pHead)
                {
                    actionInfoMng.pHead = static_cast<ActionInfoType *>(pDeleted->pNext);
                }
                else
                {
                    pPrevious->pNext = pDeleted->pNext;
                }

                if (pDeleted == actionInfoMng.pEnd)
                {
                    actionInfoMng.pEnd = pPrevious;
                }

                pDeleted->pNext =actionInfoMng.pFree;
                actionInfoMng.pFree = pDeleted;
                actionInfoMng.Cnt --;

                break;
            }

            pPrevious = pNode;
            pNode = static_cast<ActionInfoType *>(pNode->pNext);
        }
    }

    if(actionInfoMng.Cnt == 0)
    {
        ClearManualActionClearBuf();
    }

    //m_mutex->unlock();//已进行保护
}

u8 ActionCtrl::UpdateActionBufQueue(ActionInfoType *pdata)
{
    ActionInfoType *pPrevious;
    ActionInfoType *pNode;
    int nCompareRes;

    if(actionInfoMng.Cnt >0)
    {
        pPrevious = NULL;
        pNode = actionInfoMng.pHead;
        while (pNode != NULL)
        {
            nCompareRes = compareActionBuf(pdata, pNode);
            if (nCompareRes == 0)
            {
                pNode->runstate = pdata->runstate;

                return 1;
            }

            pPrevious = pNode;
            pNode = static_cast<ActionInfoType *>(pNode->pNext);
        }
    }

    return 0;
}

void ActionCtrl::AddActionInfo(u8 type,u8 id,u8 runState)
{
    ActionInfoType action;
    u8 ret;
    HysSystem *pSystem = HysSystem::getInston();

    action.actionID = id;
    action.actionType = type;
    action.runstate = runState;
    action.pNext =NULL;

    if((runState == RunStateType_IDLE)||(runState == RunStateType_STOP))
    {
        DelActionBufQueue(&action);
        if((actionInfoMng.Cnt==0)&&(pSystem->GetSystemSelfStateSlaveMode()!=SYSMODE_CONFIG_FIN))
            pSystem->SetSystemSelfStateSlaveMode(SYSMODE_NORMAL);
    }
    else
    {
        ret = UpdateActionBufQueue(&action);
        if(ret == 0)
            InsActionBufQueue(&action);

        if(actionInfoMng.Cnt>0)
            pSystem->SetSystemSelfStateSlaveMode(SYSMODE_SLAVE_CTRL);
    }
}

/***停止所有的动作**/
void ActionCtrl::StopAllAction(u8 src)
{
    u8 i;

    for(i=0; i<SINGLEACTION_NUM_MAX;i++)
    {
        SigActionAllStop(i);
    }

    for(i=0; i<COMBACTION_NUM_MAX;i++)
    {
        CombActionAllStop(i);
    }

    for(i=0; i<GROUPACTION_NUM_MAX;i++)
    {
        groupActionAllStop(i);
    }
    if(src == ActionCtrl_SourceType_CONTROLLER)
    {
        Alarm *pAlarm = Alarm::getInston();
        pAlarm->AlarmAllStop();
    }
    ActionDataInit();
    SigActionTrggerTimerInit();
}

ActionInfoType * ActionCtrl::getActionInfoHead(void)
{
    return actionInfoMng.pHead;
}

u8  ActionCtrl::getActionInfoBufSzie(void)
{
    return actionInfoMng.Cnt;
}

//----------------manual action mng-------------------
void ActionCtrl::InsManualActionBufQueue(ActionInfoType *pdata)
{
    ActionInfoType *ptemp;

    m_mutex->lock();

    if(manualActionMng.Cnt <ACTIONINFO_BUF_LEN)
    {
        ptemp = manualActionMng.pFree;
        manualActionMng.pFree = static_cast<ActionInfoType*>(ptemp->pNext);
        ptemp->pNext = NULL;

        ptemp->actionType = pdata->actionType;
        ptemp->actionID = pdata->actionID;
        ptemp->runstate = pdata->runstate;

        if (manualActionMng.pEnd != nullptr)
        {
            manualActionMng.pEnd->pNext = ptemp;
        }

        manualActionMng.pEnd = ptemp;
        if(manualActionMng.pHead == nullptr)
        {
            manualActionMng.pHead = ptemp;
        }
        manualActionMng.Cnt ++;
    }

    m_mutex->unlock();
}

void ActionCtrl::DelManualActionBufQueue(ActionInfoType *pdata)
{
    ActionInfoType *pPrevious;
    ActionInfoType *pNode,*pDeleted;
    int nCompareRes;

    m_mutex->lock();

    if(manualActionMng.Cnt >0)
    {
        pPrevious = NULL;
        pNode = manualActionMng.pHead;
        while (pNode != NULL)
        {
            nCompareRes = compareActionBuf(pdata, pNode);
            if (nCompareRes == 0)
            {
                pDeleted = pNode;
                pNode = static_cast<ActionInfoType *>(pNode->pNext);

                if (pDeleted == manualActionMng.pHead)
                {
                    manualActionMng.pHead = static_cast<ActionInfoType *>(pDeleted->pNext);
                }
                else
                {
                    pPrevious->pNext = static_cast<ActionInfoType *>(pDeleted->pNext);
                }

                if (pDeleted == manualActionMng.pEnd)
                {
                    manualActionMng.pEnd = pPrevious;
                }

                pDeleted->pNext =manualActionMng.pFree;
                manualActionMng.pFree = pDeleted;
                manualActionMng.Cnt --;

                break;
            }

            pPrevious = pNode;
            pNode = static_cast<ActionInfoType *>(pNode->pNext);
        }
    }

    m_mutex->unlock();
}

u8 ActionCtrl::UpdateManualActionBufQueue(ActionInfoType *pdata)
{
    ActionInfoType *pPrevious;
    ActionInfoType *pNode;
    int nCompareRes;

    m_mutex->lock();

    if(manualActionMng.Cnt >0)
    {
        pPrevious = nullptr;
        pNode = manualActionMng.pHead;
        while (pNode != nullptr)
        {
            nCompareRes = compareActionBuf(pdata, pNode);
            if (nCompareRes == 0)
            {
                pNode->runstate = pdata->runstate;

                m_mutex->unlock();
                return 1;
            }

            pPrevious = pNode;
            pNode = static_cast<ActionInfoType *>(pNode->pNext);
        }
    }

    m_mutex->unlock();

    return 0;
}

u8 ActionCtrl::CheckManualActionList(u8 *pactionList)
{
    ActionInfoType *pNode;
    u8 len=0;

    if(pactionList==NULL)
        return 0;

    if(manualActionMng.Cnt >0)
    {
        pNode = manualActionMng.pHead;
        while (pNode != NULL)
        {
            if((pNode->runstate == RunStateType_RUN_MANUAL)&&( pNode->actionType ==0))
            {
                pactionList[len] = pNode->actionID;
                if(len<MANUALACTIONINFO_LEN)
                    len++;
            }
            pNode = static_cast<ActionInfoType *>(pNode->pNext);
        }
    }
    return len;
}

void ActionCtrl::AddManualActionInfo(u8 type,u8 id,u8 runState)
{
    ActionInfoType action;
    u8 ret;

    action.actionID = id;
    action.actionType = type;
    action.runstate = runState;
    action.pNext =nullptr;

    if((runState == RunStateType_IDLE)||(runState == RunStateType_STOP))
    {
        DelManualActionBufQueue(&action);
    }
    else
    {
        ret = UpdateManualActionBufQueue(&action);
        if(ret == 0)
            InsManualActionBufQueue(&action);
    }
}

u8  ActionCtrl::getManualActionInfoBufSzie(void)
{
    return manualActionMng.Cnt;
}

/****主要用于清零ManualActionClear这个数组****/
bool ActionCtrl::ClearManualActionClearBuf(void)
{
    QWriteLocker lock(m_ManuClear_Locker);
    memset(&ManualActionClear,0,sizeof(ManualActionClear));
    return  true;
}

/***设置ManualActionClear的数值***/
bool ActionCtrl::SetManualActionClearBuf(uint id,u_int8_t number)
{
    if(id > MANUALACTIONINFO_LEN)
        return false;

    QWriteLocker lock(m_ManuClear_Locker);
    ManualActionClear[id] = number;

    return  true;
}

/***获取ManualActionClear的数值***/
bool ActionCtrl::GetManualActionClearBuf(uint id,uint &number)
{
    if(id > MANUALACTIONINFO_LEN)
        return false;

    QReadLocker lock(m_ManuClear_Locker);
    number = ManualActionClear[id];

    return  true;
}

/******/
bool ActionCtrl::SetManualActionSupportNum(u_int16_t addr)
{
    if(addr > V4APP_DEV_ID_MAX)
        return false;

    ManualActionSupportNum = addr;
    return true;
}

/**新增加***/
u8 ActionCtrl::CombActionCtrlFuncApi(ActionCtrlType ctrl,u8 actionID)
{
    return CombActionCtrlFunc(ctrl,actionID);
}
