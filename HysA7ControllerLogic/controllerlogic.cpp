#include "controllerlogic.h"
#include "unistd.h"
#include "HysAppInteraction/appmsgtype.h"
#include <QDebug>
#include "pthrd/thrd.h"
#include "./Sensor/sensor.h"

/**********/
enum
{
    THREAD_STACK_LEN_64K = 0x10000,
    THREAD_STACK_LEN_128K = 0x20000,
    THREAD_STACK_LEN_256K = 0x40000,
    THREAD_STACK_LEN_512K = 0x80000,

    THREAD_STACK_LEN_1M = 0x100000,
    THREAD_STACK_LEN_2M = 0x200000,
    THREAD_STACK_LEN_4M = 0x400000,
    THREAD_STACK_LEN_8M = 0x800000,
};

/**函数声明****/
void *mHysSysPeriodicProc_thread(void *arg);
void *mActPeriodicProc_thread(void *arg);
void *mActSemProc_thread(void *arg);
void *mDevMngAppShmProc_thread(void *arg);
void *mDevMngDevPeriodicProc_thread(void *arg);
void *mAppIteractionMsgProc_thread(void *arg);

/********/
ControllerLogic::ControllerLogic(QObject *parent) : QObject(parent)
{
    Init();
}

ControllerLogic::~ControllerLogic()
{
    qDebug()<<"*************ControllerLogic,~ControllerLogic**********";
    delete pActCtrl;
    delete pDeviceMng;
    delete pSystem;
}

