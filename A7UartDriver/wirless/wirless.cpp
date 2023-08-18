/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "wirless.h"
#include <qdebug.h>
#include "ydshrdlib.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include "timer/timer.h"
#include <fcntl.h>
#include "ir/ir.h"
/*********************************************************************************
* 									DEFINES
*********************************************************************************/
#define YKQ_DEV_TYPE        (0x00100A08)//遥控器
#define YKQ_PARA_SIZE       1856        //遥控器配置文件缓存大小
#define USART_SEND_BUF_SIZE  128        //程序更新缓存大小
/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/
//无线类对象
wirless *wl = nullptr;
//设备名
#ifdef PLATFORM_TYPE            //arm版
constexpr char WirlessDrivename[] = "/dev/ttymxc2";
#else                           //pc版
constexpr char WirlessDrivename[] = "/dev/ttyUSB6";
#endif
//遥控器配置文件路径
constexpr char PathRemoteActionInfo[] = "/opt/config/usr/paramconfig/Actioninfo.bin";
uint32_t RuningActionInfo = 0;              //正在执行的动作信息（1<<id-1），目的与心跳帧对比
uint16_t timerIdWlHeart = 0xff;             //键盘模块心跳帧超时定时器🆔id
uint16_t timeIdV4DriveDecode = 0xff;        //控制器主动解除对码定时器id
uint16_t timeIdRemoteActionHeart = 0xff;    //遥控器动作心跳定时器id
/*********************************************************************************
* 									声明
*********************************************************************************/
void SendYkqParaProc(void);     //发送配置文件
/*********************************************************************************
* 									FUNCTION
*********************************************************************************/
/*******************************************************************************************
**函数名称：wirless
**函数作用：构造函数
**函数参数：Name：设备名 Baud：波特率
**函数输出：无
**注意事项：接收处理及超时处理回调函数在此增加
*******************************************************************************************/
wirless::wirless(const char *Name, int Baud):UartBus(Name,Baud),LiushuiNum(1)
{
    unsigned char i;

    //处理方法及超时时间赋值
    DataProcType WirlessRecvDealFunc[] =
    {
        {WIRELESS_HEART,0,WirlessRecvHeartDealCb,NULL},
        {WIRELESS_DATA,0,WirlessRecvDataDealCb,NULL},
        {WIRELESS_STARTHINT,0,WirlessRecvStartInitDealCb,NULL},
    };

    for(i = 0; i < sizeof(WirlessRecvDealFunc)/sizeof(DataProcType);i++)
        WirlessProtocolFunc.append(WirlessRecvDealFunc[i]);
    //参数初始化
    memset(&V4ykq,0,sizeof(V4YKQ_DATA));
    OnlineStatus = 0;
    UpdateSem =  new semt;
}

