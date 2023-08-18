/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "uartbus.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <qdebug.h>
/*********************************************************************************
* 									FUNCTION
*********************************************************************************/

/*******************************************************************************************
**函数名称：UartBus
**函数作用：构造函数
**函数参数：
**函数输出：无
**注意事项：无
*******************************************************************************************/
UartBus::UartBus(const char*DriveName,int Baudrate):UartDrive(DriveName,Baudrate),OldSN(0)
{
    UartBusDev = new UARTBUS_DATA;
    TxSem = new semt;         //发送信号量
    RxSem = new semt;         //接收信号量
    TxMutx = new mutx;        //发送互斥锁
    RxMutx = new mutx;        //接收互斥锁
    SentMutx = new mutx;      //发完成互斥锁
    UartBusDataInit();
}
/*******************************************************************************************
**函数名称：~UartBus
**函数作用：析构函数
**函数参数：
**函数输出：无
**注意事项：无
*******************************************************************************************/
UartBus::~UartBus()
{
    delete TxSem;
    delete RxSem;
    delete TxMutx;
    delete TxMutx;
    delete SentMutx;
    delete UartBusDev;
}

/*******************************************************************************************
**函数名称：UartBusDataInit
**函数作用：UartBus数据初始化
**函数参数：dev 设备名
**函数输出：无
**注意事项：无
*******************************************************************************************/
char UartBus::UartBusDataInit()
{
    int i;
    memset(UartBusDev->UartTrs,0 ,sizeof(UartBusDev->UartTrs));
    memset(UartBusDev->UartRev,0 ,sizeof(UartBusDev->UartRev));
    memset(UartBusDev->UartSent,0 ,sizeof(UartBusDev->UartSent));
    UartBusDev->u16UartTrsCnt = 0;
    UartBusDev->u16UartRevCnt = 0;
    UartBusDev->u16UartSentCnt = 0;
    //发完成链表
    UartBusDev->pUartSentRead = NULL;
    UartBusDev->pUartSentWrite =&UartBusDev->UartSent[0];
    for(i=0 ; i<UART_SENT_QUEUE_NUM - 1; i++)
    {
        UartBusDev->UartSent[i].pNext = &UartBusDev->UartSent[i+1];
    }
    UartBusDev->UartSent[UART_SENT_QUEUE_NUM-1].pNext = NULL;
    //发送链表
    UartBusDev->pUartTrsHead = &UartBusDev->UartTrs[0];
    UartBusDev->pUartTrsEnd =&UartBusDev->UartTrs[0];
    for(i=0 ; i<UART_TX_QUEUE_NUM - 1; i++)
    {
        UartBusDev->UartTrs[i].pNext = &UartBusDev->UartTrs[i+1];
    }
    UartBusDev->UartTrs[UART_TX_QUEUE_NUM-1].pNext = &UartBusDev->UartTrs[0];
    //接收链表
    UartBusDev->pUartRevHead = &UartBusDev->UartRev[0];
    UartBusDev->pUartRevEnd =&UartBusDev->UartRev[0];
    for(i=0 ; i<UART_RX_QUEUE_NUM - 1; i++)
    {
        UartBusDev->UartRev[i].pNext = &UartBusDev->UartRev[i+1];
    }
    UartBusDev->UartRev[UART_RX_QUEUE_NUM-1].pNext = &UartBusDev->UartRev[0];

    TotalTTL = 1;
    return 0;
}

