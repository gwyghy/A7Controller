#ifndef ALARM_H
#define ALARM_H

/*********/
#include "./HysDeviceMng/devctrl.h"
#include "./HysDeviceMng/devalarm.h"
#include "./ScRunParamMng/paramglobaldefine.h"
#include "./ScRunParamMng/singleactionparam.h"

/*********/
#define  ALARMCTRL_BUF_LEN	40
#define  LOGIC_TIME_MS      100

/*------------------------param define----------------------*/
/****报警模型的处理方式****///注意，此处定义与本架蜂鸣器无关，蜂鸣器控制通过接口中的beepflag控制
enum
{
    ALARM_UNIT_SOUND=0,//声报警
    ALARM_UNIT_LIGHT,//光报警
    ALARM_UNIT_PHONE,//电话报警
    ALARM_UNIT_MAX
};

enum//有预警方式
{
    ALARM_MODE_NONE =0,
    ALARM_MODE_1 ,   //红灯常亮
    ALARM_MODE_2,    //红蓝闪烁
    ALARM_MODE_MAX
};

typedef enum
{
    AlarmType_Phone =0,
    AlarmType_SoundLight,
    AlarmType_Sound,
    AlarmType_Light,
    AlarmTypeMAX
}AlarmType;

/*------------------------state define----------------------*/


/*------------------------part module define----------------------*/
typedef u_int16_t (*alarmCBFunc)(PointIDType point,u_int16_t phoneAddr,void *pdata,u_int16_t alarmMode,u_int16_t beepflag);

typedef struct
{
    EnableType  enable;
    AlarmType	type;
    PointIDType point;
    u_int16_t	phoneAddr;
    int			alarmTime;
    int			ticks;
    BaseRunType state;
    u_int16_t   alarmMode;
    u_int16_t	AlarmEventType;
    u_int16_t	AlarmEventID;
    alarmCBFunc alarmFunc;
}AlarmUnitType;

typedef struct
{
    AlarmUnitType	alarmDev[ALARM_UNIT_MAX];
}AlarmModuleType;

typedef struct
{
    u_int16_t		  eventType;			//0:单动作 1：组合动作  2：成组动作
    u_int16_t		  eventID;
    u_int16_t         use;
    u_int16_t		  eventMode;            //报警方式
    int				  alarmTime;
}AlarmCtrlType;

typedef struct
{
    AlarmCtrlType    buf[ALARMCTRL_BUF_LEN];	//buf
    u_int16_t        Cnt;					    //项数
}AlarmCtrlBufType;

/********************/
class Alarm
{
public:
    Alarm();
    ~Alarm();

    static Alarm *getInston();
public:
    /***除语音报警外，其他报警方式相关参数初始化***/
    void AlarmModuleConfig(u_int16_t type,u_int16_t DriverId,u_int16_t DeviceId,u_int16_t PointId);
    /***语音报警相关参数初始化***/
    void AlarmModuleConfig_phone(u_int16_t PhoneId);
    /***报警模型的初始化，所有相关均初始化*****/
    void AlarmModuleInit(void);

    /***设置报警模型的禁止及允许***/
    u_int16_t AlarmModuleEnable(EnableType enable);
    /***设置语音报警模型的禁止及允许***/
    u_int16_t AlarmModuleEnable_phone(EnableType enable);
    /***对外部，进行报警项的相关控制****/
    u_int16_t CtrlAlarm(AlarmType	type,u_int16_t alarmTime,BaseCtrlType cmd,void *pdata,\
                        u_int16_t ctrlEventType,u_int16_t ctrlEventID,u_int16_t alarmMode);
    /***停止正在进行的报警操作***/
    void AlarmAllStop(void);
    /***对报警模型进行周期性管理***/
    void AlarmMng(u_int16_t delay);

private:
    /*****报警模型中所使用的相关参数的初始化*****/
    void AlarmCtrlBufInit(void);

    /***加入 存放具体处理命令的数组*******/
    u_int16_t InsAlarmCtrlBufQueue(AlarmCtrlType *pdata,AlarmCtrlType *pmode);
    /***删除 存放具体处理命令的数组*******/
    u_int16_t DelAlarmCtrlBufQueue(AlarmCtrlType *pdata,AlarmCtrlType *pmode);
    /***清除 存放具体处理命令的数组*******/
    u_int16_t DelAllAlarmCtrlBufQueue(void);
    /***内部，进行具体报警命令处理的相关函数****/
    u_int16_t alarmCmdFunc(AlarmUnitType *pUnit,int alarmTime,BaseCtrlType cmd,\
                                  u_int16_t mode,u_int16_t ctrlEventType,u_int16_t ctrlEventID,\
                                  u_int16_t alarmMode);
private:
    /****/
    static Alarm* that;
private:
};

#endif // ALARM_H
