#ifndef TIMER_H
#define TIMER_H

/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include <stdint.h>

/*********************************************************************************
* 									DEFINES
*********************************************************************************/
//最大允许创建的定时器个数
#define TIMEREVENT   10
/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/
//定时器超时回调函数指针
typedef void (*BspTimerCallback)(void);
//工作模式
typedef enum{
    TIMER_NOUSE = 0,
    TIMER_STOP  = 1,
    TIMER_START = 2,
}Mode;
//定时器数据结构
typedef struct
{
    uint16_t     id;                 //定时器id
    uint32_t    timer;              //计时器
    Mode        mode;               //工作模式
    uint8_t     count;              //次数
    uint32_t    backTimer;          //定时器备份
    BspTimerCallback    Callback;   //超时回调
} Util_Timer;

/*********************************************************************************
* 									CLASS
*********************************************************************************/
class Timer
{
public:
    static Timer& getInstance()
    {
        static Timer instance; // 唯一实例
        return instance;
    }
    uint16_t addTimerEvent(uint32_t internal_value, uint8_t count, BspTimerCallback  Callback); //添加一个定时器
    void startTimerEvent(uint16_t id);      //启动定时器
    void stopTimerEvent(uint16_t id);       //停止定时器
    void deleteTimerEvent(uint16_t id);     //删除定时器
    void Util_Timer_Task();                 //定时器周期人物
private:
    Timer();
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Util_Timer Utiltimer[TIMEREVENT];       //定时器数据
};

#endif // TIMER_H