/*******************************************************************************************
**函数名称：AddTotalTTL
**函数作用：更新流水号
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void UartBus::AddTotalTTL()
{
    this->TotalTTL++;
    this->TotalTTL %=255;
}
/*******************************************************************************************
**函数名称：ClearTotalTTL
**函数作用：重置流水号
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void UartBus::ClearTotalTTL()
{
    this->TotalTTL = 0;
}

/*******************************************************************************************
**函数名称：GetTotalTTL
**函数作用：获取流水号
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
__uint8_t UartBus::GetTotalTTL()
{
    return this->TotalTTL;
}

/*******************************************************************************************
**函数名称：AddUartFrameCRC
**函数作用：添加帧校验数据
**函数参数：sendframe：待发送的帧数据
**函数输出：Dev：设备 pframedata：数据 framelength：输出的数据长度
**注意事项：1：成功 0失败
*******************************************************************************************/
__uint16_t UartBus::AddUartFrameCRC(stUartTrsQueue *sendframe)
{
    __uint16_t i,Crc = 0;
    char tempdata[UARTFRAME_MAX_LEN];

//    TxMutx->lock();
    tempdata[0]=sendframe->TrsFrame.Address;
    tempdata[1]=sendframe->TrsFrame.Serial;
    tempdata[2]=sendframe->TrsFrame.FrameType;
    tempdata[3]=sendframe->TrsFrame.ACK;
    tempdata[4]=sendframe->TrsFrame.u8DLC;
    for(i = 0;i < sendframe->TrsFrame.u8DLC; i++)
        tempdata[i+5]= *(sendframe->TrsFrame.SendBuf+i);
    Crc16Ccitt(tempdata,static_cast<unsigned int >(sendframe->TrsFrame.u8DLC+UARTFRAME_HEAD_LEN),&Crc);
//    TxMutx->unlock();

    return Crc;
}
/*******************************************************************************************
**函数名称：UartBus_InsertRevBuf
**函数作用：将数据插入接收链表
**函数参数：selUart:设备名 tempdate:字节流数据 templen:数据长度
**函数输出：无
**注意事项：1：成功 0失败
*******************************************************************************************/
char UartBus::UartBus_InsertRevBuf(char *tempdata, __uint16_t templen)
{
    __uint16_t crc = 0,fcs = 0;
    UartFrame TempFrame;
    if((templen < UARTFRAME_HEAD_LEN)||(templen > USART_DRIVER_RX_SIZE))
    {
        qDebug("Uart drive data length error! length = %d",templen);
        return 0;
    }
    memcpy(&TempFrame.Address,tempdata,templen);
    if(TempFrame.u8DLC + UARTFRAME_HEAD_LEN +2 != templen)
    {
        qDebug("Uart Frame data length error! length = %d",TempFrame.u8DLC);
        return 0;
    }
    RxMutx->lock();
    Crc16Ccitt(tempdata,(templen -2),&crc);
    fcs = (((__uint16_t)*(tempdata+templen-2))<< 8)|((__uint16_t)*(tempdata+templen-1) & 0x00ff);
    if(crc !=fcs)
    {
        qDebug("Uart data crc check error");
        RxMutx->unlock();
        return 0;
    }

    if(UartBusDev->u16UartRevCnt < UART_RX_QUEUE_NUM)
    {
        memcpy(&UartBusDev->pUartRevEnd->RxFrame,&TempFrame,sizeof(UartFrame));
        UartBusDev->pUartRevEnd = (stUartRxQueue*)UartBusDev->pUartRevEnd->pNext;
        UartBusDev->u16UartRevCnt ++;
    }
    RxMutx->unlock();

    RxTaskSendMsg();
    return 1;
}

/*******************************************************************************************
**函数名称：UartBusDealFunc
**函数作用：总线数据处理
**函数参数：msg：处理方法 pData：参数
**函数输出：addr 设备地址
**注意事项：无
*******************************************************************************************/
char UartBus::UartBusDealFunc(UARTBUS_MSG msg, void *pData)
{
    __uint8_t temp = 1;
    if(msg > UARTBUS_MSG_MAX)
    {
        return 0;
    }

    switch(msg)
    {
        case UARTBUS_MSG_InsUartSentQueue:
            InsUartSentQueue(UartBusDev, (stUartTrsQueue *)pData);
            break;

        case UARTBUS_MSG_DelUartSentQueue:
            DelUartSentQueue(*(__uint16_t *)pData,UartBusDev);
            break;

        case UARTBUS_MSG_CheckUartSentInterval:
            CheckUartSentInterval(*(__uint16_t*) pData, UartBusDev);
            break;

        case UARTBUS_MSG_InsUartTrsQueue:
            InsUartTrsQueue(UartBusDev, (stUartTrsQueue *)pData);
            break;

        case UARTBUS_MSG_DelUartTrsQueue:
            DelUartTrsQueue(UartBusDev);
            break;

        case UARTBUS_MSG_GetUartRevQueue:
            temp = UartBus_GetRevBuf(UartBusDev, (UartFrame *)pData);
            break;

        default:
            break;
    }
    return temp;
}

