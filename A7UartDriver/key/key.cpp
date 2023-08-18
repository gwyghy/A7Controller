/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "key.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <qdebug.h>
#include "ydshrdlib.h"
#include "timer/timer.h"
#include <fcntl.h>

/*********************************************************************************
* 									DEFINES
*********************************************************************************/
#define USART_SEND_BUF_SIZE  128        //程序更新缓存大小
/*********************************************************************************
* 									声明
*********************************************************************************/

/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/
//全局按键类对象
Key *key = nullptr;
//设备名
#ifdef PLATFORM_TYPE            //arm版
constexpr char KeyDrivename[] = "/dev/ttymxc1";
#else
constexpr char KeyDrivename[] = "/dev/ttyUSB1";
#endif
//uart接收到的键值与标准按键键值对应关系
const KeyTurnAroundTab KeyTable[] =
{
    {UART_KEY_L1,KEY_MINUS},{UART_KEY_L2,KEY_KPASTERISK},
    {UART_KEY_L3,KEY_KPSLASH},{UART_KEY_R1,KEY_1},
    {UART_KEY_R2,KEY_4},{UART_KEY_R3,KEY_7},
    {UART_KEY_2,KEY_2},{UART_KEY_3,KEY_3},
    {UART_KEY_5,KEY_5},{UART_KEY_6,KEY_6},
    {UART_KEY_8,KEY_8},{UART_KEY_9,KEY_9},
    {UART_KEY_STOP,KEY_ESC},{UART_KEY_UP,KEY_UP},
    {UART_KEY_DOWN,KEY_DOWN},{UART_KEY_LEFT,KEY_LEFT},
    {UART_KEY_RIGHT,KEY_RIGHT},{UART_KEY_START,KEY_ENTER},
    {UART_KEY_0,KEY_0},{UART_KEY_WORK,KEY_LEFTCTRL},
};

//Qt键值与uart键值对应关系（点灯用）
const KeyTurnAroundTab KeyTableLed[] =
{
    {UART_KEY_L1, Qt::Key_Minus},{UART_KEY_L2,Qt::Key_Asterisk},
    {UART_KEY_L3,Qt::Key_Slash},{UART_KEY_R1,Qt::Key_1},
    {UART_KEY_R2,Qt::Key_4},{UART_KEY_R3,Qt::Key_7},
    {UART_KEY_2,Qt::Key_2},{UART_KEY_3,Qt::Key_3},
    {UART_KEY_5,Qt::Key_5},{UART_KEY_6,Qt::Key_6},
    {UART_KEY_8,Qt::Key_8},{UART_KEY_9,Qt::Key_9},
    {UART_KEY_STOP,Qt::Key_Escape},{UART_KEY_UP,Qt::Key_Up},
    {UART_KEY_DOWN,Qt::Key_Down},{UART_KEY_LEFT,Qt::Key_Left},
    {UART_KEY_RIGHT,Qt::Key_Right},{UART_KEY_START,Qt::Key_Return},
    {UART_KEY_0,Qt::Key_0},{UART_KEY_WORK,Qt::Key_Control},
};
uint16_t timerIdKeyHeart = 0xff;        //键盘模块心跳帧超时定时器🆔id
uint16_t KeyLongDownTimeout = 0xff;     //按键长按超时定时器
uint32_t RecvKeyNum = 0;                //接收到的按键键值
/*********************************************************************************
* 									FUNCTION
*********************************************************************************/

