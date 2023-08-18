/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "ir.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <QDebug>
#include <fcntl.h>
#include "ydshrdlib.h"
#include "wirless/wirless.h"
#include "timer/timer.h"
/*********************************************************************************
* 									DEFINES
*********************************************************************************/
#define IR_RX_INFO_HEAD		0x81	//IR红外接收遥控器对码数据的起始标志
#define IR_RX_MINE_HEAD		0x01	//IR红外接收红外发射器数据的起始标志
#define IR_RX_USER_DATA		0x00    //第二个字节
#define IR_RX_MINE_END		0xFE	//IR红外接收红外发射器数据的结束标志
#define IR_RX_INFO_LENGTH	0x03	//IR接收数据一个完整帧数据长度

/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/
//红外类对象
ir *infrared = nullptr;
//设备名
#ifdef PLATFORM_TYPE        //arm版
constexpr char IrDrivename[] = "/dev/ttymxc4";
#else                       //pc版
constexpr char IrDrivename[] = "/dev/ttyUSB5";
#endif
//无线类对象
extern wirless *wl;
bool IrSendingStatus = false;       //红外发送状态
uint8_t Irsendtimeid = 0xff;        //红外发送定时器
/*********************************************************************************
* 									申明
*********************************************************************************/


/*********************************************************************************
* 									FUNCTION
*********************************************************************************/
/*******************************************************************************************
**函数名称：ir
**函数作用：构造函数
**函数参数：Name：设备名 Baud：波特率
**函数输出：无
**注意事项：无
*******************************************************************************************/
ir::ir(const char *Name, int Baud):UartDrive(Name,Baud)
{
    memset(&IrRxBuf,0,sizeof(IR_Rx_Buf_Type));
    memset(&u32IRId,0,sizeof(unsigned char)*IR_DEV_MAX);
    memset(&u32IRData,0,sizeof(unsigned char)*IR_DEV_MAX);
    memset(&u32IRDataCtr,0,sizeof(unsigned int)*IR_DEV_MAX);
    IrSendInit();
}
/*******************************************************************************************
**函数名称：～ir
**函数作用：析构函数
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
ir::~ir()
{

}

/*******************************************************************************************
**函数名称：ThreadCreat
**函数作用：红外应用启动
**函数参数：无
**函数输出：无
**注意事项：主线程中先实例化，再调用此函数即可
*******************************************************************************************/
void ir::ThreadCreat()
{
    int res = 0;
    pthread_attr_t attr;
    size_t stack_size = 1 *64 * 1024; // 设置 64kB 的堆栈大小

    /* 初始化线程属性对象 */
    pthread_attr_init(&attr);

    /* 设置线程堆栈大小属性 */
    pthread_attr_setstacksize(&attr, stack_size);
    //创建驱动线程
    res |= pthread_create(&Drivethreadid,&attr,DriveThreadFunc,this);
    res |= pthread_create(&Irthreadid,&attr,IrThreadFunc,this);

    if(res != 0)
    {
        perror("pthread Creat errot");
    }
}