/*******************************************************************************************
**函数名称：UartTrsProc
**函数作用：链表发送数据处理
**函数参数：无
**函数输出：无
**注意事项：周期处理
*******************************************************************************************/
void UartBus::UartTrsProc()
{
    UARTBUS_DATA * pUartBus = UartBusDev;
    char    TrsData[UARTFRAME_MAX_LEN];
    unsigned char i;
    unsigned short length;
    //发送链表空
    if(pUartBus ->u16UartTrsCnt == 0)
    {
        return ;
    }
    //底层发送驱动忙
    if(this->GetTxBusy() == true)
        return;
    TxMutx->lock();
    //将帧数据转换为字节流数据
    TrsData[0]=pUartBus ->pUartTrsHead ->TrsFrame.Address;
    TrsData[1]=pUartBus ->pUartTrsHead ->TrsFrame.Serial;
    TrsData[2]=pUartBus ->pUartTrsHead ->TrsFrame.FrameType;
    TrsData[3]=pUartBus ->pUartTrsHead ->TrsFrame.ACK;
    TrsData[4]=pUartBus ->pUartTrsHead ->TrsFrame.u8DLC;
    for(i=0; i<pUartBus ->pUartTrsHead ->TrsFrame.u8DLC;i++)
    {
        TrsData[UARTFRAME_HEAD_LEN +i] = pUartBus ->pUartTrsHead ->TrsFrame.SendBuf[i];
    }
    TrsData[UARTFRAME_HEAD_LEN + pUartBus ->pUartTrsHead ->TrsFrame.u8DLC] = (pUartBus ->pUartTrsHead ->TrsFrame.CRC & 0xff00) >> 8;
    TrsData[UARTFRAME_HEAD_LEN + pUartBus ->pUartTrsHead ->TrsFrame.u8DLC+1] = (pUartBus ->pUartTrsHead ->TrsFrame.CRC & 0x00ff);
    length = UARTFRAME_HEAD_LEN + pUartBus ->pUartTrsHead ->TrsFrame.u8DLC +2;
    TxMutx->unlock();
    //等待应答或重发
    if((pUartBus->pUartTrsHead->TraBusHead.ack ==1) ||(pUartBus->pUartTrsHead->TraBusHead.count != 0))
    {
        UartBusDealFunc(UARTBUS_MSG_InsUartSentQueue,pUartBus ->pUartTrsHead);
    }
    UartBusDealFunc(UARTBUS_MSG_DelUartTrsQueue,pUartBus);

    Send(TrsData,length);
}

/*******************************************************************************************
**函数名称：SendDataDealApi
**函数作用：数据发送接口
**函数参数：imd：功能码 ack：应答标志 pdata：数据 Length：长度
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char UartBus::SendDataDealApi(__uint8_t imd, __uint8_t ack, char *pdata, __uint16_t Length, __uint8_t add)
{
    stUartTrsQueue sendframe;
    //判断数据长度，防止缓存越界
    if(Length > UARTFRAME_MAX_LEN)
        return 1;

    this->UartTrsProtocolDeal(imd,&sendframe);
    sendframe.TrsFrame.Address =  add;
    sendframe.TrsFrame.Serial = this->GetTotalTTL();
    sendframe.TrsFrame.FrameType = imd;
    sendframe.TrsFrame.ACK = ack;
    sendframe.TrsFrame.u8DLC = Length;

    memcpy(sendframe.TrsFrame.SendBuf ,pdata,Length);
    sendframe.TrsFrame.CRC = AddUartFrameCRC(&sendframe);

    UartBusDealFunc(UARTBUS_MSG_InsUartTrsQueue,&sendframe);

    return 0;
}
/*******************************************************************************************
**函数名称：RecievedDataDealApi
**函数作用：数据接收处理接口
**函数参数：pframe：数据
**函数输出：无
**注意事项：无
*******************************************************************************************/
char UartBus::RecievedDataDealApi(UartFrame *pframe)
{
    //重复帧过滤
    if(OldSN == pframe->Serial)
    {
        qDebug("same uart SN error");
        return 1;
    }
    OldSN = pframe->Serial;
    //收到应答数据，删除重发
    if(pframe->ACK == 1)
    {
        UartBusDealFunc(UARTBUS_MSG_DelUartSentQueue,&pframe->FrameType);
    }
    //获取处理方法
    this->UartRecvProtocolDeal(pframe);

    return 0;
}