/*******************************************************************************************
**函数名称：Key
**函数作用：构造函数
**函数参数：Name：设备名 Baud：波特率
**函数输出：无
**注意事项：无
*******************************************************************************************/
Key::Key(const char*Name,int Baud):UartBus(Name,Baud)
{
    unsigned char i;

    //处理方法及超时时间赋值
    DataProcType KeyRecvDealFunc[] =
    {
        {HEARTBEAT,0,HeartRecvDealCb,NULL},
        {KEYVALUEANDSTATUS,0,KeyNumRecvDealCb,NULL},
    };

    for(i = 0; i < sizeof(KeyRecvDealFunc)/sizeof(DataProcType);i++)
        KeyProtocolFunc.append(KeyRecvDealFunc[i]);
    //uinput设备初始化
    uinputInstance = new UinputController;
    uinputInstance->Init();
    memset(AppVerson,0,sizeof(AppVerson));
    OnlineStatus = 0;
    UpdateSem =  new semt;
}
/*******************************************************************************************
**函数名称：～Key
**函数作用：析构函数
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
Key::~Key()
{
    uinputInstance->Close();
    delete uinputInstance;
    delete UpdateSem;
}

/*******************************************************************************************
**函数名称：AppStart
**函数作用：键盘应用启动
**函数参数：无
**函数输出：无
**注意事项：主线程中先实例化，再调用此函数即可
*******************************************************************************************/
void Key::ThreadCreat()
{
    int res = 0;
    pthread_attr_t attr;
    size_t stack_size = 1 *512 * 1024; // 设置 256kB 的堆栈大小

    /* 初始化线程属性对象 */
    pthread_attr_init(&attr);

    /* 设置线程堆栈大小属性 */
    pthread_attr_setstacksize(&attr, stack_size);
    //创建线程
    res |= pthread_create(&Drivethreadid,&attr,DriveThreadFunc,this);
    res |= pthread_create(&RecvPthreadID,&attr,RecievedThreadFunc,this);
    res |= pthread_create(&SendPthreadID,&attr,SendThreadFunc,this);
    res |= pthread_create(&CyclePthreadID,&attr,CycleMngThreadFunc,this);
    res |= pthread_create(&UpdatePthreadID,&attr,ProgrameUpdataPro,this);

    if(res != 0)
    {
        perror("pthread Creat errot");
    }
}
/*******************************************************************************************
**函数名称：UartTrsProtocolDeal
**函数作用：发送协议处理
**函数参数：funcid：功能码 pframe：待发送的帧数据
**函数输出：0正确，非0错误
**注意事项：时间单位（ms）
*******************************************************************************************/
char Key::UartTrsProtocolDeal(__uint8_t funcid, stUartTrsQueue *pframe)
{
    //给超时时间和处理方法赋值
    foreach (DataProcType list, KeyProtocolFunc)
    {
        if(list.id == funcid)
        {
            pframe->TraBusHead.Overtime  = list.OverTime;       //ms
            pframe->TraBusHead.OtCbFunc = list.OverTimeFuncCb;
            break;
        }
    }

    switch(funcid)
    {
        case HEARTBEAT :				//
        case COMMUNICATIONLED:
        {
            pframe->TraBusHead.ack = 0;
            pframe->TraBusHead.count = 1;
            pframe->TraBusHead.u16CurInterval = 0;
            pframe->TraBusHead.u16Interval = 100;           //ms
        }break;
        case UPDATAPROGRAM:
        {
            pframe->TraBusHead.ack = 0;
            pframe->TraBusHead.count = 1;
            pframe->TraBusHead.u16CurInterval = 50;
            pframe->TraBusHead.u16Interval = 0;           //ms
        }break;
        default:
        {
            pframe->TraBusHead.ack = 0;
            pframe->TraBusHead.count = 0;
            pframe->TraBusHead.u16CurInterval = 0;
            pframe->TraBusHead.u16Interval = 0;
        }break;
    }
    AddTotalTTL();

    return 0;
}
/********************************************************************************
* 功能描述： 接收协议处理
* 入口参数：funcid 功能码 pframe 发送数据链表
* 返回值：无
********************************************************************************/
char Key::UartRecvProtocolDeal(UartFrame *pframe)
{
    if(pframe->FrameType > KEY_FUNC_MAX-1)
    {
        qDebug("Frame Type Error");
        return 1;
    }
    //开启定时器,键盘模块发键值的时候就不发心跳了，因此收到数就重置定时器，防止心跳超时认为掉线
    Timer::getInstance().startTimerEvent(timerIdKeyHeart);

    foreach (DataProcType list, KeyProtocolFunc)
    {
        if(list.id == pframe->FrameType)
        {
            if(list.RecvFuncCb == NULL)
                break;
            list.RecvFuncCb(pframe,this);
        }
    }
    return 0;
}

