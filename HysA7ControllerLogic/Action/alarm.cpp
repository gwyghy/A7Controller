#include "alarm.h"
#include "string.h"
#include <QDebug>

/****/
Alarm *Alarm::that = NULL;

/****/
AlarmModuleType AlarmModule;//模型定义
AlarmCtrlBufType    AlarmCtrlMng;//存放具体处理命令的数组
u_int16_t AlarmAllStopFlag;//停止命令或标志

/****/
Alarm::Alarm()
{
    AlarmAllStopFlag = 0;
}

Alarm::~Alarm()
{

}

Alarm *Alarm::getInston()
{
    if( that == NULL)
    {
        that = new Alarm();
    }
    return that;
}

/*****报警模型中所使用的相关参数的初始化*****/
void Alarm::AlarmCtrlBufInit(void)
{
    memset(AlarmCtrlMng.buf, 0 ,sizeof(AlarmCtrlMng.buf));
    AlarmCtrlMng.Cnt=0;
}

/***除语音报警外，其他报警方式相关参数初始化***/
void Alarm::AlarmModuleConfig(u_int16_t type,u_int16_t DriverId,\
                              u_int16_t DeviceId,u_int16_t PointId)
{
    if(type >=ALARM_UNIT_PHONE)
        return;

    AlarmModule.alarmDev[type].point.driver = DriverId;
    AlarmModule.alarmDev[type].point.device = DeviceId;
    AlarmModule.alarmDev[type].point.point = PointId;
}

/***语音报警相关参数初始化***/
void Alarm::AlarmModuleConfig_phone(u_int16_t PhoneId)
{
    AlarmModule.alarmDev[ALARM_UNIT_PHONE].phoneAddr = PhoneId;
}

/***报警模型的相关处理函数，声/光报警相关***/
/**point：点号
 * phoneAddr：电话编号
 * pdata：数据
 * alarmMode：报警方式
 * beepflag：蜂鸣器是否需要同时控制
**/
u_int16_t alarmFunc(PointIDType point,u_int16_t phoneAddr,void *pdata,u_int16_t alarmMode,u_int16_t beepflag)
{
    u_int16_t ret = 0;
    u_int16_t buzzermode = SELF_IO_POINT_BUZZER_MODE_NONE;

    if(pdata ==nullptr)
        return 0;

    u_int16_t ctrlcmd = *((u_int16_t *)pdata);//0停1启
    u_int16_t ctrlmode = ALARM_BLUE_RED_NEVER;
    if(ctrlcmd == BaseCtrlType_STARTUP)
        ;    //tode set alarm param

    if(alarmMode == ALARM_MODE_1)
    {
        if(AlarmModule.alarmDev[ALARM_UNIT_SOUND].enable != EnableType_DISABLE)
        {
            if(ctrlcmd == BaseCtrlType_STARTUP)
                ctrlmode = ALARM_RED_ALWAYS;
            else
                ctrlmode = ALARM_BLUE_RED_NEVER;
            ret = DevCtrlInquire(point.driver,\
                           point.device, 1,\
                           point.point, (u_int8_t *)&ctrlmode,0x02);
        }
    }
    else if(alarmMode == ALARM_MODE_2)
    {
        if(AlarmModule.alarmDev[ALARM_UNIT_SOUND].enable != EnableType_DISABLE)
        {
            if(ctrlcmd == BaseCtrlType_STARTUP)
                ctrlmode = ALARM_RED20_BLUE20;
            else
                ctrlmode = ALARM_BLUE_RED_NEVER;
            ret = DevCtrlInquire(point.driver,\
                           point.device,1,\
                           point.point, (u_int8_t *)&ctrlmode, 0x02);
        }
    }
    else
    {
        if(AlarmModule.alarmDev[ALARM_UNIT_SOUND].enable != EnableType_DISABLE)
            ret = DevCtrlInquire(point.driver,\
                                 point.device,1,\
                                 point.point,(u_int8_t *)&ctrlmode, 0x02);
    }

    if((*((u_int16_t *)pdata) == BaseCtrlType_STARTUP)||(beepflag))
    {
        buzzermode = *((u_int16_t *)pdata);
        if(*((u_int16_t *)pdata) == BaseCtrlType_STARTUP)
            buzzermode = SELF_IO_POINT_BUZZER_AUTO_ACTION;
        else
            buzzermode = SELF_IO_POINT_BUZZER_MODE_NONE;
        ret = DevCtrlInquire(DRIVER_IO,\
                        EXTDEV_TYPE_SELF_IO,0x01,\
                        SELF_IO_POINT_BUZZER,(u_int8_t *)&buzzermode,0x02);
    }
    /*****/
    if(ret == 0)
        return 1;
    else
        return 0;
}

