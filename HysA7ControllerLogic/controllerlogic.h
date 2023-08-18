#ifndef CONTROLLERLOGIC_H
#define CONTROLLERLOGIC_H

#include <QObject>
#include "Action/actionctrl.h"
#include "Action/alarm.h"
#include "HysDeviceMng/hysdevicemng.h"
#include "HysSystem/hyssystem.h"
#include "HysAppInteraction/hysappinteraction.h"
#include "libhysa7parammng.h"

/******/
class ControllerLogic : public QObject
{
    Q_OBJECT
public:
    explicit ControllerLogic(QObject *parent = nullptr);
    ~ControllerLogic();

private:
    int Init(void);

public:
    HysSystem *pSystem;
    ActionCtrl *pActCtrl;
    HysDeviceMng *pDeviceMng;
    HysAppInteraction *pAppIteractionMng;

private:
    pthread_t mSysPeriodic_thread_id;

    pthread_t mActPeriodic_thread_id;
    pthread_t mActSem_thread_id;

    pthread_t mDevMngAppShm_thread_id;
    pthread_t mDevMngDevPeriodic_thread_id;
    pthread_t mAppIteraction_thread_id;

private:

signals:

public slots:

};

#endif // CONTROLLERLOGIC_H
