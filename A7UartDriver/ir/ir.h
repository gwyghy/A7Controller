#ifndef IR_H
#define IR_H
/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "uart/uartdrive.h"
#include "wirless/wirless.h"
/*********************************************************************************
* 									DEFINES
*********************************************************************************/
#define IR_DEV_MAX          0x01	//红外发射设备的最大数量
#define IR_RX_BUF_MAX		64		//IR接收缓冲区长度
#define IR_TX_BUF_MAX		0		//IR发送缓冲区长度

/*********************************************************************************
* 									DECLARATIONS
*********************************************************************************/
void InfraredAppStart(void);    //红外逻辑
void RemoteContralIRSendConnectAck(const V4YKQ_DATA &temp); //红外对码应答
/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/

//IR红外协议帧结构
typedef struct{
    unsigned int Type:4;
    unsigned int ScNoLSB3:3;
    unsigned int Sign1:1;
    unsigned int ScNoMSB6:6;
    unsigned int Dir:1;
    unsigned int Sign2:1;

    unsigned int Result:1;
    unsigned int ACK:1;
    unsigned int RemoteID:4;
    unsigned int ScNoIncDir:1;
    unsigned int Sign3:1;
} IR_INFO_s;
//IR红外协议帧联合
typedef union{
    IR_INFO_s	sIrInfo;
    unsigned char	u8IrInfo[3];
} IR_INFO_u;
//IR数据缓存结构
typedef struct
{
    unsigned char IR_RxBuf[IR_RX_BUF_MAX];
    unsigned char IR_RxBufWpt;
    unsigned char IR_RxBufRpt;
    unsigned char IR_RxBufcnt;
} IR_Rx_Buf_Type;

/*********************************************************************************
* 									CLASS
*********************************************************************************/
class ir : public UartDrive
{
public:
    ir(const char*Name,int Baud);
    ~ir();
    void ThreadCreat(void);
    virtual char Recived(char *pdata,unsigned short *Length);   //串口驱动对外接收接口
    virtual char Send(char *pdata,unsigned short Length);       //串口驱动对外发送接口
    static void *IrThreadFunc(void *arg);                       //红外数据处理线程函数
    void IrSendInit(void);                                      //红外发射初始化
    char IrSendData(uint32_t &data);                            //红外发射接口
protected:
    void IrRxAddIrInf(unsigned char id,unsigned char status);       //更新红外接收次数，状态
    void IrRxClearIrInf(unsigned char id);                          //清除红外接收次数
    unsigned char IrRxGetIrDevId(unsigned char id);                 //获取红外设备id
    unsigned char IrRxGetIrRevStatus(unsigned char id);             //获取红外设备状态
    unsigned int IrRxGetIrRevTimes(unsigned char id);               //获取接收次数
    void IR_RxDataProc(void);                                       //红外数据接收处理
    void SendIrMessForLogic(unsigned short delay);                  //向逻辑发送红外煤机定位
public:
    pthread_t Irthreadid;                       //红外数据处理线程id
private:
    IR_Rx_Buf_Type IrRxBuf;                     //红外接收数据缓存
    unsigned char   u32IRId[IR_DEV_MAX];        //红外发射的设备号
    unsigned char   u32IRData[IR_DEV_MAX];      //红外接收的用户数据，0x00正常，0x01不正常
    unsigned int    u32IRDataCtr[IR_DEV_MAX];   //红外接收计数
    int fd;     //红外发射文件描述符
};

#endif // IR_H