int ControllerLogic::Init(void)
{
    pthread_attr_t attr;
    size_t PageSize;
    size_t getSize;
    void *statckAddr = nullptr;
    void *getStatckAddr = nullptr;

    /*****/
    pSystem = HysSystem::getInston(this);
    pSystem->Init();

    pDeviceMng = HysDeviceMng::getInston();
    pActCtrl = ActionCtrl::getInston();
    pActCtrl->SetProcIntvMs(10);
    pActCtrl->Init();

    pAppIteractionMng = HysAppInteraction::getInston();
    pAppIteractionMng->Init();

    /******/
    PageSize = getpagesize();

    /******/
    posix_memalign(&statckAddr,PageSize,THREAD_STACK_LEN_1M);
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr,statckAddr,THREAD_STACK_LEN_1M);
    pthread_attr_getstack(&attr,&getStatckAddr,&getSize);
    pthread_create(&mAppIteraction_thread_id,&attr,mAppIteractionMsgProc_thread, (void *)this);

    /******/
    posix_memalign(&statckAddr,PageSize,THREAD_STACK_LEN_1M);
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr,statckAddr,THREAD_STACK_LEN_1M);
    pthread_attr_getstack(&attr,&getStatckAddr,&getSize);
    pthread_create(&mSysPeriodic_thread_id,&attr,mHysSysPeriodicProc_thread, (void *)this);

    /******/
    posix_memalign(&statckAddr,PageSize,THREAD_STACK_LEN_1M);
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr,statckAddr,THREAD_STACK_LEN_1M);
    pthread_attr_getstack(&attr,&getStatckAddr,&getSize);
    pthread_create(&mDevMngAppShm_thread_id,&attr,mActPeriodicProc_thread, (void *)this);

    /******/
    posix_memalign(&statckAddr,PageSize,THREAD_STACK_LEN_1M);
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr,statckAddr,THREAD_STACK_LEN_1M);
    pthread_attr_getstack(&attr,&getStatckAddr,&getSize);
    pthread_create(&mDevMngDevPeriodic_thread_id,&attr,mDevMngDevPeriodicProc_thread, (void *)this);

    /******/
    posix_memalign(&statckAddr,PageSize,THREAD_STACK_LEN_1M);
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr,statckAddr,THREAD_STACK_LEN_1M);
    pthread_attr_getstack(&attr,&getStatckAddr,&getSize);
    pthread_create(&mActPeriodic_thread_id,&attr,mDevMngAppShmProc_thread, (void *)this);

    /******/
    posix_memalign(&statckAddr,PageSize,THREAD_STACK_LEN_1M);
    pthread_attr_init(&attr);
    pthread_attr_setstack(&attr,statckAddr,THREAD_STACK_LEN_1M);
    pthread_attr_getstack(&attr,&getStatckAddr,&getSize);
    pthread_create(&mActSem_thread_id,&attr,mActSemProc_thread, (void *)this);

    /******/
    sleep(5);//因注册失败，所以增加这个延时
    pDeviceMng->Init();

    while(1)
    {
        sleep(10);
        #if 0//for test
            static int i = 0;
            DriverExtDevStatusType driverdevstate;
            driverdevstate.AppVersionHigh = 0x0103;
            driverdevstate.AppVersionLow = 0x0204;
            driverdevstate.BootVersionHigh = 0x0405;
            driverdevstate.BootVersionLow = 0x0607;
            driverdevstate.Status = 1;
            driverdevstate.Value = 10;
            pSystem->SetDriverExtDevState(1,4,1,driverdevstate);
            SensorInsertStateApi(1,4,1,1,50);
        #endif
        #if 0//for test
            SensorInsertStateApi(3,15,1,2,450);
            qDebug()<<"*************ControllerLogic,single 2 start ,1******times*is*****";
            SysCtrlInfoType  ctrlInfoTemp;
            ctrlInfoTemp.actionType = 0;
            ctrlInfoTemp.actionID   = 3;
            ctrlInfoTemp.ctrl.CtrlCmd  = 0;
            ctrlInfoTemp.ctrl.CtrlMode = 0;
            ctrlInfoTemp.ctrl.CtrlSourceAddr = 1;
            ctrlInfoTemp.ctrl.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ctrlInfoTemp.ctrl.UserID = 0;
            pActCtrl->InsCtrlBufQueue(&ctrlInfoTemp);
            sleep(90);
        #endif

        #if 0//for test
        //            qDebug()<<"*************ControllerLogic,assemble start ,1******times*is*****"<<i++;
            SysCtrlInfoType  ctrlInfoTemp;
            ctrlInfoTemp.actionType = 2;
            ctrlInfoTemp.actionID   = 2;
            ctrlInfoTemp.ctrl.CtrlCmd  = 0;
            ctrlInfoTemp.ctrl.CtrlMode = 1;
            ctrlInfoTemp.ctrl.CtrlSourceAddr = 1;
            ctrlInfoTemp.ctrl.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ctrlInfoTemp.ctrl.UserID = 0;
            ctrlInfoTemp.ParamLength = 0x07;
            ctrlInfoTemp.ParamBuf[0] = 111;
            ctrlInfoTemp.ParamBuf[1] = 105;
            ctrlInfoTemp.ParamBuf[2] = 0;
        //            qDebug()<<"*************ControllerLogic,assemble start ,2**********";
            pActCtrl->InsCtrlBufQueue(&ctrlInfoTemp);
            sleep(300);
        #endif

        #if 0//for test
            qDebug()<<"*************ControllerLogic,single start ,3***times*is*"<<i++;
            SysCtrlInfoType  ctrlInfoTemp;
            ctrlInfoTemp.actionType = 0;
            ctrlInfoTemp.actionID   = 3;
            ctrlInfoTemp.ctrl.CtrlCmd  = 0;
            ctrlInfoTemp.ctrl.CtrlMode = 1;
            ctrlInfoTemp.ctrl.CtrlSourceAddr = 1;
            ctrlInfoTemp.ctrl.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ctrlInfoTemp.ctrl.UserID = 0;
            pActCtrl->InsCtrlBufQueue(&ctrlInfoTemp);
            sleep(50);
        #endif

        #if 0//for test
            qDebug()<<"*************ControllerLogic,alarm start ,3***times*is*"<<i++;
            SysCtrlInfoType  ctrlInfoTemp;
            ctrlInfoTemp.actionType = 3;
            ctrlInfoTemp.actionID   = 1;
            ctrlInfoTemp.ctrl.CtrlCmd  = 0;
            ctrlInfoTemp.ctrl.CtrlMode = 1;
            ctrlInfoTemp.ctrl.CtrlSourceAddr = 1;
            ctrlInfoTemp.ctrl.CtrlSourceType = ActionCtrl_SourceType_CONTROLLER;
            ctrlInfoTemp.ctrl.UserID = 0;
            pActCtrl->InsCtrlBufQueue(&ctrlInfoTemp);
            sleep(10);

            qDebug()<<"*************ControllerLogic,alarm stop ,3***times*is*"<<i++;
            ctrlInfoTemp.actionType = 3;
            ctrlInfoTemp.actionID   = 1;
            ctrlInfoTemp.ctrl.CtrlCmd  = 1;
            ctrlInfoTemp.ctrl.CtrlMode = 1;
            ctrlInfoTemp.ctrl.CtrlSourceAddr = 1;
            ctrlInfoTemp.ctrl.CtrlSourceType = ActionCtrl_SourceType_CONTROLLER;
            ctrlInfoTemp.ctrl.UserID = 0;
            pActCtrl->InsCtrlBufQueue(&ctrlInfoTemp);
            sleep(10);
        #endif
    }
}

void *mHysSysPeriodicProc_thread(void *arg)
{
    ControllerLogic *pThis = static_cast<ControllerLogic *>(arg);
    pThis->pSystem->HysSystemPeriodicProc();
    return (void*)0;

}
void *mActPeriodicProc_thread(void *arg)
{
    ControllerLogic *pThis = static_cast<ControllerLogic *>(arg);
    pThis->pActCtrl->ActionCtrlPeriodRun();
    return (void*)0;

}

void *mActSemProc_thread(void *arg)
{
    ControllerLogic *pThis = static_cast<ControllerLogic *>(arg);
    pThis->pActCtrl->ActionCtrlSemProc();
    return (void*)0;
}

void *mDevMngAppShmProc_thread(void *arg)
{
    ControllerLogic *pThis = static_cast<ControllerLogic *>(arg);
    pThis->pDeviceMng->HysSystemAppRecvShmProc();
    return (void*)0;
}

void *mDevMngDevPeriodicProc_thread(void *arg)
{
    ControllerLogic *pThis = static_cast<ControllerLogic *>(arg);
    pThis->pDeviceMng->HysSystemDevPeriodicProc();
    return (void*)0;
}

void *mAppIteractionMsgProc_thread(void *arg)
{
    ControllerLogic *pThis = static_cast<ControllerLogic *>(arg);
    pThis->pAppIteractionMng->ReceiveMsgProc();
    return (void*)0;
}