/***报警模型的相关处理函数，语音报警相关***/
u_int16_t alarmFunc_phone(PointIDType point,u_int16_t phoneAddr,void *pdata,u_int16_t alarmMode,u_int16_t beepflag)
{
    return 0;//暂无此功能。因此返回。2023.07.18

//    u_int16_t ret;

//    ret = DevCtrlPhoneInquire(phoneAddr,pdata,0x02);//0x02为暂定长度
//    return ret;
}

/***报警模型的初始化，所有相关均初始化*****/
void Alarm::AlarmModuleInit(void)
{
    u_int16_t i;

    memset(&AlarmModule,0,sizeof(AlarmModuleType));
    for(i=0;i<ALARM_UNIT_PHONE;i++)
        AlarmModule.alarmDev[i].alarmFunc = alarmFunc;
    /**暂不实现**/
    AlarmModule.alarmDev[ALARM_UNIT_PHONE].alarmFunc = alarmFunc_phone;

    AlarmModule.alarmDev[ALARM_UNIT_SOUND].enable = EnableType_ENABLE;
    AlarmModule.alarmDev[ALARM_UNIT_LIGHT].enable = EnableType_ENABLE;
    AlarmModuleConfig(ALARM_UNIT_LIGHT,DRIVER_CAN1,EXTDEV_TYPE_ALARM,ALARM_POINT_LED);
    AlarmModuleConfig(ALARM_UNIT_SOUND,DRIVER_CAN1,EXTDEV_TYPE_ALARM,ALARM_POINT_BEEP);
    AlarmCtrlBufInit();
}

/***设置报警模型的禁止及允许***/
u_int16_t Alarm::AlarmModuleEnable(EnableType enable)
{
    u_int16_t i;
    u_int16_t ctrlmode;
    if(enable == EnableType_DISABLE)
    {
        for(i=0;i<ALARM_UNIT_PHONE;i++)
        {
            if(AlarmModule.alarmDev[i].state != BaseRunType_RUN)
                continue;
            ctrlmode = ALARM_MODE_NONE;
            DevCtrlInquire(AlarmModule.alarmDev[i].point.driver,\
                           AlarmModule.alarmDev[i].point.device,1,\
                           AlarmModule.alarmDev[i].point.point,(u_int8_t *)&ctrlmode, 0x02);
        }
    }
    for(i=0;i<ALARM_UNIT_PHONE;i++)
        AlarmModule.alarmDev[i].enable = enable;

    return 1;
}

/***设置语音报警模型的禁止及允许***/
u_int16_t Alarm::AlarmModuleEnable_phone(EnableType enable)
{
    AlarmModule.alarmDev[ALARM_UNIT_PHONE].enable = enable;
    return 1;
}