/*******************************************************************************************
**函数名称：SendDataDealApi
**函数作用：应答数据发送接口
**函数参数：imd：功能码 ack：应答标志 pdata：数据 Length：长度
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char UartBus::UartSendAckApi(UartFrame *pframe, char *pdata, unsigned short len, __uint8_t addr)
{
    (void) addr;     // 使用类型转换将未使用的参数转换为空操作，解决警告

    stUartTrsQueue sendframe;
     //判断数据长度，防止缓存越界
    if(len > UARTFRAME_MAX_LEN-UARTFRAME_HEAD_LEN)
        return 1;

    sendframe.TraBusHead.ack = 0;
    sendframe.TraBusHead.count = 0;
    sendframe.TraBusHead.u16CurInterval = 0;
    sendframe.TraBusHead.u16Interval = 0;
    sendframe.TraBusHead.Overtime = 0;
    sendframe.TraBusHead.OtCbFunc = NULL;
    //复制数据及帧头，重新计算crc
    memcpy(&sendframe.TrsFrame,pframe,sizeof(UartFrame));
    sendframe.TrsFrame.ACK =0;
    sendframe.TrsFrame.u8DLC = len;
    memcpy(sendframe.TrsFrame.SendBuf ,pdata,len);
    sendframe.TrsFrame.CRC = AddUartFrameCRC(&sendframe);

    UartBusDealFunc(UARTBUS_MSG_InsUartTrsQueue,&sendframe);

    return 0;
}

/*******************************************************************************************
**函数名称：Recived
**函数作用：虚函数，将底层数据插入接收数据链表
**函数参数：pdata：数据 Length：长度
**函数输出：无
**注意事项：0：成功 ！0失败
*******************************************************************************************/
char UartBus::Recived(char *pdata, unsigned short *Length)
{
    UartBus_InsertRevBuf(pdata,*Length);
    return 0;
}
/*******************************************************************************************
**函数名称：Send
**函数作用：虚函数，将发送链表数据插入底层缓存
**函数参数：pdata：数据 Length：长度
**函数输出：无
**注意事项：0：成功 ！0失败
*******************************************************************************************/
char UartBus::Send(char *pdata, unsigned short Length)
{
    memcpy(UartDriveData->UartBufTx,pdata,Length);
    UartDriveData->BufTxCount = Length;
    return 0;
}
/*******************************************************************************************
**函数名称：UartBus_GetRevBuf
**函数作用：从接收链表中获取数据
**函数参数：pUartBus：总线设备 RevUart：包数据
**函数输出：Dev：设备 pframedata：数据 framelength：输出的数据长度
**注意事项：1：成功 0失败
*******************************************************************************************/
char UartBus::UartBus_GetRevBuf(UARTBUS_DATA *pUartBus, UartFrame *RevUart)
{
    char res = 1;

    RxMutx->lock();
    if(pUartBus->u16UartRevCnt > 0)
    {
        memcpy(&RevUart->Address,&pUartBus->pUartRevHead->RxFrame.Address,sizeof(UartFrame));
        memset(&pUartBus->pUartRevHead->RxFrame.Address,0,sizeof(UartFrame));
        UartBusDev->pUartRevHead =( stUartRxQueue* )UartBusDev->pUartRevHead ->pNext;
        UartBusDev->u16UartRevCnt --;
    }
    else
    {
        res = 0;
    }
    RxMutx->unlock();

    return res;
}

