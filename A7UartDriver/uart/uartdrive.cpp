/********************************************************************************
* 文件名：	uartDriver.h
* 作者：
* 版本：   	V1.0
* 修改：
* 日期：    2021年11月2日
* 功能描述: 串口驱动
* 修改说明：
*       >>>>  在工程中的位置  <<<<
*             4-应用层
*			  3-逻辑层
*      		  2-服务层
*       >>>>  1-硬件驱动层		<<<<
*********************************************************************************
* @copy
* <h2><center>&copy; COPYRIGHT 天津华宁电子有限公司 研发中心 电液控</center></h2>
*********************************************************************************/

/*********************************************************************************
* 									INCLUDES
*********************************************************************************/
#include "uartdrive.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <qdebug.h>

/*********************************************************************************
* 									FUNCTION
*********************************************************************************/
/**********************************************************************************************************
*	函 数 名: UartDrive
*	功能说明: 构造函数
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
UartDrive::UartDrive(const char*Name,int Baud):DriveName(Name),Baudrate(Baud)
{
    UartDriveData = new sUartDriverDataType;
    DriveInit();
}
/**********************************************************************************************************
*	函 数 名: ～UartDrive
*	功能说明: 析构函数
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
UartDrive::~UartDrive()
{
    delete UartDriveData;
    this->Close();
}

/**********************************************************************************************************
*	函 数 名: DriveInit
*	功能说明: 底层数据初始化
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
void UartDrive::DriveInit()
{
    memset(&UartDriveData->UartBufTx[0],0,sizeof(sUartDriverDataType));
    ReadAddr = &UartDriveData->UartBufRx[0];
    SetTxBusy(false);
    ReadFlag.COM = 0;

    fd = Open(DriveName);
    if(fd <0)
    {
        qDebug("uart%s Open Error",DriveName);
        return;
    }
    if(Init(fd,Baudrate) < 0)
    {
        qDebug("uart%s init Error",DriveName);
        return;
    }

    qDebug("uart%s drive init finist",DriveName);
}

/**********************************************************************************************************
*	函 数 名: Init
*	功能说明: 设备初始化
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
int UartDrive::Init(int fd, int Baudrate)
{
    struct termios newtio,oldtio;

    if ( tcgetattr( fd,&oldtio) != 0)
    {
        perror("SetupSerial 1");
        return -1;
    }

    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    newtio.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    newtio.c_oflag  &= ~OPOST;   /*Output*/
    newtio.c_cflag |= CS8;      //8bit
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);
    newtio.c_cflag &= ~PARENB;  //N
    if(Baudrate == 115200)
    {
        cfsetispeed(&newtio, B115200);  //115200
        cfsetospeed(&newtio, B115200);
    }
    else
    {
        cfsetispeed(&newtio, B9600);  //9600
        cfsetospeed(&newtio, B9600);
    }


    newtio.c_cflag &= ~CSTOPB;      //1 stop


    newtio.c_cc[VMIN]  = 0;  /* 读数据时的最小字节数*/
    newtio.c_cc[VTIME] = 0; /* 等待第1个数据的时间:
                             * 比如VMIN设为10表示至少读到10个数据才返回,
                             * 但是没有数据总不能一直等吧? 可以设置VTIME(单位是10秒)
                             * 假设VTIME=1，表示:
                             *    10秒内一个数据都没有的话就返回
                             *    如果10秒内至少读到了1个字节，那就继续等待，完全读到VMIN个数据再返回
                             */

    tcflush(fd,TCIFLUSH);

    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }

    return 0;
}

