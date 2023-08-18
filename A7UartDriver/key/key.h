#ifndef KEY_H
#define KEY_H
/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "uart/uartbus.h"
#include "uart/UartDataType.h"
#include <QList>
#include "uinput_controller.h"
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include "sem/sem.h"

/*********************************************************************************
* 									DEFINES
*********************************************************************************/
//按键值宏定义
#define	UART_KEY_L1				0x00000001
#define	UART_KEY_L2				0x00000002
#define	UART_KEY_L3				0x00000004
#define	UART_KEY_R1				0x00000008
#define	UART_KEY_R2				0x00000010
#define	UART_KEY_R3				0x00000020

#define	UART_KEY_2				0x00000040
#define	UART_KEY_3				0x00000080
#define	UART_KEY_5				0x00000100
#define	UART_KEY_6				0x00000200
#define	UART_KEY_8				0x00000400
#define	UART_KEY_9				0x00000800

#define	UART_KEY_STOP			0x00001000
#define	UART_KEY_UP             0x00002000
#define	UART_KEY_DOWN			0x00004000

#define	UART_KEY_LEFT           0x00008000
#define	UART_KEY_RIGHT			0x00010000
#define	UART_KEY_START			0x00020000
#define	UART_KEY_0              0x00040000
#define	UART_KEY_WORK			0x00080000
#define	UART_KEY_MENU		    0x00100000

/*********************************************************************************
* 									DECLARATIONS
*********************************************************************************/
void KeyAppStart(void);
void RecvShmCallback_KeyLedCtrl(uint8_t point,uint8_t* data,int len);       //逻辑层指示灯控制
void RecvMsgCallbackKeyinit(uint8_t point,uint8_t* data,int len);           //初始化
void RecvMsgCallback_KeyFileDownLoad(uint8_t point,uint8_t* data,int len);  //程序更新
/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/
//帧类别枚举
enum
{
    HEARTBEAT,//心跳数据
    KEYVALUEANDSTATUS=1,//按键信息
    COMMUNICATIONLED,//LED信息
    NAVIGATIONLED,//LED导航
    SETINFORMAION,//设置信息
    UPDATAPROGRAM,//程序升级
    RSUPDATAPROGRAM,//程序更新
    RESTORATION,//复位

    KEY_FUNC_MAX,
};

//uart按键值与按键事件值的对应关系
typedef struct
{
     int UartKeyNum;
     int  KeEventData;
}KeyTurnAroundTab;

 /*********************************************************************************
 * 									CLASS
 *********************************************************************************/
class Key : public UartBus
{
public:
    Key(const char*Name,int Baud);
    ~Key();
    void ThreadCreat(void);                                                         //应用启动
    virtual char UartTrsProtocolDeal(__uint8_t funcid ,stUartTrsQueue *pframe);     //发送协议处理
    virtual char UartRecvProtocolDeal(UartFrame *pframe);                           //接收协议处理
    char SendProgrameUpdateProc(char* name);                                        //程序更新
    static void *ProgrameUpdataPro(void *arg);                                      //程序更新线程
protected:
    static char HeartRecvDealCb(UartFrame *pframe,void*arg);            //键盘心跳接收处理函数
    static char KeyNumRecvDealCb(UartFrame *pframe,void*arg);           //按键值接收处理函数
private:
    QList <DataProcType> KeyProtocolFunc;    //数据处理方法
public:
    pthread_t UpdatePthreadID;              //程序更新线程id
    UinputController *uinputInstance;        //uinput子系统
    semt *UpdateSem;                        //程序更新信号量
public:
    char AppVerson[4];                      //版本信息
    char OnlineStatus;                     //键盘板在线标志
    char Programename[100];                 //存放程序更新名
};

#endif // KEY_H