/*******************************************************************************************
**函数名称：Recived
**函数作用：纯虚函数，获取底层数据
**函数参数：pdata：数据 Length：长度
**函数输出：无
**注意事项：0：成功 ！0失败
*******************************************************************************************/
char ir::Recived(char *pdata, unsigned short *Length)
{
    memcpy(&IrRxBuf.IR_RxBuf[IrRxBuf.IR_RxBufWpt],pdata,*Length);
    IrRxBuf.IR_RxBufWpt += *Length;
    IrRxBuf.IR_RxBufWpt %= IR_RX_BUF_MAX;
    IrRxBuf.IR_RxBufcnt += *Length;

    return 0;
}
/*******************************************************************************************
**函数名称：Send
**函数作用：纯虚函数，将数据放入底层缓存
**函数参数：pdata：数据 Length：长度
**函数输出：0：成功 ！0失败，
**注意事项：红外不通过uart发送，不实现
*******************************************************************************************/
char ir::Send(char *pdata,unsigned short Length)
{
    (void)*pdata;   // 使用类型转换将未使用的参数转换为空操作，解决警告
    (void)Length;   // 使用类型转换将未使用的参数转换为空操作，解决警告
    return 0;
}
/*******************************************************************************************
**函数名称：IrThreadFunc
**函数作用：红外数据处理线程
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void *ir::IrThreadFunc(void *arg)
{
    ir * self;
    self = static_cast<ir *>(arg);

    for(;;)
    {
        //红外数据接收处理
        self->IR_RxDataProc();
        //向逻辑发送接收的红外次数（1s一次）
        self->SendIrMessForLogic(100);

        usleep(10000);
    }

    return nullptr;
}

/*******************************************************************************************
**函数名称：IrSendInit
**函数作用：红外发射初始化
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void ir::IrSendInit()
{
    char path[] = "/dev/tsop38x";

    fd = open(path, O_WRONLY);
    if(fd < 0)
    {
        qDebug("ir send drive open error");
        return;
    }
    qDebug("ir send drive init");
}

/*******************************************************************************************
**函数名称：IrSendData
**函数作用：红外发射接口
**函数参数：data：数据
**函数输出：无
**注意事项：无
*******************************************************************************************/
char ir::IrSendData(uint32_t &data)
{
    int res = write(fd, &data, 4);
    if(res < 0)
    {
        qDebug("ir send data write error");
        return 1;
    }

    qDebug("ir send data: 0x%08x",data);
    return 0;
}

/*******************************************************************************************
**函数名称：IrRxAddIrInf
**函数作用：更新红外接收次数，状态
**函数参数：🆔id 设备号，status 设备状态，times 接收次数
**函数输出：无
**注意事项：无
*******************************************************************************************/
void ir::IrRxAddIrInf(unsigned char id, unsigned char status)
{
    if(id > IR_DEV_MAX)
        return;

    u32IRId[id - 1] = id;
    u32IRData[id - 1] += status;//红外模块之间通讯正常与否，否为0x1，正常0x0;最终发送的u32IRData 是一次上报间隔期间不正确的通讯
    u32IRDataCtr[id - 1]++;
}

/*******************************************************************************************
**函数名称：IrRxClearIrInf
**函数作用：清除红外接收次数，状态
**函数参数：🆔id 设备号，
**函数输出：无
**注意事项：无
*******************************************************************************************/
void ir::IrRxClearIrInf(unsigned char id)
{
    if(id > IR_DEV_MAX)
        return;

    u32IRId[id - 1] = 0x00;
    u32IRData[id - 1] = 0x00;
    u32IRDataCtr[id - 1] = 0x00;
}
/*******************************************************************************************
**函数名称：IrRxGetIrDevId
**函数作用：获取设备id
**函数参数：🆔id 设备号，
**函数输出：无
**注意事项：无
*******************************************************************************************/
unsigned char ir::IrRxGetIrDevId(unsigned char id)
{
    //目前只有一个红外接收设备
    return 0x01;

    if(id > IR_DEV_MAX)
        return 0xFF;

    return u32IRId[id - 1];
}
/*******************************************************************************************
**函数名称：IrRxGetIrRevStatus
**函数作用：获取设备状态
**函数参数：🆔id 设备号，
**函数输出：无
**注意事项：无
*******************************************************************************************/
unsigned char ir::IrRxGetIrRevStatus(unsigned char id)
{
    if(id > IR_DEV_MAX)
        return 0xFF;

    return u32IRData[id - 1];
}