/*******************************************************************************************
**函数名称：KeyHeartTimeoutDeal
**函数作用：心跳超时回调
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void KeyHeartTimeoutDeal(void)
{
    if(key->OnlineStatus == 1)
    {
        //上报无线设备状态
        uint16_t tdata[6];
        tdata[0] = 0;
        tdata[1] = 0;
        tdata[2] = 0;
        tdata[3] = 0;
        tdata[4] = 0;
        tdata[5] = 0;
        Ydshrdlib::getInston()->sendShmDataFunc(1,65,((1<<5)|8),1,0,(unsigned char *)tdata,sizeof(tdata));
        //清除在线标志
        key->OnlineStatus = 0;
        qDebug("Key heart time out");
    }
}
/*******************************************************************************************
**函数名称：HeartRecvDealCb
**函数作用：心跳接收处理
**函数参数：frame： 帧格式 send：发送处理函数指针 Rev：接受处理函数指针
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char Key::HeartRecvDealCb(UartFrame *pframe,void*arg)
{
    Key * self;
    self = static_cast<Key* > (arg);

//    qDebug("recv key verson:%d.%d.%d.%d",pframe->SendBuf[4],pframe->SendBuf[5],pframe->SendBuf[6],pframe->SendBuf[7]);
    //上报无线设备状态
    pframe->SendBuf[4] = 0;      //版本只有3位，读上来的第一位是错的
    if((self->OnlineStatus == 0) || (memcmp(self->AppVerson,&pframe->SendBuf[4],sizeof(self->AppVerson)) != 0))
    {
        //读取版本号
        memcpy(self->AppVerson,&pframe->SendBuf[4],sizeof(self->AppVerson));
//        self->AppVerson[0] = 0;  //版本只有3位，读上来的第一位是错的
        self->OnlineStatus = 1;
        uint16_t tdata[6];
        tdata[0] = 1;
        tdata[1] = 0;
        tdata[2] = ((uint16_t)self->AppVerson[1])|((uint16_t)self->AppVerson[0] << 8);
        tdata[3] = ((uint16_t)self->AppVerson[3])|((uint16_t)self->AppVerson[2] << 8);
        tdata[4] = 0;
        tdata[5] = 0;
        Ydshrdlib::getInston()->sendShmDataFunc(1,65,((1<<5)|8),1,0,(unsigned char *)tdata,sizeof(tdata));
        qDebug("key verson:%d.%d.%d.%d",self->AppVerson[0],self->AppVerson[1],self->AppVerson[2],self->AppVerson[3]);
    }
    return 0;
}
/*******************************************************************************************
**函数名称：KeyNumRecvDealCb
**函数作用：接受到按键值处理函数
**函数参数：frame： 帧格式 send：发送处理函数指针 Rev：接受处理函数指针
**函数输出：无
**注意事项：无
*******************************************************************************************/
char Key::KeyNumRecvDealCb(UartFrame *pframe,void*arg)
{
    Key * self;
    self = static_cast<Key* > (arg);
    unsigned int  KeyNum = 0 ,temp1,temp2;
    static unsigned int keyold = 0;     //上一次的按键值
    unsigned char i;

    if(pframe->u8DLC != 9)
    {
        qDebug("key data length error");
        return 1;
    }
    //取按键值
    KeyNum = (static_cast<unsigned int> (pframe->SendBuf[0])) & 0x000000ff;
    KeyNum |= ((static_cast<unsigned int> (pframe->SendBuf[1])) & 0x000000ff) << 8;
    KeyNum |= ((static_cast<unsigned int> (pframe->SendBuf[2])) & 0x000000ff) << 16;
    KeyNum |= ((static_cast<unsigned int> (pframe->SendBuf[3])) & 0x000000ff) << 24;
    //发送应答数据
    char ackdata = 1;
    self->UartSendAckApi(pframe,&ackdata,1,0);
    //长按按超时处理
    RecvKeyNum = KeyNum;
    if(KeyNum)
        Timer::getInstance().startTimerEvent(KeyLongDownTimeout);
    else
        Timer::getInstance().stopTimerEvent(KeyLongDownTimeout);
    //键值未发生变化
    if(keyold == KeyNum)
        return 1;
    //test
//    qDebug("keynum=0x%08x",KeyNum);
//    qDebug("keyold=0x%08x",keyold);
    //键值转换，并发送按键事件
    temp1 = (keyold^KeyNum)&keyold;     //获取keyold中变化的位
    temp2 = (keyold^KeyNum)&KeyNum;     //获取keyNum中变化的位
    for(i = 0; i < (sizeof(KeyTable)/sizeof(KeyTurnAroundTab)); i++)
    {
        if((((KeyNum & KeyTable[i].UartKeyNum)^KeyTable[i].UartKeyNum) == 0)
            ||(((keyold & KeyTable[i].UartKeyNum)^KeyTable[i].UartKeyNum) == 0))
        {
            if(temp2 & KeyTable[i].UartKeyNum)
            {
                 self->uinputInstance->send_key_press(KeyTable[i].KeEventData);
                 qDebug("key%d down",KeyTable[i].KeEventData);
                 //上报按键按下
                 uint16_t temp = 1;
                 Ydshrdlib::getInston()->sendShmDataFunc(1,55,((1<<5)|8),1,1,(unsigned char *)&temp,2);
            }
            if(temp1 & KeyTable[i].UartKeyNum)
            {
                self->uinputInstance->send_key_release(KeyTable[i].KeEventData);
                qDebug("key%d up",KeyTable[i].KeEventData);
            }
        }
    }
    keyold = KeyNum;    //记录上一次的按键值

    return 0;
}