/**********************************************************************************************************
*	函 数 名: Open
*	功能说明: 打开设备
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
int UartDrive::Open(const char *DriveName)
{
    fd = open(DriveName, O_RDWR|O_NOCTTY| O_NDELAY |O_EXCL);
    if (-1 == fd)
    {
        perror("uart open error");

        return(-1);
    }
    if(fcntl(fd, F_SETFL, FNDELAY) < 0) // 将串口设置为非阻塞模式
    {
        perror("fcntl false");
        return -1;
    }
//    if(fcntl(fd, F_SETFL, 0)<0) /* 设置串口为阻塞状态*/
//    {
//        perror("fcntl false");
//        return -1;
//    }

    return fd;
}
/**********************************************************************************************************
*	函 数 名: Read
*	功能说明: 串口设备读操作
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
char UartDrive::Read()
{
    int len ,i = 5;
    timeval time;
    long interval = 0;

    if(!ReadFlag.Bits.Stop)
    {
        gettimeofday(&time, NULL);
        while((len = read(fd, ReadAddr,USART_DRIVER_RX_SIZE)) && (i--))
        {
            ReadFlag.Bits.Begin = 1;
            if(UartDriveData->BufRxCount +len < USART_DRIVER_RX_SIZE)
            {
                UartDriveData->BufRxCount+=len;
                ReadAddr += len;
            }
            else
            {
                memset(&UartDriveData->UartBufRx[0],0,USART_DRIVER_RX_SIZE);
                UartDriveData->BufRxCount = 0;
                ReadAddr = &UartDriveData->UartBufRx[0];
                ReadFlag.Bits.Begin = 0;
                ReadFlag.Bits.Stop = 0;

                return 1;
            }
            gettimeofday(&timeLastRead, NULL);
        }
        if(len == 0)
        {
            if(ReadFlag.Bits.Begin == 0)
                return 1;
            //计算超时间隔
            interval = (time.tv_sec - timeLastRead.tv_sec)*1000 +  (time.tv_usec - timeLastRead.tv_usec)/1000;
            if((interval >= USART_RX_TX_INTERVAL)&&(ReadFlag.Bits.Begin))
            {
                //调用接收处理接口
                Recived(&UartDriveData->UartBufRx[0],&UartDriveData->BufRxCount);
                //清除缓存及标志
                memset(&UartDriveData->UartBufRx[0],0,USART_DRIVER_RX_SIZE);
                UartDriveData->BufRxCount = 0;
                ReadAddr = &UartDriveData->UartBufRx[0];
                ReadFlag.Bits.Begin = 0;
                ReadFlag.Bits.Stop = 0;
            }
        }
    }
    return 0;
}

/**********************************************************************************************************
*	函 数 名: Write
*	功能说明: 串口设备写操作
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
char UartDrive::Write()
{
    timeval time;
    long interval = 0;
    if(GetTxBusy() == false)
    {
        if(UartDriveData->BufTxCount)
        {
            //发送数据，获取当前时间，并设置设备忙
            if(write(fd,UartDriveData->UartBufTx,UartDriveData->BufTxCount) > 0)
            {
                gettimeofday(&timeLastWrite, NULL);
                SetTxBusy(true);
            }
        }
    }
    else
    {
        gettimeofday(&time, NULL);
        //计算超时间隔，超出发送间隔，设置设备空闲
          interval = (time.tv_sec - timeLastWrite.tv_sec)*1000 +  (time.tv_usec - timeLastWrite.tv_usec)/1000;
          if(interval >= USART_RX_TX_INTERVAL)
          {
              SetTxBusy(false);
              memset(UartDriveData->UartBufTx,0,USART_DRIVER_TX_SIZE);
              UartDriveData->BufTxCount = 0;
          }
    }

    return 0;
}
/**********************************************************************************************************
*	函 数 名: Close
*	功能说明: 串口设备关闭
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
void UartDrive::Close()
{
    close(fd);
}
/**********************************************************************************************************
*	函 数 名: SetTxBusy
*	功能说明: 设置总线发送状态
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
void UartDrive::SetTxBusy(bool Status)
{
    this->TxBusyStatus = Status;
}
/**********************************************************************************************************
*	函 数 名: GetTxBusy
*	功能说明: 获取总线发送状态
*	形    参：无
*	返 回 值: 无
**********************************************************************************************************/
bool UartDrive::GetTxBusy()
{
    return this->TxBusyStatus;
}

/**********************************************************************************************************
*	函 数 名: DriveThreadFunc
*	功能说明: 串口驱动层线程函数
*	形    参：Key类
*	返 回 值: 无
**********************************************************************************************************/
void *UartDrive::DriveThreadFunc(void *arg)
{
    UartDrive * self;
    self = static_cast<UartDrive *>(arg);
    while(1)
    {
        self->Read();
        self->Write();
        usleep(USART_RX_TX_INTERVAL*1000);
    }
    return nullptr;
}