/***加入 存放具体处理命令的数组*******/
u_int16_t Alarm::InsAlarmCtrlBufQueue(AlarmCtrlType *pdata,AlarmCtrlType *pmode)
{
    int i;
    u_int16_t  flag=0;
    u_int16_t  save=0xff;
    u_int16_t  insert=0;

    pmode->eventMode =ALARM_MODE_NONE;
    if(AlarmCtrlMng.Cnt <ALARMCTRL_BUF_LEN)
    {
        for(i=0;i<ALARMCTRL_BUF_LEN;i++)
        {
            if(AlarmCtrlMng.buf[i].use )
            {
                if((AlarmCtrlMng.buf[i].eventType == pdata->eventType)&&
                  (AlarmCtrlMng.buf[i].eventID   == pdata->eventID)
                  /*&&(AlarmCtrlMng.buf[i].eventMode == pdata->eventMode)*/)
                {
                    flag =1;
                    break;
                }
            }
        }

        if(flag)
            return 0;

        for(i=0;i<ALARMCTRL_BUF_LEN;i++)
        {
            if(AlarmCtrlMng.buf[i].use ==0)
            {
                AlarmCtrlMng.buf[i].eventType = pdata->eventType;
                AlarmCtrlMng.buf[i].eventID   = pdata->eventID;
                AlarmCtrlMng.buf[i].eventMode = pdata->eventMode;
                AlarmCtrlMng.buf[i].alarmTime = pdata->alarmTime*LOGIC_TIME_MS;
                AlarmCtrlMng.buf[i].use =1;
                AlarmCtrlMng.Cnt ++;
                insert = i;
                break;
            }
        }

        for(i=0;i<ALARMCTRL_BUF_LEN;i++)
        {
            if(AlarmCtrlMng.buf[i].use )
            {
                if(AlarmCtrlMng.buf[i].eventMode == ALARM_MODE_2)
                {
                    memcpy(pmode,&AlarmCtrlMng.buf[i],sizeof(AlarmCtrlType));
                    return 1;
                }
                else if(AlarmCtrlMng.buf[i].eventMode == ALARM_MODE_1)
                {
                    save = i;
                }
            }
        }
        if(save!=0xff)
        {
            memcpy(pmode,&AlarmCtrlMng.buf[save],sizeof(AlarmCtrlType));
            return (save+1);
        }
        else
            return (insert+1);
    }
    return 0;
}
/***删除 存放具体处理命令的数组*******/
u_int16_t Alarm::DelAlarmCtrlBufQueue(AlarmCtrlType *pdata,AlarmCtrlType *pmode)
{
    int i;
    u_int16_t  save=0xff;

    pmode->eventMode =ALARM_MODE_NONE;
    if(AlarmCtrlMng.Cnt)
    {
        for(i=0;i<ALARMCTRL_BUF_LEN;i++)
        {
            if(AlarmCtrlMng.buf[i].use )
            {
                if((AlarmCtrlMng.buf[i].eventType == pdata->eventType)&&
                  (AlarmCtrlMng.buf[i].eventID   == pdata->eventID))
                {
                    AlarmCtrlMng.buf[i].eventType = 0;
                    AlarmCtrlMng.buf[i].eventID   = 0;
                    AlarmCtrlMng.buf[i].use =0;
                    AlarmCtrlMng.buf[i].eventMode = 0;
                    AlarmCtrlMng.buf[i].alarmTime = 0;
                    AlarmCtrlMng.Cnt --;
                    break;
                }
            }
        }

        for(i=0;i<ALARMCTRL_BUF_LEN;i++)
        {
            if(AlarmCtrlMng.buf[i].use )
            {
                if(AlarmCtrlMng.buf[i].eventMode == ALARM_MODE_2)
                {
                    memcpy(pmode,&AlarmCtrlMng.buf[i],sizeof(AlarmCtrlType));
                    return (i+1);
                }
                else if(AlarmCtrlMng.buf[i].eventMode == ALARM_MODE_1)
                {
                    save = i;
                }
            }
        }
        if(save!=0xff)
        {
            memcpy(pmode,&AlarmCtrlMng.buf[save],sizeof(AlarmCtrlType));
            return (save+1);
        }
    }
    return 0;
}
/***清除 存放具体处理命令的数组*******/
u_int16_t Alarm::DelAllAlarmCtrlBufQueue(void)
{
    int i;

    if(AlarmCtrlMng.Cnt)
    {
        for(i=0;i<ALARMCTRL_BUF_LEN;i++)
        {
            if(AlarmCtrlMng.buf[i].use )
            {
                AlarmCtrlMng.buf[i].eventType = 0;
                AlarmCtrlMng.buf[i].eventID   = 0;
                AlarmCtrlMng.buf[i].use =0;
                AlarmCtrlMng.buf[i].eventMode = 0;
                AlarmCtrlMng.buf[i].alarmTime = 0;
            }
        }
        AlarmCtrlMng.Cnt =0;
    }
    return 1;
}