/*******************************************************************************************
**函数名称：～wirless
**函数作用：析构函数
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
wirless::~wirless()
{
    delete UpdateSem;
}

/*******************************************************************************************
**函数名称：ThreadCreat
**函数作用：无线应用启动
**函数参数：无
**函数输出：无
**注意事项：主进程中先实例化，再调用此函数
*******************************************************************************************/
void wirless::ThreadCreat()
{
    int res = 0;
    pthread_attr_t attr;
    size_t stack_size = 1 *512 * 1024; // 设置 512kB 的堆栈大小

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
**函数参数：funcid 功能码 pframe 发送数据链表
**函数输出：无
**注意事项：单位（ms）
*******************************************************************************************/
char wirless::UartTrsProtocolDeal(__uint8_t funcid, stUartTrsQueue *pframe)
{
    //给超时时间和处理方法赋值
    foreach (DataProcType list, WirlessProtocolFunc)
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
        case WIRELESS_DATA:
        {
            STR_WLCAN_Frame tframe;
            memcpy(&tframe,&pframe->TrsFrame.SendBuf[1],sizeof(STR_WLCAN_Frame));
            switch (tframe.u32ID.ID.FrameType)      //遥控器数据协议
            {
                case V4YKQ_ACTIVE_DECODE:
                    pframe->TraBusHead.ack = 0;
                    pframe->TraBusHead.count = 3;
                    pframe->TraBusHead.u16CurInterval = 0;
                    pframe->TraBusHead.u16Interval = 60;
                    break;
                case V4YKQ_CONTROL_DATA_ACK:
                    pframe->TraBusHead.ack = 0;
                    pframe->TraBusHead.count = 3;
                    pframe->TraBusHead.u16CurInterval = 0;
                    pframe->TraBusHead.u16Interval = 30;
                    break;
                case RFID_FT_UPDATE_DWL_VERSION:
                case RFID_FT_UPDATE_WRITE_VERSION:
                case RFID_FT_UPDATE_DWL_CODE:
                case RFID_FT_UPDATE_WRITE_CODE:
                case RFID_FT_UPDATE_END:
                {
                    pframe->TraBusHead.ack = 0;
                    pframe->TraBusHead.count = 2;
                    pframe->TraBusHead.u16CurInterval = 50;
                    pframe->TraBusHead.u16Interval = 50;
                }break;
                default:
                    pframe->TraBusHead.ack = 0;
                    pframe->TraBusHead.count = 0;
                    pframe->TraBusHead.u16CurInterval = 0;
                    pframe->TraBusHead.u16Interval = 0;
                    break;
            }
        }break;
        case WIRELESS_PARASET:
        {
            pframe->TraBusHead.ack = 0;
            pframe->TraBusHead.count = 2;
            pframe->TraBusHead.u16CurInterval = 30;
            pframe->TraBusHead.u16Interval = 30;
        }break;
        case WIRELESS_UPDATE:
        {
            pframe->TraBusHead.ack = 0;
            pframe->TraBusHead.count = 1;
            pframe->TraBusHead.u16CurInterval = 50;
            pframe->TraBusHead.u16Interval = 0;
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
/*******************************************************************************************
**函数名称：UartRecvProtocolDeal
**函数作用：接收协议处理
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::UartRecvProtocolDeal(UartFrame *pframe)
{
    if(pframe->FrameType > WIRLESS_FUNC_MAX-1)
    {
        qDebug("Frame Type Error");
        return 1;
    }
//    qDebug("wirless uart frame type = %d", pframe->FrameType);
    foreach (DataProcType list, WirlessProtocolFunc)
    {
        if(list.id == pframe->FrameType)
        {
            if(list.RecvFuncCb == NULL)
            {
                qDebug("wirless Recv call back NULL");
                break;
            }
            list.RecvFuncCb(pframe,this);
        }
    }

    return 0;
}

/*******************************************************************************************
**函数名称：RemoteContralSendDataApi
**函数作用：遥控器数据发送接口
**函数参数：pframe:CAN帧数据
**函数输出：无
**注意事项：无
*******************************************************************************************/
void wirless::RemoteContralSendDataApi(STR_WLCAN_Frame *pframe)
{
    //更新流水号
    AddLiuShuiNum();
    //将遥控器使用的外围设备协议作为uart帧的包载荷
    char buff[UARTFRAME_MAX_LEN];
    buff[0] = 0xff - V4ykq.YkqNum;
    memcpy(&buff[1],pframe,sizeof(STR_WLCAN_Frame));
    uint16_t len = 1 + 4 + pframe->u16DLC +1;
    //调用uart接口发送
    SendDataDealApi(WIRELESS_DATA,pframe->u32ID.ID.ACK,(char*)buff,len);
}

/*******************************************************************************************
**函数名称：RemoteContralIrRecvCBFunc
**函数作用：遥控器红外对码接收处理
**函数参数：pframe:CAN帧数据
**函数输出：无
**注意事项：无
*******************************************************************************************/
void wirless::RemoteContralIrRecvCBFunc(char &ykqnum)
{
    AddLiuShuiNum();
    //将红外数据包装成外设CAN帧，为防止重复帧过滤，临时使用了发送的流水号
    STR_WLCAN_Frame RxIrCan;

    RxIrCan.u8DT[0] = 0x0;
    RxIrCan.u8DT[1] = 0x0;
    RxIrCan.u8DT[2] = 0x0;
    RxIrCan.u8DT[3] = 0x0;
    RxIrCan.u8DT[4] = 0x0;
    RxIrCan.u8DT[5] = ykqnum;
    RxIrCan.u8DT[6] = 0x0;
    RxIrCan.u8DT[7] = 0x0;
    RxIrCan.u16DLC = 8;
    RxIrCan.u32ID.ID.ACK = 0x00;
    RxIrCan.u32ID.ID.FrameType = V4YKQ_IR_CODEMATCH;
    RxIrCan.u32ID.ID.LiuShuiNumb = GetLiuShuiNum();
    RxIrCan.u32ID.ID.NoUsed = 0;
    RxIrCan.u32ID.ID.RxID = 0;
    RxIrCan.u32ID.ID.Sub = 0;
    RxIrCan.u32ID.ID.Sum = 0;
    RxIrCan.u32ID.ID.TxID = 3;
    //将数据加入接收链表
    uint16_t len = sizeof(STR_WLCAN_Frame)+UARTFRAME_HEAD_LEN;
    char data[24];
    data[0] = 3;
    data[1] = GetLiuShuiNum();
    data[2] = WIRELESS_DATA;
    data[3] = NOACK;
    data[4] = sizeof(STR_WLCAN_Frame);
    memcpy(&data[5],&RxIrCan,sizeof(STR_WLCAN_Frame));
    uint16_t crc = 0;
    Crc16Ccitt(data,len,&crc);
    data[len + 1] = crc & 0x00ff;
    data[len] = (crc & 0xff00) >> 8;

    UartBus_InsertRevBuf(data,len+2);
}
/*******************************************************************************************
**函数名称：AddLiuShuiNum
**函数作用：遥控器流水号自增
**函数参数：无
**函数输出：无
**注意事项：根据外围设备协议，变化范围0x0-0xf，（4bit）
*******************************************************************************************/
void wirless::AddLiuShuiNum()
{
    this->LiushuiNum++;
    this->LiushuiNum %= 16;
}
/*******************************************************************************************
**函数名称：ClearLiuShuiNum
**函数作用：遥控器重置流水号
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void wirless::ClearLiuShuiNum()
{
    this->LiushuiNum = 1;
}
/*******************************************************************************************
**函数名称：GetLiuShuiNum
**函数作用：遥控器获取流水号
**函数参数：无
**函数输出：流水号
**注意事项：无
*******************************************************************************************/
__uint8_t wirless::GetLiuShuiNum()
{
    return this->LiushuiNum;
}

/*******************************************************************************************
**函数名称：SetWirlessSyncword
**函数作用：遥控器设置无线同步字
**函数参数：无
**函数输出：SYNC_ACTION：动作控制模式，SYNC_UPDATA：程序更新
**注意事项：无
*******************************************************************************************/
void wirless::SetWirlessSyncword(uint8_t mode)
{
    uint8_t Sendbuf[3];
    Sendbuf[0] = V4ykq.SCNum;
    if(mode == SYNC_ACTION)
    {
        Sendbuf[1] = 0x33;
        Sendbuf[2] = 0x33;
    }
    else
    {
        Sendbuf[1] = 0x33;
        Sendbuf[2] = 0x55;
    }
    SendDataDealApi(WIRELESS_PARASET,ACK,(char*)Sendbuf,sizeof(Sendbuf),0);
}

/*******************************************************************************************
**函数名称：SendProgrameUpdateProc
**函数作用：无线模块程序更新
**函数参数：name：完整文件路径+文件名
**函数输出：0:正确，1：错误
**注意事项：无
*******************************************************************************************/
char wirless::SendProgrameUpdateProc(char *name)
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
        SendDataDealApi(WIRELESS_UPDATE,ACK,buff,bytes_read + 1,0);
        SentProgramSize += bytes_read;     //已发送大小
        Sentpercent = (SentProgramSize*100)/ProgramSize;
        //进度每变化1%上报
        if(oldSentpercent != Sentpercent)
        {
            oldSentpercent = Sentpercent;
            UpdataResType temp;
            temp.FileType = 3;  //程序
            temp.DestDriveType = 9; //无线设备类型
            temp.DestDriveNUm = 1;
            temp.Result = Sentpercent;
            Ydshrdlib::getInston()->sendMsgDataFunc(1,3,((2<<5)|9),1,0,(uint8_t*)&temp,sizeof(temp));
            qDebug("sent updata pargram %d percent",Sentpercent);
        }
        if(SentProgramSize <= USART_SEND_BUF_SIZE)    //首帧数据，设备需要擦除flash，延时3s
        {
            usleep(3000*1000);
        }
        else {
            usleep(150*1000);
        }
        bytes_read = 0;
    }

    close(file);  // 关闭文件
    qDebug("----------program send finish-----------");

    return 0;
}

/*******************************************************************************************
**函数名称：WlHeartTimeoutDeal
**函数作用：无线心跳超时回调
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void WlHeartTimeoutDeal(void)
{
    if(wl->OnlineStatus == 1)
    {
        //上报无线设备状态
        uint16_t tdata[6];
        tdata[0] = 0;
        tdata[1] = 0;
        tdata[2] = 0;
        tdata[3] = 0;
        tdata[4] = 0;
        tdata[5] = 0;
        Ydshrdlib::getInston()->sendShmDataFunc(1,65,((2<<5)|9),1,0,(uint8_t *)tdata,sizeof(tdata));
        //清除在线标志
        wl->OnlineStatus = 0;
        qDebug("wirless heart time out");
    }
}
/*******************************************************************************************
**函数名称：WirlessRecvHeartDealCb
**函数作用：无线心跳数据处理
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::WirlessRecvHeartDealCb(UartFrame *pframe, void *arg)
{
    wirless * self;
    self = static_cast<wirless* > (arg);
    //开启定时器
    Timer::getInstance().startTimerEvent(timerIdWlHeart);

    //上报无线设备状态
    pframe->SendBuf[4] = 0;      //版本只有3位，读上来的第一位是错的
    if((self->OnlineStatus == 0)|| (memcmp(self->V4ykq.AppVerson,&pframe->SendBuf[4],sizeof(self->V4ykq.AppVerson)) != 0))
    {
        //读取版本号
        memcpy(self->V4ykq.AppVerson,&pframe->SendBuf[4],sizeof(self->V4ykq.AppVerson));
//        self->V4ykq.AppVerson[0] = 0;
        self->OnlineStatus = 1;
        uint16_t tdata[6];
        tdata[0] = 1;
        tdata[1] = 0;
        tdata[2] = ((uint16_t)self->V4ykq.AppVerson[1])|((uint16_t)self->V4ykq.AppVerson[0] << 8);
        tdata[3] = ((uint16_t)self->V4ykq.AppVerson[3])|((uint16_t)self->V4ykq.AppVerson[2] << 8);
        tdata[4] = 0;
        tdata[5] = 0;
        Ydshrdlib::getInston()->sendShmDataFunc(1,65,((2<<5)|9),1,0,(unsigned char *)tdata,sizeof(tdata));
        qDebug("wirless verson:%d.%d.%d.%d",self->V4ykq.AppVerson[0],self->V4ykq.AppVerson[1],self->V4ykq.AppVerson[2],self->V4ykq.AppVerson[3]);
    }
    return 0;
}

/*******************************************************************************************
**函数名称：WirlessRecvDataDealCb
**函数作用：无线数据处理
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::WirlessRecvDataDealCb(UartFrame *pframe, void *arg)
{
    wirless * self;
    self = static_cast<wirless* > (arg);
    //数据类型转换
    STR_WLCAN_Frame tempcanframe;
    memcpy(&tempcanframe,pframe->SendBuf,sizeof(STR_WLCAN_Frame));
    //判断是否时重复帧
    static char oldLiuShuiNumb = 0xf;
    if(tempcanframe.u32ID.ID.LiuShuiNumb == oldLiuShuiNumb)
    {
        qDebug("remote control LiuShuiNumb same,liushuihao = %d",oldLiuShuiNumb);
        if(tempcanframe.u32ID.ID.FrameType == V4YKQ_NEIGHBOR_REQUEST
            ||tempcanframe.u32ID.ID.FrameType == V4YKQ_CONTROL_DATA)
            return 1;
    }
    oldLiuShuiNumb = tempcanframe.u32ID.ID.LiuShuiNumb;
    //数据处理
    switch (tempcanframe.u32ID.ID.FrameType)
    {
        case V4YKQ_HEART:               //心跳
        {
            self->RemoteContralHeartDeal(&tempcanframe);
        }break;
        case V4YKQ_IR_CODEMATCH:        //红外对码
        {
            self->RemoteContralIrRecvDeal(&tempcanframe);
        }break;
        case V4YKQ_WL_CODEMATCH:        //遥控器无线对码
        {
            self->RemoteContralColibarayeDeal(&tempcanframe);
        }break;
        case V4YKQ_PASSIVE_DECODE:      //遥控器主动解除对码
        {
            self->RemoteContralDecondeDeal(&tempcanframe);
        }break;
        case V4YKQ_CONTROL_DATA:        //动作控制
        {
            self->RemoteContralActionCtrlDeal(&tempcanframe);
        }break;
        case V4YKQ_FILE_REQUEST:        //配置文件请求
        {
            self->RemoteContralActionInfoReqDeal(&tempcanframe);
        }break;
        case V4YKQ_NEIGHBOR_REQUEST:    //邻架控制
        {
            self->RemoteContralNeighboorCtrlReqDeal(&tempcanframe);
        }break;
        default:
            break;
    }
    return 0;
}

/*******************************************************************************************
**函数名称：WirlessRecvStartInitDealCb
**函数作用：设置无线同步字
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::WirlessRecvStartInitDealCb(UartFrame *pframe, void *arg)
{
    (void) *pframe;     // 使用类型转换将未使用的参数转换为空操作，解决警告

    wirless * self;
    self = static_cast<wirless* > (arg);

    self->SetWirlessSyncword(SYNC_ACTION);
    if(self->V4ykq.YkqState == STATE_WL_SUCCESS)
    {
        self->RemoteContralCodeMatchingAck_Proc(1);
    }

    return 0;
}
/*******************************************************************************************
**函数名称：RemoteContralHeartTimeOutCb
**函数作用：遥控器动作心跳超时处理
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：心跳超时停掉所有动作
*******************************************************************************************/
void RemoteContralHeartTimeOutCb(void)
{
    //关闭定时器
    Timer::getInstance().stopTimerEvent(timeIdRemoteActionHeart);

    for(uint8_t i = 0;i < 32; i++)
    {
        if(((RuningActionInfo >> i)&0x01) != 0)
        {
            //超时停动作
            RemoteActionCtrlDataType ActionData;
            ActionData.ActionID = i+1;
            ActionData.ContralSCNum = wl->V4ykq.controllNum;
            ActionData.ContralCommand = ActionCtrl_CmdType_STOP;
            ActionData.ActionCtrlMode = ActionCtrl_ModeType_MANUAL; //手动
            ActionData.CtrlSourceAddr = wl->V4ykq.YkqNum;
            ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ActionData.UserID = 0;
            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,0,(unsigned char *)&ActionData,14);
            qDebug("Action Heart time out,Stop action:%d",ActionData.ActionID);
        }
    }
    RuningActionInfo = 0;
}
/*******************************************************************************************
**函数名称：RemoteContralHeartDeal
**函数作用：遥控器心跳帧处理
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::RemoteContralHeartDeal(STR_WLCAN_Frame *frame)
{
    uint32_t temp ;

    if(frame->u8DT[0] != V4ykq.SCNum)
        return 1;
    if(frame->u8DT[1] != V4ykq.YkqNum)
        return 1;
    //重置定时器
    if(RuningActionInfo != 0)
        Timer::getInstance().startTimerEvent(timeIdRemoteActionHeart);
    //动作id对应的bit位
    uint32_t manualActionID;
    manualActionID = frame->u8DT[3]|(frame->u8DT[4]<<8)|(frame->u8DT[5]<<16)|(frame->u8DT[6]<<24);
//    qDebug("remote contral heart = 0x%08x",manualActionID);
    //给逻辑上报手动动作心跳，将动作🆔id对应的位转换位🆔id并填入u16的数组中上报给逻辑
    uint16_t TempActionId[10];
    temp = RuningActionInfo & manualActionID;
    TempActionId[0] = V4ykq.controllNum;
    if(temp != 0)
    {
        uint8_t j = 1;
        for(uint8_t i = 0;i < 32; i++)
        {
            if(((temp >> i)&0x01) != 0)
            {
                TempActionId[j] = i + 1;
                j++;
                j = j % 10;
            }
        }
        Ydshrdlib::getInston()->sendShmDataFunc(1,9,((2<<5)|9),1,0,(unsigned char *)TempActionId,j*sizeof(uint16_t));
    }
    //正常情况，心跳与实时动作一致。
    if((manualActionID^RuningActionInfo) == 0)
    {
        return 0;
    }
    else
    {
        temp = (manualActionID^RuningActionInfo)&RuningActionInfo;      //当前运行的动作与心跳不一致，找到这个动作并停止
        if(temp)
        {
            for(uint8_t i = 0;i < 32; i++)
            {
                if(((temp >> i)&0x01) != 0)
                {
                    //停动作
                    RemoteActionCtrlDataType ActionData;
                    ActionData.ActionID = i+1;
                    ActionData.ContralSCNum = V4ykq.controllNum;
                    ActionData.ContralCommand = ActionCtrl_CmdType_STOP;
                    ActionData.ActionCtrlMode = ActionCtrl_ModeType_MANUAL; //手动
                    ActionData.CtrlSourceAddr = V4ykq.YkqNum;
                    ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
                    ActionData.UserID = 0;
                    Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,0,(unsigned char *)&ActionData,14);
                    qDebug("Action Heart Error,Mast Stop action:%d",ActionData.ActionID);
                }
                else
                    continue;
            }

        }
    }
    //无动作的情况
    if(manualActionID == 0)
    {
        TempActionId[1] = 0;
        Ydshrdlib::getInston()->sendShmDataFunc(1,9,((2<<5)|9),1,0,(unsigned char *)TempActionId,4);
    }

    return 0;
}
/*******************************************************************************************
**函数名称：RemoteContralIrRecvDeal
**函数作用：遥控器红外对码接收处理
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::RemoteContralIrRecvDeal(STR_WLCAN_Frame *frame)
{
    if(V4ykq.YkqState == STATE_IDLE)
    {
        V4ykq.YkqState = STATE_IR_CODEMATCHING;
        V4ykq.YkqNum = frame->u8DT[5];
    }
    if(V4ykq.YkqState == STATE_WL_SUCCESS)
    {
        if(V4ykq.YkqNum != frame->u8DT[5])
            return 1;
    }
    V4ykq.YkqNum = frame->u8DT[5];
    //test
    qDebug("remote control ir Connect YkqNum = %d",V4ykq.YkqNum);
    //状态上报
    uint16_t RemoteContralConnectStatus = 1;
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((2<<5)|9),1,11,(unsigned char *)&RemoteContralConnectStatus,2);
    //发送红外对码应答
    RemoteContralIRSendConnectAck(V4ykq);
    return 0;
}

/*******************************************************************************************
**函数名称：WirelessSCDecode_Proc
**函数作用：控制器主动解除对码
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void WirelessSCDecode_Proc(void)
{
    if(wl->V4ykq.YkqNum == 0)
        return;

    if(wl->V4ykq.YkqState != STATE_WL_SUCCESS)
        return;

    Timer::getInstance().stopTimerEvent(timeIdV4DriveDecode);
    //发送解除对码帧
    STR_WLCAN_Frame frame;
    frame.u32ID.ID.ACK = 0;
    frame.u32ID.ID.FrameType = V4YKQ_ACTIVE_DECODE;
    frame.u32ID.ID.LiuShuiNumb = wl->GetLiuShuiNum();
    frame.u32ID.ID.RxID = 3;
    frame.u16DLC = 8;
    frame.u8DT[0] = 0;
    frame.u8DT[1] = 0;
    frame.u8DT[2] = 0;
    frame.u8DT[3] = wl->V4ykq.SCNum;
    frame.u8DT[4] = wl->V4ykq.YkqNum;
    frame.u8DT[5] = 0;
    frame.u8DT[6] = 0;
    frame.u8DT[7] = 0;

    wl->RemoteContralSendDataApi(&frame);
    //上报状态
    uint16_t RemoteContralConnectStatus = 3;
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((2<<5)|9),1,11,(unsigned char *)&RemoteContralConnectStatus,2);
    //停预警
    wl->RemoteContralOverrideWarnin(wl->V4ykq.controllNum,ActionCtrl_CmdType_STOP);
    //test
    qDebug("V4drive active decode,ykqid = %d，sc num = %d",wl->V4ykq.YkqNum,wl->V4ykq.SCNum);
    //清除遥控器状态信息
    wl->V4ykq.YkqNum = 0;
    wl->V4ykq.YkqCrc = 0;
    wl->V4ykq.YkqState = STATE_IDLE;
}
/*******************************************************************************************
**函数名称：RemoteContralColibarayeDeal
**函数作用：遥控器对码帧处理
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::RemoteContralColibarayeDeal(STR_WLCAN_Frame *frame)
{
    if(frame->u8DT[4] != V4ykq.SCNum)
    {
        qDebug("connect sc num error");
        return 1;
    }
    if((frame->u8DT[2]&0x40) != 0x40)//如果是红外对码
    {
        if(V4ykq.YkqState != STATE_IR_CODEMATCHING)
        {
            if(V4ykq.YkqNum != frame->u8DT[5])
                return 1;
        }
    }
    else//如果是±键对码
    {
        if(V4ykq.YkqState == STATE_WL_SUCCESS)
        {
            if(V4ykq.YkqNum != frame->u8DT[5])
                return 1;
        }
        else
            V4ykq.YkqNum = frame->u8DT[5];
    }

    //更新遥控器参数
    V4ykq.YkqCrc = (frame->u8DT[7]<<8)|frame->u8DT[6];
    //test
    qDebug("wl connect V4ykq.YkqNum = %d",V4ykq.YkqNum);
    qDebug("wl connect V4ykq.YkqCrc = 0x%x",V4ykq.YkqCrc);
    qDebug("wl connect V4ykq.YkqSCCrc = 0x%x",V4ykq.YkqSCCrc);
    V4ykq.YkqKey = frame->u8DT[0];
    V4ykq.YkqNum = frame->u8DT[5];
    V4ykq.YkqState = STATE_WL_SUCCESS;
    V4ykq.controllNum = V4ykq.SCNum;
    V4ykq.YkqCodingMode = ((frame->u8DT[2]&0x80) == 0x40)?0:1;
    //无线对码状态应答
    RemoteContralCodeMatchingAck_Proc(1);
    //状态上报
    uint16_t RemoteContralConnectStatus = STATE_IR_SUCCESS;
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((2<<5)|9),1,11,(unsigned char *)&RemoteContralConnectStatus,2);
    //发送预警
    RemoteContralOverrideWarnin(V4ykq.controllNum,ActionCtrl_CmdType_STARTUP);
    //启动定时器
    Timer::getInstance().startTimerEvent(timeIdV4DriveDecode);

    return 0;
}

/*******************************************************************************************
**函数名称：RemoteContralCodeMatchingAck_Proc
**函数作用：发送遥控器对码帧应答
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
void wirless::RemoteContralCodeMatchingAck_Proc(uint8_t state)
{
    //获取主控时间
    uint16_t scactiveTimer;
    STR_WLCAN_Frame frame;
    memset(&frame,0,sizeof(STR_WLCAN_Frame));

    scactiveTimer = V4ykq.MasterCtrlTime;
    frame.u32ID.ID.ACK = NOACK;
    frame.u32ID.ID.FrameType = V4YKQ_WL_CODEMATCH_ACK;
    frame.u32ID.ID.LiuShuiNumb = GetLiuShuiNum();
    frame.u32ID.ID.RxID = 3;
    frame.u16DLC = 8;
    frame.u8DT[0] = V4ykq.YkqKey;
    frame.u8DT[1] = V4ykq.supportDirect;
    frame.u8DT[2] = V4ykq.YkqCodingMode == 0?0x00:0x40;
    frame.u8DT[2] |= 0x80;
    frame.u8DT[2] |= state;
    frame.u8DT[3] = V4ykq.YkqSCCrc == V4ykq.YkqCrc?1:0;
    frame.u8DT[4] = V4ykq.SCNum;
    frame.u8DT[5] = V4ykq.YkqNum;
    frame.u8DT[6] = scactiveTimer>>8;
    frame.u8DT[7] = scactiveTimer&0xFF;

    this->RemoteContralSendDataApi(&frame);
}

/*******************************************************************************************
**函数名称：RemoteContralDecondeDeal
**函数作用：遥控器主动解除对码帧
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::RemoteContralDecondeDeal(STR_WLCAN_Frame *frame)
{
    //获取本架架号
    uint8_t scNum;
    scNum = V4ykq.SCNum;

    if(frame->u8DT[3] != scNum)
        return 1;
    if(frame->u8DT[4] != V4ykq.YkqNum)
        return 1;
    if(V4ykq.YkqState != STATE_WL_SUCCESS)
        return 1;

    //发送解除预警
    RemoteContralOverrideWarnin(V4ykq.controllNum,ActionCtrl_CmdType_STOP);

    //发送解除对码状态，更新遥控器状态
    V4ykq.YkqNum = 0;
    V4ykq.controllNum = 0;
    V4ykq.YkqState = STATE_IDLE;
    uint16_t RemoteContralConnectStatus = 3;
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((2<<5)|9),1,11,(unsigned char *)&RemoteContralConnectStatus,sizeof(RemoteContralConnectStatus));
    //停定时器
    Timer::getInstance().stopTimerEvent(timeIdV4DriveDecode);

    return 0;
}

/*******************************************************************************************
**函数名称：RemoteContralActionCtrlDeal
**函数作用：遥控器动作控制帧
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::RemoteContralActionCtrlDeal(STR_WLCAN_Frame *frame)
{

    if(frame->u8DT[0] != V4ykq.YkqNum)
    {
        qDebug("action ctrl ykqNum error,recv ykqid = %d,para ykqid = %d",frame->u8DT[0],V4ykq.YkqNum);
        return 1;
    }
    if(V4ykq.YkqState != STATE_WL_SUCCESS)
    {
        qDebug("action ctrl ykqStatus error");
        return 1;
    }
    //刷新定时器
    Timer::getInstance().startTimerEvent(timeIdV4DriveDecode);
    //动作控制处理
    RemoteActionCtrlDataType ActionData;
    switch(frame->u8DT[2]&0X07)
    {
        case SINGLE_CONTROL:        //单动作
        {
            ActionData.ContralSCNum = V4ykq.controllNum;
            ActionData.ActionID = frame->u8DT[3];
            ActionData.ContralCommand = frame->u8DT[4];
            if((frame->u8DT[2]&0x08) == 0x08)
                ActionData.ActionCtrlMode = ActionCtrl_ModeType_AUTO;   //自动
            else
                ActionData.ActionCtrlMode = ActionCtrl_ModeType_MANUAL; //手动
            ActionData.CtrlSourceAddr = V4ykq.YkqNum;
            ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ActionData.UserID = 0;

            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,0,(unsigned char *)&ActionData,14);
            //手动动作心跳处理
            if(ActionData.ActionCtrlMode == ActionCtrl_ModeType_MANUAL)
            {
                if(ActionData.ContralCommand == ActionCtrl_CmdType_STARTUP)
                {
                    RuningActionInfo |= 1 <<  (ActionData.ActionID - 1);
                    Timer::getInstance().startTimerEvent(timeIdRemoteActionHeart);
                }
                else
                {
                    RuningActionInfo &= 0 <<  (ActionData.ActionID - 1);
                    Timer::getInstance().stopTimerEvent(timeIdRemoteActionHeart);
                }
//                qDebug("RuningActionInfo = 0x%08x",RuningActionInfo);
            }
            //test
            qDebug("----------single control------------");
            qDebug("action id = %d",ActionData.ActionID);
            qDebug("ActionCtrlMode = %d",ActionData.ActionCtrlMode);
            qDebug("ContralCommand = %d",ActionData.ContralCommand);
            qDebug("-----------------------------------");
        }break;
        case COMB_CONTROL:      //组合动作
        {
            ActionData.ContralSCNum = V4ykq.controllNum;
            ActionData.ActionID = frame->u8DT[3];
            ActionData.ContralCommand = frame->u8DT[4];
            ActionData.ActionCtrlMode = ActionCtrl_ModeType_AUTO;
            ActionData.CtrlSourceAddr = V4ykq.YkqNum;
            ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ActionData.UserID = 0;

            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,1,(unsigned char *)&ActionData,14);
            //test
            qDebug("----------comb control------------");
            qDebug("action id = %d",ActionData.ActionID);
            qDebug("ContralCommand = %d",ActionData.ContralCommand);
            qDebug("-----------------------------------");
        }break;
        case GROUP_CONTROL:         //成组动作
        {
            if(frame->u8DT[7] == 0) //从近到远
            {
                ActionData.ContralSCNum = frame->u8DT[5];    //V4ykq.controllNum;
                ActionData.GroupBeginSC = frame->u8DT[5];
                ActionData.GroupEndSc = frame->u8DT[6];
            }
            else {
                ActionData.ContralSCNum = frame->u8DT[6];    //V4ykq.controllNum;
                ActionData.GroupBeginSC = frame->u8DT[6];
                ActionData.GroupEndSc = frame->u8DT[5];
            }
            //小到大1，大到小2
            if(ActionData.GroupBeginSC < ActionData.GroupEndSc)
                ActionData.GroupExecutionDirection = 1;
            else
                ActionData.GroupExecutionDirection = 2;

            ActionData.ActionID = frame->u8DT[3];
            ActionData.ContralCommand = frame->u8DT[4];
            ActionData.ActionCtrlMode = ActionCtrl_ModeType_AUTO;
            ActionData.CtrlSourceAddr = V4ykq.YkqNum;
            ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ActionData.UserID = 0;
            ActionData.DataLength = 6;

            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,2,(unsigned char *)&ActionData,sizeof(RemoteActionCtrlDataType));
            //test
            qDebug("----------Group control------------");
            qDebug("action id = %d",ActionData.ActionID);
            qDebug("ContralCommand = %d",ActionData.ContralCommand);
            qDebug("GroupBeginSC id = %d",ActionData.GroupBeginSC);
            qDebug("GroupEndSc = %d",ActionData.GroupEndSc);
            qDebug("GroupExecutionDirection = %d",ActionData.GroupExecutionDirection);
            qDebug("-----------------------------------");
        }break;
        case FUNCTION_KEY:      //停止键
        {
            //被控架号切换回控制器架号。
            V4ykq.controllNum = V4ykq.SCNum;
            //给逻辑报停止
            uint16_t temp[2];
            temp[0] = 1;
            temp[1] = 1;
            //停止键
            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,99,(unsigned char *)temp,sizeof(temp));
            //test
            qDebug("remote control STOP");
        }break;
        default:
            break;
    }
    this->RemoteContralActionSendAck(frame->u8DT[2],frame->u8DT[3]);

    return 0;
}

/*******************************************************************************************
**函数名称：RemoteContralActionSendAck
**函数作用：遥控器动作控制应答帧
**函数参数：actionType:动作类型， actionid：动作id
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
void wirless::RemoteContralActionSendAck(const uint8_t &actionType, const uint8_t &actionID)
{
    if(V4ykq.YkqNum == 0)
        return;

    STR_WLCAN_Frame frame;
    memset(&frame,0,sizeof(STR_WLCAN_Frame));

    frame.u32ID.ID.ACK = 0;
    frame.u32ID.ID.FrameType = V4YKQ_CONTROL_DATA_ACK;
    frame.u32ID.ID.LiuShuiNumb = GetLiuShuiNum();
    frame.u32ID.ID.RxID = 3;
    frame.u16DLC = 8;
    frame.u8DT[0] = V4ykq.YkqNum;
    frame.u8DT[1] = V4ykq.SCNum;
    frame.u8DT[2] = actionType;
    frame.u8DT[3] = actionID;
    frame.u8DT[4] = 1;
    frame.u8DT[5] = 0;
    frame.u8DT[6] = 0;
    frame.u8DT[7] = 0;

    this->RemoteContralSendDataApi(&frame);
}

/*******************************************************************************************
**函数名称：RemoteContralActionInfoReqDeal
**函数作用：遥控器配置文件同步请求
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::RemoteContralActionInfoReqDeal(STR_WLCAN_Frame *frame)
{
    uint8_t scNum;
    scNum = V4ykq.SCNum;

    if(frame->u8DT[1] != scNum)
        return 0;
    if(frame->u8DT[0] != V4ykq.YkqNum)
        return 0;
    if(V4ykq.YkqState != STATE_WL_SUCCESS)
        return 0;
    //配置文件接收处理
    SendYkqParaProc();
    //同步完成后遥控器会重启因此在此处亭预警，修改状态
    V4ykq.YkqState = STATE_IDLE;
    RemoteContralOverrideWarnin(V4ykq.controllNum,ActionCtrl_CmdType_STOP);  //停预警
    return 0;
}

/*******************************************************************************************
**函数名称：RemoteContralNeighboorCtrlReqDeal
**函数作用：遥控器邻架控制请求
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
char wirless::RemoteContralNeighboorCtrlReqDeal(STR_WLCAN_Frame *frame)
{
    //获取本架架号
    uint8_t scNum;
    uint16_t gapNum;
    scNum = V4ykq.SCNum;
    gapNum = V4ykq.GapSupportNum+1;

    if(frame->u8DT[1] != scNum)
        return 1;
    if(frame->u8DT[0] != V4ykq.YkqNum)
        return 1;
    if(V4ykq.YkqState != STATE_WL_SUCCESS)
        return 1;
    //记录上一次的预警架号
    qDebug("Recv control sc num = %d",frame->u8DT[2]);
    RemoteContralOverrideWarnin(V4ykq.controllNum,ActionCtrl_CmdType_STOP);
    qDebug("stop alarm sc num = %d",V4ykq.controllNum);
    //判断隔架架数
    if(scNum > frame->u8DT[2])
    {
        V4ykq.controllNum = (scNum-frame->u8DT[2] > gapNum)?(V4ykq.SCNum - gapNum):frame->u8DT[2];
    }
    else
    {
        V4ykq.controllNum = (frame->u8DT[2] - scNum > gapNum)?(V4ykq.SCNum + gapNum):frame->u8DT[2];
    }
    //发送邻架控制应答
    RemoteContralNeighborAckProc();
    //发送邻架预警
    RemoteContralOverrideWarnin(V4ykq.controllNum,ActionCtrl_CmdType_STARTUP);
    qDebug("start alarm sc num = %d",V4ykq.controllNum);
    //邻架控制状态上报
    uint16_t RemoteContralConnectStatus = 4;
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((2<<5)|9),1,11,(unsigned char *)&RemoteContralConnectStatus,sizeof(uint16_t));

    return 0;
}

/*******************************************************************************************
**函数名称：RemoteContralNeighboorCtrlReqDeal
**函数作用：遥控器邻架控制请求应答
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void wirless::RemoteContralNeighborAckProc()
{
    if(V4ykq.YkqNum == 0)
        return;

    STR_WLCAN_Frame frame;
    memset(&frame,0,sizeof(STR_WLCAN_Frame));

    frame.u32ID.ID.ACK = 0;
    frame.u32ID.ID.FrameType = V4YKQ_NEIGHBOR_REQUEST_ACK;
    frame.u32ID.ID.LiuShuiNumb = GetLiuShuiNum();
    frame.u32ID.ID.RxID = 3;
    frame.u16DLC = 3;
    frame.u8DT[0] = V4ykq.YkqNum;
    frame.u8DT[1] = V4ykq.SCNum;
    frame.u8DT[2] = V4ykq.controllNum;

    this->RemoteContralSendDataApi(&frame);
}

/*******************************************************************************************
**函数名称：RemoteContralOverrideWarnin
**函数作用：控制器预警控制
**函数参数：ykqID：被控架号，status：0启动，1停止
**函数输出：无
**注意事项：无
*******************************************************************************************/
void wirless::RemoteContralOverrideWarnin(uint8_t ScID, const bool &status)
{
    RemoteActionCtrlDataType ActionData;
    memset(&ActionData,0,sizeof(RemoteActionCtrlDataType));

    ActionData.ContralSCNum = ScID;
    ActionData.ActionID = 1;
    ActionData.ContralCommand = status;
    ActionData.ActionCtrlMode = ActionCtrl_ModeType_AUTO;
    ActionData.CtrlSourceAddr = V4ykq.YkqNum;
    ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
    ActionData.UserID = 0;

    Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,3,(unsigned char *)&ActionData,14);
}