/*******************************************************************************************
**函数名称：IrRxGetIrRevTimes
**函数作用：获取红外接收次数
**函数参数：🆔id 设备号，
**函数输出：接收次数
**注意事项：无
*******************************************************************************************/
unsigned int ir::IrRxGetIrRevTimes(unsigned char id)
{
    if(id > IR_DEV_MAX)
        return 0xFF;

    return u32IRDataCtr[id - 1];
}
/*******************************************************************************************
**函数名称：IR_RxDataProc
**函数作用：红外数据接收处理
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void ir::IR_RxDataProc()
{
    __uint16_t	u16i, u16j;
    IR_INFO_u RxIrInfo;
    u16j = IrRxBuf.IR_RxBufcnt;

    while (u16j >= IR_RX_INFO_LENGTH)
    {
        //煤机定位数据
         u16i = IrRxBuf.IR_RxBufRpt;
        if((IrRxBuf.IR_RxBuf[(u16i) % IR_RX_BUF_MAX] == IR_RX_MINE_HEAD)\
            &&(IrRxBuf.IR_RxBuf[(u16i+1) % IR_RX_BUF_MAX] == IR_RX_USER_DATA)\
            &&(IrRxBuf.IR_RxBuf[(u16i+2) % IR_RX_BUF_MAX] == IR_RX_MINE_END))
        {
            //更新煤机定位数据
            IrRxAddIrInf(IR_DEV_MAX,RxIrInfo.u8IrInfo[2]);
            //更新读指针
            IrRxBuf.IR_RxBufRpt = u16i+2;
            IrRxBuf.IR_RxBufcnt = IrRxBuf.IR_RxBufcnt - IR_RX_INFO_LENGTH;
            u16j = IrRxBuf.IR_RxBufcnt;
            continue;
        }
        //遥控器红外对码数据
        else if((IrRxBuf.IR_RxBuf[(u16i) % IR_RX_BUF_MAX] == IR_RX_INFO_HEAD)\
                &&(IrRxBuf.IR_RxBuf[(u16i+1) % IR_RX_BUF_MAX] == IR_RX_USER_DATA)\
                &&(IrRxBuf.IR_RxBuf[(u16i+2) % IR_RX_BUF_MAX] != IR_RX_MINE_END))
        {
            memcpy(&RxIrInfo.u8IrInfo[0],&IrRxBuf.IR_RxBuf[u16i],IR_RX_INFO_LENGTH);
            //正处于发送状态禁止接收，
            if(IrSendingStatus == false)
            {
                char temp = RxIrInfo.sIrInfo.RemoteID;
                wl->RemoteContralIrRecvCBFunc(temp);    //遥控器红外接收处理
            }
            //更新读指针
            IrRxBuf.IR_RxBufRpt = u16i+2;
            IrRxBuf.IR_RxBufcnt = IrRxBuf.IR_RxBufcnt - IR_RX_INFO_LENGTH;
            u16j = IrRxBuf.IR_RxBufcnt;
            continue;
        }
        else
        {
            IrRxBuf.IR_RxBufRpt++;
            IrRxBuf.IR_RxBufRpt %= IR_RX_BUF_MAX;
            if(IrRxBuf.IR_RxBufcnt > 0)
            {
                IrRxBuf.IR_RxBufcnt--;
                u16j = IrRxBuf.IR_RxBufcnt;
            }
            else
            {
                u16j = 0x00;
            }
        }
    }
    //读写指针对齐，小于3个数会丢弃，防止老数据影响煤机定位结果
    IrRxBuf.IR_RxBufRpt = IrRxBuf.IR_RxBufWpt;
    IrRxBuf.IR_RxBufcnt = 0;
}
/*******************************************************************************************
**函数名称：SendIrMessForLogic
**函数作用：向逻辑层发送煤机定位数据
**函数参数：delay 发送间隔时间
**函数输出：无
**注意事项：delay与任务调度周期有关
*******************************************************************************************/
void ir::SendIrMessForLogic(unsigned short delay)
{
    static unsigned short timer = 0;

    if(timer < delay)
    {
        timer++;
        return;
    }
    timer = 0;

    //获取数据，非0则发送
    unsigned short temp[3];
    temp[0] = this->IrRxGetIrDevId(1);
    temp[1] = this->IrRxGetIrRevStatus(1);
    temp[2] = this->IrRxGetIrRevTimes(1);

    if((temp[2] == 0) || (temp[2] == 0xff))
        return;
    //上报煤机定位数据后清缓存
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((4<<5)|17),1,1,(unsigned char *)temp,6);
    qDebug("recv mine cont 1 sec = %d",temp[2]);
    IrRxClearIrInf(1);
}