/*******************************************************************************************
**函数名称：SendProgrameUpdateProc
**函数作用：程序更新
**函数参数：name 文件名（绝对路径）
**函数输出：无
**注意事项：无
*******************************************************************************************/
char Key::SendProgrameUpdateProc(char* name)
{
    if(name == nullptr)
    {
        qDebug("updata Programe name null");
        return 1;
    }
    uint32_t ProgramSize = 0,SentProgramSize = 0;      //程序大小
    uint8_t Sentpercent = 0;
    static uint8_t oldSentpercent = 0;
    char buff[USART_SEND_BUF_SIZE * 2];

    int file = open(name, O_RDONLY);
    if (file == -1) {
        qDebug("Programe file open error ");
        return 1;
    }
    lseek(file, 0x0c, SEEK_SET);  // 定位到程序大小
    ssize_t bytes_read = read(file, &ProgramSize, sizeof(uint32_t));  // 读取遥控器程序大小
    qDebug("--------updata Programe start ! file size :0x%x---------- ",ProgramSize);
    lseek(file, 0, SEEK_SET);  // 定位到起始位置
    //发送程序
    while((bytes_read = read(file,buff, USART_SEND_BUF_SIZE)))
    {
        if(bytes_read > USART_SEND_BUF_SIZE)
            break;
        buff[bytes_read] = bytes_read;
        SendDataDealApi(UPDATAPROGRAM,ACK,buff,bytes_read+1,0);
        SentProgramSize += bytes_read;     //已发送大小
        Sentpercent = (SentProgramSize*100)/ProgramSize;
        //进度每变化1%上报
        if(oldSentpercent != Sentpercent)
        {
            oldSentpercent = Sentpercent;
            UpdataResType temp;
            temp.FileType = 3;  //程序
            temp.DestDriveType = 8; //键盘设备类型
            temp.DestDriveNUm = 1;
            temp.Result = Sentpercent;
            Ydshrdlib::getInston()->sendMsgDataFunc(1,3,((1<<5)|8),1,0,(uint8_t*)&temp,sizeof(temp));
            qDebug("sent updata pargram %d percent",Sentpercent);
        }
        if(SentProgramSize <= USART_SEND_BUF_SIZE)    //首帧设备需要擦除flash，延时3s
        {
            usleep(3000*1000);
        }
        else {
            usleep(150*1000);
        }
    }

    close(file);  // 关闭文件
    qDebug("----------program send finish-----------");

    return 0;
}