/*******************************************************************************************
**函数名称：InsUartTrsQueue
**函数作用：插入发送链表
**函数参数：pUartBus：总线设备  SentUart：待发送链表数据
**函数输出：无
**注意事项：无
*******************************************************************************************/
char UartBus::InsUartTrsQueue(UARTBUS_DATA *pUartBus, stUartTrsQueue *SentUart)
{
    char res = 0;

    TxMutx->lock();
    if(pUartBus->u16UartTrsCnt < UART_TX_QUEUE_NUM)
    {
        pUartBus->pUartTrsEnd->TrsFrame = SentUart->TrsFrame;
//        memcpy(&pUartBus->pUartTrsEnd->TrsFrame.Address,&SentUart->TrsFrame.Address,sizeof(UartFrame));
        pUartBus->pUartTrsEnd->TraBusHead = SentUart->TraBusHead;
        pUartBus->pUartTrsEnd = (stUartTrsQueue*)(pUartBus->pUartTrsEnd->pNext);
        pUartBus ->u16UartTrsCnt ++;
        TxTaskSendMsg();
    }
    TxMutx->unlock();

    return res;
}

/*******************************************************************************************
**函数名称：DelUartTrsQueue
**函数作用：从发送链表中删除
**函数参数：pUartBus：总线设备  SentUart：待发送链表数据
**函数输出：无
**注意事项：无
*******************************************************************************************/
char UartBus::DelUartTrsQueue(UARTBUS_DATA *pUartBus)
{
    TxMutx->lock();
    if (pUartBus->u16UartTrsCnt)
    {
        pUartBus ->u16UartTrsCnt --;
        pUartBus ->pUartTrsHead = static_cast<stUartTrsQueue* >(pUartBus ->pUartTrsHead->pNext);
        memset(&pUartBus->pUartTrsHead->TrsFrame.Address,0,sizeof(UartFrame));
    }
    TxMutx->unlock();

    return 0;
}