/***内部，进行具体报警命令处理的相关函数****/
u_int16_t Alarm::alarmCmdFunc(AlarmUnitType *pUnit,int alarmTime,BaseCtrlType cmd,\
                              u_int16_t mode,u_int16_t ctrlEventType,u_int16_t ctrlEventID,\
                              u_int16_t alarmMode)
{
    u_int16_t data[2],processMode;
    int processTime;
    u_int16_t ret=1;
    AlarmCtrlType alarmCtrl,saveCtrl;

    switch(cmd)
    {
        case BaseCtrlType_STARTUP:
        {
            alarmCtrl.eventType =ctrlEventType;
            alarmCtrl.eventID =ctrlEventID;
            alarmCtrl.eventMode =alarmMode;
            alarmCtrl.alarmTime =alarmTime*LOGIC_TIME_MS;
            if(pUnit == &AlarmModule.alarmDev[ALARM_UNIT_LIGHT])
            {
                ret =InsAlarmCtrlBufQueue(&alarmCtrl,&saveCtrl);
                if(ret)
                {
                    pUnit->AlarmEventType = AlarmCtrlMng.buf[ret-1].eventType;
                    pUnit->AlarmEventID = AlarmCtrlMng.buf[ret-1].eventID;
                    processMode =saveCtrl.eventMode;
                    processTime =saveCtrl.alarmTime;
                }
                else
                    break;
            }
            else
            {
                processMode =alarmMode;
                processTime =alarmTime*LOGIC_TIME_MS;
            }
            if(ret)
            {
                if((pUnit->state == BaseRunType_RUN)&&(pUnit->alarmMode !=processMode))
                {
                    data[0] = BaseCtrlType_STOP;
                    pUnit->alarmFunc(pUnit->point,pUnit->phoneAddr,data,0,0);
                }

                data[0] = BaseCtrlType_STARTUP;
                data[1] = mode;
                ret =pUnit->alarmFunc(pUnit->point,pUnit->phoneAddr,data,processMode,0);
                if(ret)
                {
                    pUnit->state = BaseRunType_RUN;
                    pUnit->alarmTime = processTime;
                    pUnit->ticks =0;
                    pUnit->alarmMode =processMode;
                }
            }
        }
        break;

        case BaseCtrlType_STOP:
        {
            if(pUnit->state != BaseRunType_RUN)
                return 0;

            alarmCtrl.eventType =ctrlEventType;
            alarmCtrl.eventID =ctrlEventID;
            alarmCtrl.eventMode =alarmMode;
            alarmCtrl.alarmTime =alarmTime*LOGIC_TIME_MS;
            ret = DelAlarmCtrlBufQueue(&alarmCtrl,&saveCtrl);
            if(AlarmCtrlMng.Cnt==0)
            {
                data[0] = BaseCtrlType_STOP;
                ret =pUnit->alarmFunc(pUnit->point,pUnit->phoneAddr,data,alarmMode,1);
                pUnit->state = BaseRunType_STOP;
                pUnit->alarmTime = 0;
                pUnit->ticks =0;
                pUnit->alarmMode = 0;
            }
            else if(ret>0)
            {
                if(pUnit->alarmMode !=saveCtrl.eventMode)
                {
                    data[0] = BaseCtrlType_STOP;
                    pUnit->alarmFunc(pUnit->point,pUnit->phoneAddr,data,0,0);

                    data[0] = BaseCtrlType_STARTUP;
                    data[1] = mode;
                    pUnit->AlarmEventType = AlarmCtrlMng.buf[ret-1].eventType;
                    pUnit->AlarmEventID = AlarmCtrlMng.buf[ret-1].eventID;
                    ret =pUnit->alarmFunc(pUnit->point,pUnit->phoneAddr,data,saveCtrl.eventMode,0);
                    if(ret)
                    {
                        pUnit->state = BaseRunType_RUN;
                        pUnit->alarmTime = saveCtrl.alarmTime;
                        pUnit->ticks =0;
                        pUnit->alarmMode =saveCtrl.eventMode;
                    }
                }
                else
                {
                    pUnit->alarmTime = saveCtrl.alarmTime;
                    pUnit->ticks =0;
                }
            }
        }
        break;

        default:
            break;
    }
    return ret;
}