/*******************************************************************************************
**函数名称：EstimateIrSendingTime
**函数作用：估算红外发送时间
**函数参数：irsenddata：发送的红外数据
**函数输出：预估的红外发送时间
**注意事项：无
*******************************************************************************************/
uint8_t EstimateIrSendingTime(uint32_t irsenddata)
{
    uint8_t count = 0;
    //Brian Kernighan 计算发送数据中1的个数
    while (irsenddata) {
        irsenddata &= irsenddata - 1;
        count++;
    }
    //总时间 = 逻辑1时间+逻辑0时间+引导时间+结束时间+预估5ms
    return ((count*(560+560*3) + (32-count)*(560+560) + (9000+4500)*2)/1000 + 5);
}

/*******************************************************************************************
**函数名称：IrSendFinishTimeCb
**函数作用：红外发送完成定时器回调
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void IrSendFinishTimeCb(void)
{
    IrSendingStatus = false;
    Timer::getInstance().deleteTimerEvent(Irsendtimeid);
}

/*******************************************************************************************
**函数名称：RemoteContralIRSendConnectAck
**函数作用：遥控器红外对码应答
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RemoteContralIRSendConnectAck(const V4YKQ_DATA &temp)
{
    IR_INFO_u data;

    data.sIrInfo.Type = 2;									//命令类型
    data.sIrInfo.ScNoLSB3 = temp.SCNum & 0x07;              //对码架号低3位
    data.sIrInfo.Sign1 = 1;									//起始标识
    data.sIrInfo.ScNoMSB6 = (temp.SCNum & 0x01f8) >> 3;     //对码架号低6位
    data.sIrInfo.Dir = 1;									//发送方向
    data.sIrInfo.Sign2 = 0;									//中间字节标识
    data.sIrInfo.Result = 0x01;                             //对码结果
    data.sIrInfo.ACK = 0;									//应答标识
    data.sIrInfo.RemoteID = temp.YkqNum;                    //遥控器ID
    data.sIrInfo.ScNoIncDir = temp.supportDirect;           //支架控制器架号增向
    data.sIrInfo.Sign3 = 0;									//中间字节标识

    //第四个字节是第三个字节的反码
    uint32_t irdata;
    irdata = (static_cast<uint32_t> (~data.u8IrInfo[2])) & 0x000000ff;
    irdata |= ((static_cast<uint32_t> (data.u8IrInfo[2])) & 0x000000ff) << 8;
    irdata |= ((static_cast<uint32_t> (data.u8IrInfo[1])) & 0x000000ff) << 16;
    irdata |= ((static_cast<uint32_t> (data.u8IrInfo[0])) & 0x000000ff) << 24;
    infrared->IrSendData(irdata);
    //设置发送标志
    IrSendingStatus = true;
    //添加红外发送定时器，发送期间禁止接收，
    Irsendtimeid = Timer::getInstance().addTimerEvent(EstimateIrSendingTime(irdata),1,IrSendFinishTimeCb);
    Timer::getInstance().startTimerEvent(Irsendtimeid);
}

/*******************************************************************************************
**函数名称：InfraredAppStart
**函数作用：红外逻辑
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
void InfraredAppStart()
{
    //实例化红外并启动
    infrared = new ir(IrDrivename,9600);
    //创建相关线程
    infrared->ThreadCreat();
}