/*******************************************************************************************
**函数名称：InsUartSentQueue
**函数作用：将数据插入发送完成链表
**函数参数：selUart:设备名 tempdate:字节流数据 templen:数据长度
**函数输出：Dev：设备 pframedata：数据 framelength：输出的数据长度
**注意事项：1：成功 0失败
*******************************************************************************************/
char UartBus::InsUartSentQueue(UARTBUS_DATA *pUartBus, stUartTrsQueue *SentUart)
{
    stUartTrsQueue *pUartSentTemp;

    SentMutx->lock();
    if(pUartBus->u16UartSentCnt < UART_SENT_QUEUE_NUM)
    {
        memcpy(&(pUartBus->pUartSentWrite->TrsFrame.Address),&(SentUart->TrsFrame.Address),sizeof(UartFrame));
        memcpy(&(pUartBus->pUartSentWrite->TraBusHead.count),&(SentUart->TraBusHead.count),sizeof(UartBusHead));
        pUartSentTemp = pUartBus->pUartSentWrite;
        pUartBus->pUartSentWrite = (stUartTrsQueue* )pUartBus->pUartSentWrite->pNext;
        //next向前找，先入后处理，后入先处理
        pUartSentTemp->pNext =  pUartBus->pUartSentRead;
        pUartBus->pUartSentRead = pUartSentTemp;
        pUartBus ->u16UartSentCnt ++;
    }
    SentMutx->unlock();

    return 0;
}
/*******************************************************************************************
**函数名称：DelUartSentQueue
**函数作用：删除发完成链表数据
**函数参数：imd：功能码 pUartBus：设备总线
**函数输出：ttl
**注意事项：无
*******************************************************************************************/
char UartBus::DelUartSentQueue(__uint16_t TTL, UARTBUS_DATA *pUartBus)
{
    stUartTrsQueue	*pUartSentDelTemp = NULL;
    stUartTrsQueue	*pUartSentTemp = pUartBus->pUartSentRead;

    SentMutx->unlock();
    pUartSentDelTemp = CheckUartSentQueue(TTL,pUartBus);
    if(pUartSentDelTemp != NULL)
    {
        if(pUartSentTemp == pUartSentDelTemp)
        {
            pUartBus->pUartSentRead = (stUartTrsQueue*)pUartBus->pUartSentRead ->pNext;
            pUartBus ->u16UartSentCnt --;
            pUartSentTemp->pNext = pUartBus ->pUartSentWrite;
            pUartBus ->pUartSentWrite = pUartSentTemp;
        }
        else
        {
            do
            {
                if(pUartSentTemp -> pNext == pUartSentDelTemp )
                {
                    pUartSentTemp -> pNext = pUartSentDelTemp->pNext;
                    pUartSentDelTemp->pNext = pUartBus ->pUartSentWrite;
                    pUartBus ->pUartSentWrite = pUartSentDelTemp;
                    pUartBus ->u16UartSentCnt --;
                    break;
                }
                pUartSentTemp = (stUartTrsQueue*)pUartSentTemp->pNext;
            }
            while(pUartSentTemp !=NULL);
        }
    }
    SentMutx->unlock();

    return 0;
}
/*******************************************************************************************
**函数名称：CheckUartSentInterval
**函数作用：检测发送完成链表
**函数参数：u8way：设备 delay：延时时间 pUartbus：总线设备
**函数输出：ttl
**注意事项：无
*******************************************************************************************/
char UartBus::CheckUartSentInterval( __uint16_t delay, UARTBUS_DATA *pUartBus)
{
    stUartTrsQueue    *pUartSentTemp ,*pUartSentProTemp;

    SentMutx->lock();
    //没数
    if(pUartBus ->u16UartSentCnt ==0)
    {
        SentMutx->unlock();
        return  1 ;
    }

    pUartSentProTemp = pUartBus->pUartSentRead;
    pUartSentTemp = (stUartTrsQueue  *)pUartSentProTemp ->pNext;
    //只有一个数
    if(pUartSentTemp == NULL)
    {
        if(pUartSentProTemp->TraBusHead.u16CurInterval >= delay)
        {
            pUartSentProTemp->TraBusHead.u16CurInterval -= delay;
        }
        else
        {
            pUartSentProTemp->TraBusHead.u16CurInterval = 0;
        }

        if(pUartSentProTemp->TraBusHead.u16CurInterval == 0)
        {
            if(pUartSentProTemp->TraBusHead.count > 0)
            {
                pUartSentProTemp->TraBusHead.count--;
                pUartSentProTemp->TraBusHead.u16CurInterval = pUartSentProTemp->TraBusHead.u16Interval;
                InsUartTrsQueue(pUartBus,pUartSentProTemp);
                //删链表
                memset(pUartSentProTemp->TrsFrame.SendBuf,0,sizeof(UartFrame));
                pUartBus->pUartSentRead = NULL;
                pUartSentProTemp->pNext = pUartBus ->pUartSentWrite;
                pUartBus ->pUartSentWrite = pUartSentProTemp;
                pUartBus ->u16UartSentCnt =0;
                //需要等应答
                if((pUartSentProTemp->TraBusHead.count == 0) &&(pUartSentProTemp->TraBusHead.ack == 1))
                {
                    pUartSentProTemp->TraBusHead.u16CurInterval = pUartSentProTemp->TraBusHead.Overtime;
                }
            }
            else
            {
                //超时回调非空的时候才能回调，否则会死机
                if(pUartSentProTemp->TraBusHead.OtCbFunc != NULL)
                    pUartSentProTemp->TraBusHead.OtCbFunc(&pUartSentProTemp->TrsFrame);
                //删链表
                memset(pUartSentProTemp->TrsFrame.SendBuf,0,sizeof(UartFrame));
                pUartBus->pUartSentRead = NULL;
                pUartSentProTemp->pNext = pUartBus ->pUartSentWrite;
                pUartBus ->pUartSentWrite = pUartSentProTemp;
                pUartBus ->u16UartSentCnt =0;
            }

        }
    }
    SentMutx->unlock();
    return  0;
}
/*******************************************************************************************
**函数名称：CheckUartSentQueue
**函数作用：通过功能码查找链表中的数据
**函数参数：imd：功能码 pUartBus：设备总线
**函数输出：ttl
**注意事项：无
*******************************************************************************************/
stUartTrsQueue *UartBus::CheckUartSentQueue(__uint8_t imd, UARTBUS_DATA *pUartBus)
    {
        stUartTrsQueue    *pUartSentTemp ,*pUartSentResTemp;

        pUartSentResTemp=NULL;
        pUartSentTemp = pUartBus->pUartSentRead;
        if(pUartBus ->u16UartSentCnt ==0)
        {
            return pUartSentResTemp;
        }
        //遍历发完成链表，找到功能码对应的数据
        do
        {
            if(this->CheckFrameIMD(&(pUartSentTemp->TrsFrame),imd))
            {
                pUartSentResTemp = pUartSentTemp;
                break;
            }
            pUartSentTemp = (stUartTrsQueue *)pUartSentTemp->pNext;
        }while(pUartSentTemp != NULL);

        return pUartSentResTemp;
    }