/***对外部，进行报警项的相关控制****/
u_int16_t Alarm::CtrlAlarm(AlarmType	type,u_int16_t alarmTime,BaseCtrlType cmd,void *pdata,\
                           u_int16_t ctrlEventType,u_int16_t ctrlEventID,u_int16_t alarmMode)
{
    u_int16_t ret=0;

//    qDebug("CtrlAlarm,type id %d,alarmmode is %d,alarmTime is %d,ctrlEventType is %d,ctrlEventID is %d",type,alarmMode,alarmTime,ctrlEventType,ctrlEventID);
    if((pdata ==NULL)&&(cmd==BaseCtrlType_STARTUP))
        return 0;

    if(type ==AlarmType_Phone)
    {
        if(pdata == NULL)
            return 0;

        AlarmModule.alarmDev[ALARM_UNIT_PHONE].alarmFunc(AlarmModule.alarmDev[ALARM_UNIT_PHONE].point,AlarmModule.alarmDev[ALARM_UNIT_PHONE].phoneAddr,pdata,alarmMode,0);
        return 1;
    }
    else
    {

        if((type ==AlarmType_SoundLight )||(type ==AlarmType_Sound ))
        {
            ret = alarmCmdFunc(&AlarmModule.alarmDev[ALARM_UNIT_SOUND],alarmTime*LOGIC_TIME_MS,cmd,*((u_int16_t*)pdata),ctrlEventType,ctrlEventID,alarmMode);
        }
        if((type ==AlarmType_SoundLight )||(type ==AlarmType_Light ))
        {
            ret |= alarmCmdFunc(&AlarmModule.alarmDev[ALARM_UNIT_LIGHT],alarmTime*LOGIC_TIME_MS,cmd,*((u_int16_t*)pdata+1),ctrlEventType,ctrlEventID,alarmMode);
        }
        return ret;
    }
}

/***停止正在进行的报警操作***/
void Alarm::AlarmAllStop(void)
{
    AlarmAllStopFlag =1;
}

/***对报警模型进行周期性管理***/
void Alarm::AlarmMng(u_int16_t delay)
{
    u_int16_t i,j;
    u_int16_t data[2],ret;
    AlarmUnitType *pUnit;

    for(i=0;i<ALARM_UNIT_PHONE;i++)
    {
        if(AlarmModule.alarmDev[i].state == BaseRunType_RUN)
        {
            if(i !=ALARM_UNIT_LIGHT)
            {
                if(AlarmModule.alarmDev[i].alarmTime !=0)
                {
                    AlarmModule.alarmDev[i].ticks +=delay;
                    if(AlarmModule.alarmDev[i].ticks >= AlarmModule.alarmDev[i].alarmTime)
                    {
                        data[0] = BaseCtrlType_STOP;
                        ret =AlarmModule.alarmDev[i].alarmFunc(AlarmModule.alarmDev[i].point,AlarmModule.alarmDev[i].phoneAddr,&data[0],0,1);
                        if(ret)
                        {
                            AlarmModule.alarmDev[i].state = BaseRunType_STOP;
                            AlarmModule.alarmDev[i].alarmTime = 0;
                            AlarmModule.alarmDev[i].ticks =0;
                        }
                    }
                }
            }
            else
            {
                if(AlarmModule.alarmDev[i].alarmTime !=0)
                {
                    AlarmModule.alarmDev[i].ticks +=delay;
                    if(AlarmModule.alarmDev[i].ticks >= AlarmModule.alarmDev[i].alarmTime)
                    {
                        pUnit =&AlarmModule.alarmDev[i];
                        alarmCmdFunc(pUnit,pUnit->alarmTime,BaseCtrlType_STOP,0,pUnit->AlarmEventType,pUnit->AlarmEventID,pUnit->alarmMode);
                    }
                }

                for(j=0;j<ALARMCTRL_BUF_LEN;j++)
                {
                    if(AlarmCtrlMng.buf[j].use )
                    {
                        if(AlarmModule.alarmDev[i].alarmTime !=0)
                        {
                            if(AlarmCtrlMng.buf[j].alarmTime >delay)
                                AlarmCtrlMng.buf[j].alarmTime -= delay;
                            else
                            {
                                AlarmCtrlMng.buf[j].eventType = 0;
                                AlarmCtrlMng.buf[j].eventID   = 0;
                                AlarmCtrlMng.buf[j].use =0;
                                AlarmCtrlMng.buf[j].eventMode = 0;
                                AlarmCtrlMng.buf[j].alarmTime = 0;
                                AlarmCtrlMng.Cnt --;
                            }
                        }
                    }
                }
            }

            if(AlarmAllStopFlag)
            {
                data[0] = BaseCtrlType_STOP;
                ret =AlarmModule.alarmDev[i].alarmFunc(AlarmModule.alarmDev[i].point,AlarmModule.alarmDev[i].phoneAddr,&data[0],0,1);
                if(ret)
                {
                    AlarmModule.alarmDev[i].state = BaseRunType_STOP;
                    AlarmModule.alarmDev[i].alarmTime = 0;
                    AlarmModule.alarmDev[i].ticks =0;
                }

                if(i ==ALARM_UNIT_LIGHT)
                    DelAllAlarmCtrlBufQueue();
            }
        }
    }
    AlarmAllStopFlag =0;
}
