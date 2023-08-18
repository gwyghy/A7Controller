#ifndef ACTIONCTRL_H
#define ACTIONCTRL_H

/**************/
#include <sys/types.h>
#include <QMutex>
#include "actionctrldatatype.h"
#include <QReadWriteLock>
#include "./sem/AppSem.h"
#include "devicemngapi.h"
#include "alarm.h"

/****动作控制****/
class ActionCtrl
{
public:
    ActionCtrl();
    ~ActionCtrl();
    static ActionCtrl *getInston();

    int Init(void);
    bool SetProcIntvMs(uint16_t ms_t);

    void ActionCtrlPeriodRun(void);//周期性处理的函数
    void ActionCtrlSemProc(void);//信号量处理的函数
    void AddActionInfo(u8 type,u8 id,u8 runState);
    /***停止所有的动作**/
    void StopAllAction(u8 src);
    /***插入新的控制动作***/
    int InsCtrlBufQueue(SysCtrlInfoType *pdata);

    /****主要用于清零ManualActionClear这个数组****/
    bool ClearManualActionClearBuf(void);
    /***设置ManualActionClear的数值***/
    bool SetManualActionClearBuf(uint id, u_int8_t number);
    /***获取ManualActionClear的数值***/
    bool GetManualActionClearBuf(uint id,uint &number);

    /******/
    bool SetManualActionSupportNum(u_int16_t addr);

    /**新增加***/
    u8 CombActionCtrlFuncApi(ActionCtrlType ctrl,u8 actionID);

private:
    void reportManualSingleAction(u16 delay);
    /*****/
    void SysCtrlBufInit(void);
    void DelCtrlBufQueue(void);

    /*******/
    void ActionDataInit(void);
    void InsActionBufQueue(ActionInfoType *pdata);
    int compareActionBuf(ActionInfoType *pdata,ActionInfoType *pNode);
    void DelActionBufQueue(ActionInfoType *pdata);
    u8 UpdateActionBufQueue(ActionInfoType *pdata);

    ActionInfoType * getActionInfoHead(void);
    u8  getActionInfoBufSzie(void);
    void InsManualActionBufQueue(ActionInfoType *pdata);
    void DelManualActionBufQueue(ActionInfoType *pdata);
    u8 UpdateManualActionBufQueue(ActionInfoType *pdata);
    u8 CheckManualActionList(u8 *pactionList);
    void AddManualActionInfo(u8 type,u8 id,u8 runState);
    u8  getManualActionInfoBufSzie(void);

private:
    /****/
    static ActionCtrl* that;

    semt m_sem;//信号量
    QMutex *m_mutex;//2023.01.09,为排查故障，改变互斥量的类型，mry

    u_int16_t ProcSetIntv;      //设置的处理周期

    SysCtrlInfoBufType    ctrlInfoMng;
    ActionInfoBufType	 actionInfoMng;
    ActionInfoBufType	 manualActionMng;

    /******/
    u8 manualCtrlAddr ;//手动控制的架号

    //手动控制单动作数组
    u_int8_t   ManualActionClear[MANUALACTIONINFO_LEN];
    uchar   ManualActionSupportNum;

    /*****/
    QReadWriteLock *m_ManuClear_Locker;//用于保护ManualActionClear及ManualActionSupportNum，专用
};

#endif // ACTIONCTRL_H