/*******************************************************************************************
**函数名称：sendYkqParaProc
**函数作用：发送遥控器配置文件
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void SendYkqParaProc(void)
{
    char SendBuf[50];
    char len = 0;
    RFID_UPDATE_FRAME_TYPE updateFrame;
    uint16_t paraPt = 0;
    uint32_t ProgSendPacketNumb = 0;
    char V4ykqParaQueue[YKQ_PARA_SIZE];

    int file = open(PathRemoteActionInfo, O_RDONLY);
    if (file == -1) {
        qDebug("RemoteActionInfo open error ");
    }
    ssize_t bytes_read = read(file, V4ykqParaQueue, YKQ_PARA_SIZE);  // 读取遥控器配置文件信息
    close(file);  // 关闭文件

    if (bytes_read == 0) {
        qDebug("RemoteActionInfo read info error");
        wl->V4ykq.YkqState = STATE_IDLE;
        return;
    }
    sleep(2);
    wl->ClearTotalTTL();
    while(paraPt < bytes_read)
    {
        ProgSendPacketNumb++;

        memset(SendBuf,0,50);
        memset(&updateFrame,0,sizeof(RFID_UPDATE_FRAME_TYPE));

        updateFrame.u8DT[1] = ProgSendPacketNumb&0xFF;
        updateFrame.u8DT[2] = (ProgSendPacketNumb>>8)&0xFF;

        if(paraPt==0)
        {
            updateFrame.u32ID.ID.SDL = YKQ_DEV_TYPE&0xFF;
            updateFrame.u32ID.ID.Reservd = 0;
            updateFrame.u8DT[0] = 0xFF - wl->V4ykq.YkqNum;
            updateFrame.u32ID.ID.FrameType = RFID_FT_UPDATE_DWL_VERSION;
            updateFrame.u8DLC = 32+3;
            memcpy(&updateFrame.u8DT[3],V4ykqParaQueue,32);

            len = updateFrame.u8DLC+4+1+1;
            SendBuf[0] = 0xFF - wl->V4ykq.YkqNum;
            memcpy(&SendBuf[1],&updateFrame,len);

            paraPt = 32;
            wl->SendDataDealApi(WIRELESS_DATA,0,SendBuf,len,0);
//            qDebug("send action info verson");
            usleep(500*1000);

            updateFrame.u32ID.ID.FrameType = RFID_FT_UPDATE_WRITE_VERSION;
            updateFrame.u8DT[3] = 1;
            updateFrame.u8DLC  = 4;
            SendBuf[0] = 0xFF - wl->V4ykq.YkqNum;
            len = updateFrame.u8DLC+4+1+1;
            memcpy(&SendBuf[1],&updateFrame, len-1);

            wl->SendDataDealApi(WIRELESS_DATA,0,SendBuf,len,0);
            usleep(500*1000);
        }
        else
        {
            updateFrame.u32ID.ID.SDL = YKQ_DEV_TYPE&0xFF;
            updateFrame.u32ID.ID.Reservd = 0;
            updateFrame.u32ID.ID.FrameType = RFID_FT_UPDATE_DWL_CODE;
            updateFrame.u8DT[0] = 0xFF - wl->V4ykq.YkqNum;

            if(bytes_read > 32 + paraPt)
            {
                memcpy(&updateFrame.u8DT[3],V4ykqParaQueue+paraPt, 32);
                updateFrame.u8DLC = 32+3;
                paraPt+= 32;
            }
            else
            {
                memcpy(&updateFrame.u8DT[3],V4ykqParaQueue+paraPt, bytes_read - paraPt);
                updateFrame.u8DLC = bytes_read - paraPt+3;
                paraPt = bytes_read;
            }
            SendBuf[0] = 0xFF - wl->V4ykq.YkqNum;
            len = updateFrame.u8DLC+4+1+1;
            memcpy(&SendBuf[1],&updateFrame,len-1);

            wl->SendDataDealApi(WIRELESS_DATA,0,SendBuf,len,0);
//            qDebug("send action info code");
            usleep(200*1000);

            if((paraPt-32)%256 == 0 || paraPt == bytes_read)
            {
                updateFrame.u32ID.ID.FrameType = RFID_FT_UPDATE_WRITE_CODE;
                updateFrame.u8DLC = 4;
                updateFrame.u8DT[3] = 1;

                SendBuf[0] = 0xFF - wl->V4ykq.YkqNum;
                len = updateFrame.u8DLC+4+1+1;
                memcpy(&SendBuf[1],&updateFrame,len-1);

                wl->SendDataDealApi(WIRELESS_DATA,0,SendBuf,len,0);
//                qDebug("send action info code write");
                usleep(200*1000);
            }
        }
    }
    updateFrame.u32ID.ID.FrameType = RFID_FT_UPDATE_END;
    updateFrame.u8DLC = 4;
    len = updateFrame.u8DLC+4+1+1;
    SendBuf[0] = 0xFF - wl->V4ykq.YkqNum;
    memcpy(&SendBuf[1],&updateFrame,len-1);

    wl->SendDataDealApi(WIRELESS_DATA,0,SendBuf,len,0);
    qDebug("send action info finish");
    sleep(1);
}

/*******************************************************************************************
**函数名称：RecvMsgCallback_Wlinit
**函数作用：设在无线模块初始化接收处理回调
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvMsgCallback_Wlinit(uint8_t point,uint8_t* data,int len)
{
    (void)point;    //消除警告

    qDebug("wirless init message");
    //更新遥控器状态
    uint16_t temp[6];
    if((unsigned int)len > sizeof(temp))
        return;
    memcpy(temp,data,len);
    wl->V4ykq.SCNum = temp[1];
    wl->V4ykq.supportDirect = temp[2];
    wl->V4ykq.MasterCtrlTime = temp[4];
    wl->V4ykq.GapSupportNum = temp[5];
    //test
    qDebug("SCNum =%d",wl->V4ykq.SCNum);
    qDebug("supportDirect =%d",wl->V4ykq.supportDirect);
    qDebug("MasterCtrlTime =%d",wl->V4ykq.MasterCtrlTime);
    qDebug("GapSupportNum =%d",wl->V4ykq.GapSupportNum);
    qDebug("-----------------------------------------------");
    //读取配置文件crc
    int file = open(PathRemoteActionInfo, O_RDONLY);
    if (file == -1) {
        qDebug("RemoteActionInfo open error ");
    }
    lseek(file, 6, SEEK_SET);  // 定位到指定位置
    read(file, &wl->V4ykq.YkqSCCrc, sizeof(short));  // 读取两个字节的数据
    close(file);  // 关闭文件
    //设在无线同步字
    wl->SetWirlessSyncword(SYNC_ACTION);
    //上报无线设备状态
    uint16_t tdata[6];
    if(wl->OnlineStatus == 0)
    {
        tdata[0] = 0;
        tdata[1] = 0;
        tdata[2] = 0;
        tdata[3] = 0;
        tdata[4] = 0;
        tdata[5] = 0;
    }
    else {
        tdata[0] = 1;
        tdata[1] = 0;
        tdata[2] = ((uint16_t)wl->V4ykq.AppVerson[1])|((uint16_t)wl->V4ykq.AppVerson[0] << 8);
        tdata[3] = ((uint16_t)wl->V4ykq.AppVerson[3])|((uint16_t)wl->V4ykq.AppVerson[2] << 8);
        tdata[4] = 0;
        tdata[5] = 0;
    }

    Ydshrdlib::getInston()->sendShmDataFunc(1,65,((2<<5)|9),1,0,(unsigned char *)tdata,sizeof(tdata));
    //对码状态解除对码
    if(wl->V4ykq.YkqState == STATE_WL_SUCCESS)
        WirelessSCDecode_Proc();
}

/*******************************************************************************************
**函数名称：RecvMsgCallback_WlFileDownLoad
**函数作用：设在无线模块程序更新
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvMsgCallback_WlFileDownLoad(uint8_t point,uint8_t* data,int len)
{
    (void)point;    //消除警告
    uint16_t temp;

    memset(wl->Programename,0,sizeof(wl->Programename));
    memcpy(&temp,data,2);
    if(temp != 3)
    {
        qDebug("programe file type error:%d",temp);
        return;
    }
    memcpy(&temp,data+2,2);
    if(temp != 9)
    {
        qDebug("Updata Drive type error:%d",temp);
        return;
    }
    memcpy(wl->Programename,data+6,len-6);
    //test
    qDebug() << "programe file name:"<< wl->Programename;
    wl->UpdateSem->post_sem();
}
/*******************************************************************************************
**函数名称：RecvShmCallback_Wl
**函数作用：无线共享内存接收处理回调（57，控制驱动）
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RecvShmCallback_Wl(uint8_t point,uint8_t *data, int len)
{
    if(point == 11)
    {
        if((wl->V4ykq.YkqState == STATE_WL_SUCCESS)&&(*data == 3))  //逻辑要求解除对码
        {
            WirelessSCDecode_Proc();
        }
    }

}

/*******************************************************************************************
**函数名称：ProgrameUpdataPro
**函数作用：程序更新线程
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void *wirless::ProgrameUpdataPro(void *arg)
{
    wirless *self;
    self = static_cast<wirless*>(arg);
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
**函数名称：WirlessAppStart
**函数作用：无线逻辑
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void WirlessAppStart()
{
    //实例化无线模块并启动
    wl = new wirless(WirlessDrivename,115200);
    //创建无线相关线程
    wl->ThreadCreat();
    //初始化v4ykq状态
    wl->V4ykq.YkqState = STATE_IDLE;
    //添加无线模块超时定时器,超时9s认为模块掉线
    timerIdWlHeart = Timer::getInstance().addTimerEvent(9000,0xff,WlHeartTimeoutDeal);
    Timer::getInstance().startTimerEvent(timerIdWlHeart);
    //添加定时器，120s无动作主动解除对码
    timeIdV4DriveDecode = Timer::getInstance().addTimerEvent(120*1000,1,WirelessSCDecode_Proc);
    //添加动作心跳定时器，手动单动作200ms收不到心跳停动作
    timeIdRemoteActionHeart = Timer::getInstance().addTimerEvent(300,1,RemoteContralHeartTimeOutCb);

}


