#ifndef GROUPACTION_H
#define GROUPACTION_H

/*******************************/
#include "./HysDeviceMng/devctrl.h"
#include "./ScRunParamMng/paramglobaldefine.h"
#include "./ScRunParamMng/groupactionparam.h"
#include "singleaction.h"

/*------------------------state define----------------------*/
typedef struct
{
    ParamUnit       Id;          //新增加。mry
    ParamUnit		RunState;	//动作执行状态
    ParamUnit		RunStateBak;//动作执行状态(备份)
    ActionCtrlType	ActionCtrl;	//动作控制信息
}groupActionStateType;



/*------------------------module define----------------------*/
enum
{
    GroupModuleParamENUM_STARTID=0,
    GroupModuleParamENUM_ENDID,
    GroupModuleParamENUM_DIRECT,
    GroupModuleParamENUM_MAX
};

typedef struct
{
    groupActionParamType	*pParam;
    groupActionStateType	*pState;

    int 					ticks_delay;
    u8						suspend_flag;
    u8						currentOperateId;
    //u8						actionSupport[GROUP_ACTION_SUPPORT_NUM_MAX];
    u8						startID;
    u8						endID;
    u8						direct;
    u8						groupEndID;
    u8						groupFinish;
}groupActionModuleType;

/*------------------------other define----------------------*/
void groupActionModuleInit(void);
void groupActionModuleLoad(u8 moduleID,groupActionParamType *pParam,groupActionStateType *pState);
u8 groupActionCtrlFunc(ActionCtrlType ctrl,u8 actionID,u8 *pctrlParam,u8 length);
u8 groupActionStepProcess(u16 delay,u8 moduleID);
u8 groupActionAllStop(u8 actionID);
u8 stopGroupAction(u8 startId,u8 endId,u8 moduleID,ActionCtrl_CmdType cmd);
void groupActionJudge(ActionCtrlType ctrl,u8 actionID);

#endif // GROUPACTION_H
