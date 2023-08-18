/********************************************************************************
* 文件名：	uartDriver.h
* 作者：       zzw
* 版本：   	V1.0
* 修改：
* 日期：    2023年05月10日
* 功能描述: 串口驱动
* 修改说明：
*       >>>>  在工程中的位置  <<<<
*                   3-应用层
*                   2-总线层
*       >>>>  1-硬件驱动层		<<<<
*********************************************************************************
* @copy
* <h2><center>&copy; COPYRIGHT 天津华宁电子有限公司 研发中心 电液控</center></h2>
*********************************************************************************/
#ifndef UARTDRIVE_H
#define UARTDRIVE_H

/*********************************************************************************
* 									INCLUDES
*********************************************************************************/
#include <pthread.h>
#include <time.h>
#include <termios.h>
#include <sys/time.h>
#include "sem/sem.h"
/*********************************************************************************
* 									DEFINES
*********************************************************************************/
//驱动层缓存大小
#define	 USART_DRIVER_RX_SIZE	   140
#define	 USART_DRIVER_TX_SIZE	   140
//分包间隔10ms
#define USART_RX_TX_INTERVAL        5
/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/
//uart状态
enum
{
    UART_IDLE =0,
    UART_BUSY,
};

//uart驱动层缓存
typedef struct
{
    char  	UartBufTx[USART_DRIVER_TX_SIZE];
    unsigned short 	BufTxCount;

    char  	UartBufRx[USART_DRIVER_RX_SIZE];
    unsigned short	BufRxCount;
}sUartDriverDataType;

//标志
typedef union
{
    unsigned char COM;
    struct
    {
        unsigned char Begin:4;
        unsigned char Stop:4;
    }Bits;
}Flag_Type;

/*********************************************************************************
* 									CLASS
*********************************************************************************/
//uart设备抽象
class UartDrive
{
public:
    UartDrive(const char *Name,int Baud);
    ~UartDrive();
    int Init(int fd,int Baudrate = 115200);     //设备初始化配置
    void DriveInit();                           //驱动数据初始化
    int Open(const char* DriveName);
    char Read(void);
    char Write(void);
    void Close();
    void SetTxBusy(bool Status);                                    //设置总线发送状态
    bool GetTxBusy(void);                                           //获取总线发送状态
    virtual char Recived(char *pdata,unsigned short *Length) = 0;   //对外接口
    virtual char Send(char *pdata,unsigned short Length) = 0;       //对外接口
    static void *DriveThreadFunc(void *arg);                        //底层线程函数
public:
    sUartDriverDataType *UartDriveData;     //底层数据缓存
    pthread_t Drivethreadid;               //线程id
private:
    const char *DriveName;                  //设备名
    const int Baudrate;                     //波特率
    timeval timeLastRead,timeLastWrite;     //上一次读写数据时间
    bool TxBusyStatus;                      //总线状态
    int fd;                                 //串口设备文件描述符
    Flag_Type ReadFlag;                     //读标志
    char * ReadAddr;                        //读数据地址 
};

#endif // UARTDRIVE_H