/*******************************************************************************************
**函数名称：CheckFrameIMD
**函数作用：查找功能码所在的帧
**函数参数：frame：帧：imd：功能码
**函数输出：ttl
**注意事项：无
*******************************************************************************************/
char UartBus::CheckFrameIMD(UartFrame *frame, __uint8_t imd)
{
    if(frame->FrameType == imd)
    {
        return 1;
    }

    return 0;
}

/*******************************************************************************************
**函数名称：RxTaskSendMsg
**函数作用：发送总线接受数据消息
**函数参数：无
**函数输出：信号量
**注意事项：无
*******************************************************************************************/
void UartBus::RxTaskSendMsg()
{
    RxSem->post_sem();
}

/*******************************************************************************************
**函数名称：RxTaskWaitMsg
**函数作用：等待总线接受数据消息
**函数参数：无
**函数输出：信号量
**注意事项：无
*******************************************************************************************/
int UartBus::RxTaskWaitMsg()
{
    return RxSem->wait_sem();
}

/*******************************************************************************************
**函数名称：TxTaskSendMsg
**函数作用：发送总线发送数据消息
**函数参数：无
**函数输出：发送数据信号量
**注意事项：无
*******************************************************************************************/
void UartBus::TxTaskSendMsg()
{
    TxSem->post_sem();
}

/*******************************************************************************************
**函数名称：TxTaskWaitMsg
**函数作用：等待总线发送数据消息
**函数参数：无
**函数输出：发送数据信号量
**注意事项：无
*******************************************************************************************/
int UartBus::TxTaskWaitMsg()
{
    return TxSem->wait_sem();
}

/*******************************************************************************************
**函数名称：RecievedThreadFunc
**函数作用：总线接收线程处理函数
**函数参数：Key类
**函数输出：无
**注意事项：无
*******************************************************************************************/
void *UartBus::RecievedThreadFunc(void *arg)
{
    int     err;
    UartFrame   RevBuf;
    UartBus * self;
    self = static_cast<UartBus*>(arg);

    while (1)
    {
        err =  self->RxTaskWaitMsg();
        if(err ==  0)
        {
            if(self->UartBusDealFunc(UARTBUS_MSG_GetUartRevQueue,&RevBuf))
            {
                self->RecievedDataDealApi(&RevBuf);
            }
        }
        else
            usleep(1000);
    }

    return nullptr;
}

/*******************************************************************************************
**函数名称：SendThreadFunc
**函数作用：总线发送线程处理函数
**函数参数：Key类
**函数输出：无
**注意事项：无
*******************************************************************************************/
void *UartBus::SendThreadFunc(void *arg)
{
    char err = 0;
    UartBus * self;
    self = static_cast<UartBus*>(arg);

    while (1)
    {
        err =  self->TxTaskWaitMsg();
        if(err == 0)
        {
            while(self->GetTxBusy() == UART_BUSY)
            {
                usleep(1000);
            }
            self->UartTrsProc();
        }
    }

    return nullptr;
}

/*******************************************************************************************
**函数名称：CycleMngThreadFunc
**函数作用：总线周期处理线程函数
**函数参数：Key类
**函数输出：无
**注意事项：无
*******************************************************************************************/
void *UartBus::CycleMngThreadFunc(void *arg)
{
    unsigned short delay;
    UartBus * self;
    self = (UartBus*)arg;

    while(1)
    {
        delay = 10;
        self->UartBusDealFunc(UARTBUS_MSG_CheckUartSentInterval,&delay);
        usleep(10000);
    }

    return nullptr;
}