/*******************************************************************************************
**函数名称：ProgrameUpdataPro
**函数作用：程序更新线程
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void *Key::ProgrameUpdataPro(void *arg)
{
    Key * self;
    self = (Key*)arg;
    while(1)
    {
        if(self->UpdateSem->wait_sem() == 0)
        {
            self->SendProgrameUpdateProc(self->Programename);
        }

        usleep(10000);
    }
}
/*******************************************************************************************
**函数名称：RecvMsgCallbackKeyinit
**函数作用：设在键盘消息接收处理回调
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvMsgCallbackKeyinit(uint8_t point,uint8_t* data,int len)
{
    (void)* data;   //消除警告
    (void) len;     //消除警告
    (void) point;     //消除警告

    qDebug("key init message");
    //上报无线设备状态
    uint16_t tdata[6];
    if(key->OnlineStatus == 0)
    {
        tdata[0] = 0;
        tdata[1] = 0;
        tdata[2] = 0;
        tdata[3] = 0;
        tdata[4] = 0;
        tdata[5] = 0;
    }
    else
    {
        tdata[0] = 1;
        tdata[1] = 0;
        tdata[2] = ((uint16_t)key->AppVerson[1])|((uint16_t)key->AppVerson[0] << 8);
        tdata[3] = ((uint16_t)key->AppVerson[3])|((uint16_t)key->AppVerson[2] << 8);
        tdata[4] = 0;
        tdata[5] = 0;
    }

    Ydshrdlib::getInston()->sendShmDataFunc(1,65,((1<<5)|8),1,0,(unsigned char *)tdata,sizeof(tdata));

    //上电熄灭所有的键盘灯，避免指示灯异常
    char SendBuf[8];
    //on
    SendBuf[0] = 0;
    SendBuf[1] = 0;
    SendBuf[2] = 0;
    SendBuf[3] = 0;
    //0ff
    SendBuf[4] = 0xff;
    SendBuf[5] = 0xff;
    SendBuf[6] = 0xff;
    SendBuf[7] = 0xff;
    key->SendDataDealApi(NAVIGATIONLED,NOACK,SendBuf,8,0);
}

/*******************************************************************************************
**函数名称：KeyLongDownTimeoutCb
**函数作用：长按超时处理
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void KeyLongDownTimeoutCb(void)
{
    if(RecvKeyNum == 0)
        return;
     for(uint8_t i = 0; i < (sizeof(KeyTable)/sizeof(KeyTurnAroundTab));i++)
    {
        if((RecvKeyNum & KeyTable[i].UartKeyNum) != 0)
            key->uinputInstance->send_key_release(KeyTable[i].KeEventData);
        else
            continue;
    }
    qDebug("key long down time out");
}

/*******************************************************************************************
**函数名称：KeyAppStart
**函数作用：键盘逻辑
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void KeyAppStart()
{
    //全局实例化按键类对象
    key = new Key(KeyDrivename,115200);
    //创建键盘相关线程
    key->ThreadCreat();
    //添加键盘模块心跳超时定时器
    timerIdKeyHeart = Timer::getInstance().addTimerEvent(9*1000,0xff,KeyHeartTimeoutDeal);
    Timer::getInstance().startTimerEvent(timerIdKeyHeart);
    //按键长按超时
    KeyLongDownTimeout = Timer::getInstance().addTimerEvent(200,1,KeyLongDownTimeoutCb);
}

/*******************************************************************************************
**函数名称：RecvShmCallback_KeyLedCtrl
**函数作用：设在键盘导航灯处理回调（57，控制驱动）
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvShmCallback_KeyLedCtrl(uint8_t point,uint8_t* data,int len)
{
    (void)point;        //消除警告

    unsigned int keyledon = 0,keyledoff = 0;
    uint8_t i,j;
    char SendBuf[8];
    int tempkey[60];

    if((uint32_t)len > sizeof(tempkey))
    {
        qDebug("key led data length error");
        return;
    }

    memcpy(tempkey,data,len);
    for(j = 0;j < len; j++)
    {
        if(tempkey[j] == 0)     //非0才有意义
            continue;
        for(i = 0; i < (sizeof(KeyTableLed)/sizeof(KeyTurnAroundTab)); i++)
        {
            if(tempkey[j] == KeyTableLed[i].KeEventData)
            {
                if(j < len/(2*sizeof(uint32_t)))   //前30个灯亮，后30个灯灭
                {
                    keyledon |= KeyTableLed[i].UartKeyNum;
                }
                else
                {
                    keyledoff |= KeyTableLed[i].UartKeyNum;
                }
                continue;
            }
        }
    }
//    //test
//    qDebug("key led on :0x%08x",keyledon);
//    qDebug("key led off :0x%08x",keyledoff);
    SendBuf[0] = (uint8_t)( keyledon & 0x000000FF);
    SendBuf[1] = (uint8_t)((keyledon & 0x0000FF00) >> 8);
    SendBuf[2] = (uint8_t)((keyledon & 0x00FF0000) >> 16);
    SendBuf[3] = (uint8_t)((keyledon & 0xFF000000) >> 24);

    SendBuf[4] = (uint8_t)( keyledoff & 0x000000FF);
    SendBuf[5] = (uint8_t)((keyledoff & 0x0000FF00) >> 8);
    SendBuf[6] = (uint8_t)((keyledoff & 0x00FF0000) >> 16);
    SendBuf[7] = (uint8_t)((keyledoff & 0xFF000000) >> 24);
    key->SendDataDealApi(NAVIGATIONLED,NOACK,SendBuf,8,0);
}

/*******************************************************************************************
**函数名称：RecvMsgCallback_WlFileDownLoad
**函数作用：设在无线模块程序更新
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvMsgCallback_KeyFileDownLoad(uint8_t point,uint8_t* data,int len)
{
    (void)point;    //消除警告
    uint16_t temp;

    memset(key->Programename,0,sizeof(key->Programename));
    memcpy(&temp,data,2);
    if(temp != 3)           //文件类型 = 程序
    {
        qDebug("programe file type error:%d",temp);
        return;
    }
    memcpy(&temp,data+2,2);
    if(temp != 8)           //设备类型
    {
        qDebug("Updata Drive type error:%d",temp);
        return;
    }
    memcpy(key->Programename,data+6,len-6);
    //test
    qDebug() << "programe file name:"<< key->Programename;
    key->UpdateSem->post_sem();
}
