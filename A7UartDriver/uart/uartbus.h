#ifndef UARTBUS_H
#define UARTBUS_H
/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "uartdrive.h"
#include "mutex/mutex.h"
#include "crc/crc_ccitt.h"
#include "sem/sem.h"

/*********************************************************************************
* 									DEFINES
*********************************************************************************/
#define  UART_TX_QUEUE_NUM		 50        //链表大小
#define  UART_RX_QUEUE_NUM		 50
#define  UART_SENT_QUEUE_NUM	 50
#define UARTFRAME_HEAD_LEN       5          //帧头长度
#define UARTFRAME_CRC_LEN        2          //CRC长度
#define UARTFRAME_MAX_LEN        (USART_DRIVER_RX_SIZE - UARTFRAME_HEAD_LEN - UARTFRAME_CRC_LEN)        //帧数据最大数据长度 = 驱动层长度-帧头-crc

/*********************************************************************************
* 									DECLARATIONS
*********************************************************************************/
typedef void (*UartOverTimeFrameFunc)(void* pframe);	//超时函数指针

//uart发送控制
typedef struct
{

    __uint16_t 		count;				//发送次数
    __uint16_t      u16Interval;		//发送间隔
    __uint16_t      u16CurInterval;		//发送间隔备份
    __uint16_t 		Overtime;			//超时时间
    __uint16_t 		ack;				//应答标志
    UartOverTimeFrameFunc	OtCbFunc;	//超时回调
} UartBusHead;
//帧结构
typedef  struct
{
    __uint8_t  Address;       //地址
    __uint8_t  Serial;        //流水
    __uint8_t  FrameType;     //֡帧类型
    __uint8_t  ACK;           //应答
    __uint8_t  u8DLC;         //数据长度
    char  SendBuf[UARTFRAME_MAX_LEN];      //数据
    __uint16_t  CRC;            //crc
}UartFrame;
//发送链表结构体
typedef struct
{
    UartFrame     	TrsFrame;
    UartBusHead     TraBusHead;
    void            *pNext;
} stUartTrsQueue;
//接受链表结构体
typedef struct
{
    UartFrame   RxFrame;
    void        *pNext;
} stUartRxQueue;
//uart总线数据
typedef struct {
    //发送链表
    stUartTrsQueue    UartTrs[UART_TX_QUEUE_NUM];
    __uint16_t        u16UartTrsCnt;
    stUartTrsQueue*   pUartTrsHead;
    stUartTrsQueue*   pUartTrsEnd;
    //接收链表
    stUartRxQueue     UartRev[UART_RX_QUEUE_NUM];
    __uint16_t        u16UartRevCnt;
    stUartRxQueue*    pUartRevHead;
    stUartRxQueue*    pUartRevEnd;
    //发完成链表
    stUartTrsQueue    UartSent[UART_SENT_QUEUE_NUM];
    __uint16_t        u16UartSentCnt;
    stUartTrsQueue*   pUartSentRead;
    stUartTrsQueue*   pUartSentWrite;
} UARTBUS_DATA;
//总线数据处理方法
typedef enum
{
	UARTBUS_MSG_InsUartSentQueue,		//插入发完成链表
	UARTBUS_MSG_DelUartSentQueue,		//删除发完成链表
	UARTBUS_MSG_CheckUartSentInterval,	//检测发完成链表
	UARTBUS_MSG_InsUartTrsQueue,		//插入发送链表
	UARTBUS_MSG_DelUartTrsQueue,		//删除发送链表
	UARTBUS_MSG_GetUartRevQueue,		//获取接收链表
	
	UARTBUS_MSG_MAX
}UARTBUS_MSG;

/*********************************************************************************
* 									CLASS
*********************************************************************************/
//串口总线数据处理类
class UartBus : public UartDrive
{
public:
    UartBus(const char*DriveName,int Baudrate);
    ~UartBus();
    char UartBusDealFunc(UARTBUS_MSG msg,void * pData);     //总线数据处理
    char SendDataDealApi(__uint8_t imd, __uint8_t ack, char *pdata,__uint16_t Length,__uint8_t add = 3);  //bus层对应用层发送接口
    char RecievedDataDealApi(UartFrame *pframe);              //bus层对应用层接收接口
    char UartSendAckApi(UartFrame * pframe, char* pdata,unsigned short len,__uint8_t addr = 3);  //bus层对应用层应答接口
    virtual char UartTrsProtocolDeal(__uint8_t funcid ,stUartTrsQueue *pframe) = 0;      //应用层对bus层发送协议处理接口
    virtual char UartRecvProtocolDeal(UartFrame *pframe) = 0;   //应用层对bus层接收协议处理接口
    virtual char Recived(char *pdata,unsigned short *Length);   //bus层对驱动层接收接口
    virtual char Send(char *pdata,unsigned short Length);       //bus层对驱动层发送接口
    friend void Crc16Ccitt(unsigned char *u8Buf, unsigned int u32Len, unsigned short *u16CheckOld);

    void RxTaskSendMsg(void);       //接收线程消息
    int RxTaskWaitMsg(void);        //接收线程消息
    void TxTaskSendMsg(void);       //发送线程消息
    int TxTaskWaitMsg(void);        //接收线程消息
    static void *RecievedThreadFunc(void *arg);        //接收线程处理函数
    static void *SendThreadFunc(void *arg);            //发送线程处理函数
    static void *CycleMngThreadFunc(void *arg);        //周期处理线程函数
    char UartBus_InsertRevBuf(char *tempdata, __uint16_t templen);              //将数据插入接收链表
    void ClearTotalTTL(void);                   //流水号重置
    void AddTotalTTL(void);                     //流水号自增
    inline __uint8_t GetTotalTTL(void);         //获取流水号
protected:
    char UartBusDataInit(void);                 //总线数据初始化
    __uint16_t  AddUartFrameCRC(stUartTrsQueue *sendframe);                     //计算校验
    char UartBus_GetRevBuf(UARTBUS_DATA * pUartBus, UartFrame *RevUart);        //从接收链表中获取数据
    char InsUartTrsQueue( UARTBUS_DATA * pUartBus, stUartTrsQueue *SentUart);   //插入发送链表
    char DelUartTrsQueue(UARTBUS_DATA * pUartBus);                               //从发送链表中删除
    char InsUartSentQueue(UARTBUS_DATA *pUartBus, stUartTrsQueue *SentUart);     //数据插入发送完成链表
    char DelUartSentQueue(__uint16_t TTL,UARTBUS_DATA * pUartBus);               //删除发完成链表数据
    char CheckUartSentInterval(__uint16_t delay,UARTBUS_DATA * pUartBus);        //周期检测发送完成链表
    stUartTrsQueue * CheckUartSentQueue(__uint8_t imd,UARTBUS_DATA * pUartBus);  //通过功能码查找链表中的数据
    char CheckFrameIMD(UartFrame * frame,__uint8_t imd);        //查找功能码所在的帧
    void UartTrsProc(void);                                     //发送处理
public:
    pthread_t RecvPthreadID;            //接收线程id
    pthread_t SendPthreadID;            //发送线程id
    pthread_t CyclePthreadID;           //周期线程id
private:
    UARTBUS_DATA  *UartBusDev;       //总线数据
    __uint8_t    TotalTTL;          //TTL
    unsigned char OldSN;
    semt *TxSem;         //发送信号量
    semt *RxSem;         //接收信号量
    mutx *TxMutx;        //发送互斥锁
    mutx *RxMutx;        //接收互斥锁
    mutx *SentMutx;      //发完成互斥锁
};
#endif // UARTBUS_H
