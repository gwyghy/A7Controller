/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "timer.h"
#include <string.h>
/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/

/*********************************************************************************
* 									FUNCTION
*********************************************************************************/
/*******************************************************************************************
**函数名称：Timer
**函数作用：定时器构造函数
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
Timer::Timer()
{
    for (int i = 0; i < TIMEREVENT; ++i) {
        Utiltimer[i].mode = TIMER_NOUSE;
        Utiltimer[i].timer = 0;
        Utiltimer[i].count = 0;
        Utiltimer[i].backTimer = 0;
        Utiltimer[i].Callback = nullptr;
        Utiltimer[i].id = 0xff;
    }
}

/*******************************************************************************************
**函数名称：addTimerEvent
**函数作用：添加一个定时器
**函数参数：internal_value：定时时间（单位ms，最小单位10ms） count：次数（0xff表示无限次） Callback：超时回调
**函数输出：定时器id，0xff：失败
**注意事项：单位ms，最小10ms
*******************************************************************************************/
uint16_t Timer::addTimerEvent(uint32_t internal_value, uint8_t count, BspTimerCallback Callback)
{
    for(int i = 0;i < TIMEREVENT;i++)
    {
        if(Utiltimer[i].mode > TIMER_NOUSE)
            continue;

        Utiltimer[i].mode = TIMER_STOP;
        Utiltimer[i].timer = internal_value/10;
        Utiltimer[i].count = count;
        Utiltimer[i].backTimer = internal_value/10;
        Utiltimer[i].Callback = Callback;
        Utiltimer[i].id = i;
        return i;
    }
    return 0xff;
}

/*******************************************************************************************
**函数名称：startTimerEvent
**函数作用：启动定时器
**函数参数：id：要启动的定时器id
**函数输出：无
**注意事项：无
*******************************************************************************************/
void Timer::startTimerEvent(uint16_t id)
{
    if(id >= TIMEREVENT)
        return;
    Utiltimer[id].timer = Utiltimer[id].backTimer;
    Utiltimer[id].mode = TIMER_START;
}

/*******************************************************************************************
**函数名称：stopTimerEvent
**函数作用：停止定时器
**函数参数：id：要停止的定时器id
**函数输出：无
**注意事项：无
*******************************************************************************************/
void Timer::stopTimerEvent(uint16_t id)
{
    if(id >= TIMEREVENT)
        return;
    Utiltimer[id].mode = TIMER_STOP;
}

/*******************************************************************************************
**函数名称：deleteTimerEvent
**函数作用：删除定时器
**函数参数：id：定时器id
**函数输出：无
**注意事项：无
*******************************************************************************************/
void Timer::deleteTimerEvent(uint16_t id)
{
    if(id >= TIMEREVENT)
        return;
    Utiltimer[id].timer = 0;
    Utiltimer[id].mode = TIMER_NOUSE;
    Utiltimer[id].count = 0;
    Utiltimer[id].backTimer = 0;
    Utiltimer[id].id = 0xff;
}

/*******************************************************************************************
**函数名称：Util_Timer_Task
**函数作用：定时器周期任务
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void Timer::Util_Timer_Task()
{
    for(int i = 0;i < TIMEREVENT;i++)
    {
        if(Utiltimer[i].id == 0xff)
            continue;
        if(Utiltimer[i].mode == TIMER_START && Utiltimer[i].timer > 0)
        {
            Utiltimer[i].timer--;
            if(Utiltimer[i].timer == 0)
            {
                if(Utiltimer[i].count == 0XFF)
                {
                    Utiltimer[i].timer = Utiltimer[i].backTimer;
                }
                else if(Utiltimer[i].count != 0)
                {
                    Utiltimer[i].timer = Utiltimer[i].backTimer;
                    Utiltimer[i].count--;
                }
                if(Utiltimer[i].count == 0)
                    Utiltimer[i].mode = TIMER_STOP;
                //执行超时回调
                (*Utiltimer[i].Callback)();
            }
        }
    }
}
