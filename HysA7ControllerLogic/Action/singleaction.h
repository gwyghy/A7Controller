#ifndef SINGLEACTION_H
#define SINGLEACTION_H

/*******************************/
#include "./ScRunParamMng/paramglobaldefine.h"
#include "./ScRunParamMng/singleactionparam.h"
#include "./ScRunParamMng/combactionparam.h"
#include "./HysDeviceMng/devctrl.h"

/*------------------------state define----------------------*/


typedef struct
{
    ParamUnit       Id;          //新增加。mry
    ParamUnit		RunState;    //动作执行状态
    ParamUnit		RunStateBak; //动作执行状态(备份)
    ParamUnit       RunTime;	   //运行时间
    ParamUnit		CombNum;     //单动作所属的组合动作号
    ActionCtrlType	ActionCtrl;	//动作控制信息
}SingleActionStateType;

/*------------------------module define----------------------*/
typedef int (*sigActionApiType )(u8 actionID,u8 combID,void *pdata);

enum
{
    SIGACTIONAPI_NONE =0,
    SIGACTIONAPI_COMB_STOP,    //所在组合动作停止
    SIGACTIONAPI_MAX
};

typedef struct
{
    SingleActionParamType		*pParam;
    SingleActionStateType		*pState;

    int						ticks_alarm;
    int 					ticks_run;
    u8						suspend_flag;
    u8						suspend_action_flag;
    u8						combAction_num;
    u8            trigger_flag;
    u8						runFin[COMBACTION_NUM_MAX];
    sigActionApiType		apiFunc[SIGACTIONAPI_MAX];
}SingleActionModuleType;


typedef struct
{
    SingleAction_TriggerActionType		*pTigger;
    ActionCtrlType *pCtrl;
    int		ticks;
    u8		enbale;
    u8 		moduleID;
    u8      combID;
}TiggerTimerType;


/*------------------------other define----------------------*/
void SigActionModuleLoad(u8 moduleID,SingleActionParamType *pParam,SingleActionStateType *pState);
void SigActionTrggerTimerInit(void);
void SigActionModuleInit(void);
u8 SigActionCtrlFunc(ActionCtrlType ctrl,u8 actionID,u8 combAction_num);
u8 SigActionCtrlFunc_stop(ActionCtrlType ctrl,u8 actionID,u8 combAction_num);
void TiggerDelayCycle(u16 delay);
u8 getSigActionFin_comb(u8 combNum,u8 actionID,u8 *finFlag);
u8 clearSigActionFin_comb(u8 combNum,u8 actionID);
u8 SigActionStepProcess(u16 delay,u8 moduleID);
u8 SigActionAllStop(u8 moduleID);
u8 SigActionPerformJudge(ActionCtrlType ctrl,u8 actionID,u16 *pmutexAction);

int sigActionApi_CombStopFunc (u8 actionID,u8 combID,void *pdata);

u8 getSingleActionState(u8 moduleID, Action_RunStateType* type);
u8 getSigActionTriggerFlag_comb(u8 actionID,u8 *state);
u8 setSigActionTriggerFlag_comb(u8 actionID,u8 state);
void SigActionRunStateJudge(ActionCtrlType ctrl,u8 moduleID);
u8 AutoSigActionAllStop(u8 moduleID);


#endif // SINGLEACTION_H
