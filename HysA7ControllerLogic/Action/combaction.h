#ifndef COMBACTION_H
#define COMBACTION_H

/*******************************/
#include "./ScRunParamMng/paramglobaldefine.h"
#include "./ScRunParamMng/combactionparam.h"
#include "./HysDeviceMng/devctrl.h"

/*------------------------state define----------------------*/
typedef struct
{
    ParamUnit       Id;          //新增加。mry
    ParamUnit		RunState;	//动作执行状态
    ParamUnit		RunStateBak;//动作执行状态(备份)
    ActionCtrlType	ActionCtrl;	//动作控制信息
}CombActionStateType;


/*------------------------module define----------------------*/
typedef struct
{
    CombActionParamType		*pParam;
    CombActionStateType		*pState;

    int 					ticks_alarm;
    int 					ticks_delay[COMBACTION_SINGLE_NUM];
    u8 					    delayFin[COMBACTION_SINGLE_NUM];
    u8						suspend_flag;
    u16						cycle_cnt;
    u16						cycle_start;
    u16						cycle_end;
    u8						runFin[COMBACTION_SINGLE_NUM];
}CombActionModuleType;


/*------------------------other define----------------------*/
void CombActionModuleInit(void);
void CombActionModuleLoad(u8 moduleID,CombActionParamType *pParam,CombActionStateType *pState);
u8 CombActionCtrlFunc(ActionCtrlType ctrl,u8 actionID);
u8 CombActionStepProcess(u16 delay,u8 moduleID);
u8 getCombActionFin(u8 combNum,u8 actionID,u8 *finFlag);
u8 CombActionAllStop(u8 actionID);
u8 CombActionPerformJudge(ActionCtrlType ctrl,u8 actionID);
u8 checkCombActionPriorityState(u8 moduleID);

#endif // COMBACTION_H
