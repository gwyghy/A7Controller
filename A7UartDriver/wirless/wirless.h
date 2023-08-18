#ifndef WIRLESS_H
#define WIRLESS_H

/*********************************************************************************
* 									INCLUDE
*********************************************************************************/
#include "uart/uartbus.h"
#include "uart/UartDataType.h"
#include <QList>
#include "sem/sem.h"
/*********************************************************************************
* 									DEFINES
*********************************************************************************/
#define RFID_UPDATE_LEN     55
/*********************************************************************************
* 									DECLARATIONS
*********************************************************************************/
void WirlessAppStart(void); //无线逻辑
void RecvMsgCallback_Wlinit(uint8_t point,uint8_t* data,int len); //无线初始化消息
void RecvShmCallback_Wl(uint8_t point,uint8_t* data,int len);     //无线共享内存回调函数
void RecvMsgCallback_WlFileDownLoad(uint8_t point,uint8_t* data,int len);   //无线程序更新消息
/*********************************************************************************
* 									GLOBAL DATA
*********************************************************************************/
//无线数据帧类别
enum
{
    WIRELESS_HEART,         //无线心跳
    WIRELESS_DATA,          //遥控器数据
    WIRELESS_STARTHINT,     //初始化
    WIRELESS_RESERVED,
    WIRELESS_PARASET,       //设置无线同步字
    WIRELESS_UPDATE,        //程序更新
    WIRELESS_RESTORATION,

    WIRLESS_FUNC_MAX,
};

//无线同步字
enum
{
    SYNC_ACTION = 1,        //动作控制和同步配置信息
    SYNC_UPDATA,            //程序升级
};
//遥控器数据协议帧类别
enum
{
    V4YKQ_HEART = 0,                //遥控器心跳
    V4YKQ_IR_CODEMATCH = 4,         //遥控器红外对码
    V4YKQ_IR_CODEMATCH_ACK = 5,     //遥控器红外对码应答
    V4YKQ_WL_CODEMATCH = 6,         //无线对码
    V4YKQ_WL_CODEMATCH_ACK = 7,     //无线对码应答
    V4YKQ_PASSIVE_DECODE_ACK = 8,   //遥控器主动解除对码应答
    V4YKQ_CONTROL_DATA = 10,        //动作控制
    V4YKQ_CONTROL_DATA_ACK = 11,    //动作控制应答
    V4YKQ_ACTIVE_DECODE = 14,       //控制器主动解除对码
    V4YKQ_PASSIVE_DECODE = 15,      //遥控器解除对码
    V4YKQ_FILE_REQUEST = 18,        //文件同步请求
    V4YKQ_NEIGHBOR_REQUEST = 19,    //邻架控制请求
    V4YKQ_NEIGHBOR_REQUEST_ACK = 20,//邻架控制应答
};
//遥控器状态
enum
{
    STATE_IDLE,
    STATE_IR_CODEMATCHING,
    STATE_IR_SUCCESS,
    STATE_IR_FAIL,
    STATE_WL_CODEMATCHING,
    STATE_WL_SUCCESS,
    STATE_WL_FAIL,
};

//遥控器数据
typedef struct
{
    uint8_t SCNum;          //控制器编号
    uint8_t YkqNum;         //遥控器编号
    uint8_t YkqState;       //遥控器状态
    uint8_t hwemergencyNum; //急停架号（暂时不需要）
    uint8_t bisuoNum;       //闭锁架号（暂时不需要）
    uint8_t supportDirect;  //架号增向
    uint16_t MasterCtrlTime;//主控时间
    uint16_t GapSupportNum; //隔架架数
    uint8_t YkqKey;         //密钥
    uint8_t YkqCodingMode;  //对码方式  0：正常对码；1：+-键对码
    uint8_t controllNum;    //控制架号
    uint16_t YkqSCCrc;      //本机crc
    uint16_t YkqCrc;        //遥控器crc
    char AppVerson[4];      //遥控器版本信息
}V4YKQ_DATA;
//遥控器控制设备类型
enum
{
    SINGLE_CONTROL = 1,     //单动作
    COMB_CONTROL = 2,       //组合动作
    GROUP_CONTROL = 3,      //成组动作
    FUNCTION_KEY = 4        //功能键
};

//V4控制器动作控制相关类型枚举
typedef enum
{
    ActionCtrl_CmdType_STARTUP=0,   //0直接启动
    ActionCtrl_CmdType_STOP,        //1停止
    ActionCtrl_CmdType_WAITSTARTUP, //2延时启动
    ActionCtrl_CmdType_SUSPEND,     //3暂停
    ActionCtrl_CmdType_RESUME,      //4恢复
    ActionCtrl_CmdType_SUSPEND_ACTION,//5暂停动作，继续计时
    ActionCtrl_CmdTypeMAX
}ActionCtrl_CmdType;

typedef enum
{
    ActionCtrl_ModeType_MANUAL =0,  //手动控制
    ActionCtrl_ModeType_AUTO        //自动控制
}ActionCtrl_ModeType;

typedef enum
{
    ActionCtrl_SourceType_CONTROLLER=0,	//控制器
    ActionCtrl_SourceType_HANDLER,		//遥控器
    ActionCtrl_SourceType_SERVER,		//服务器
    ActionCtrl_SourceType_CONSOLE,		//操作台
    ActionCtrl_SourceType_REMOTE,		//远控
    ActionCtrl_SourceType_MAX
}ActionCtrl_SourceType;

//遥控器动作控制类型
typedef struct
{
    uint16_t ContralSCNum;  //控制架号
    uint16_t ActionID;      //动作id
    uint16_t ContralCommand;//控制命令
    uint16_t ActionCtrlMode;//动作控制方式
    uint16_t CtrlSourceAddr;//控制源地址
    uint16_t CtrlSourceType;//控制源类型
    uint16_t UserID;        //用户id

//    仅在成祖动作控制时下属数据才有用
    uint16_t DataLength;        //数据长度
    uint16_t GroupBeginSC;      //成祖起始架号
    uint16_t GroupEndSc;        //成祖结束架号
    uint16_t GroupExecutionDirection;   //成祖方向

}RemoteActionCtrlDataType;
//V2外围设备can帧格式
typedef struct
{
    union
    {
        struct
        {
            unsigned int	RxID:3;				//接收方ID
            unsigned int	TxID:3;				//发送方ID
            unsigned int	FrameType:10;		//帧类别
            unsigned int	Sum:4;              //子帧序号或
            unsigned int	LiuShuiNumb:5;		//流水序列号，0x0-0xf循环，ACK=1应答帧的流水，采用被应答帧的流水
            unsigned int	Sub:1;              //组合帧/子帧，0：组合帧总数帧：1：子帧
            unsigned int	ACK:1;				//应答标志，1:应答，0:无需应答
            unsigned int	Reservd:2;			//预留区域。用于传程序:01,其他:00
            unsigned int	NoUsed:3;			//不存在区域
        } ID;   //帧ID
        uint32_t u32Id;//帧ID
    } u32ID;	//帧ID
    uint8_t	u16DLC;		//帧长度
    uint8_t	u8DT[8];	//帧数据

} STR_WLCAN_Frame;

/**遥控器配置文件同步和无线模块程序更新数据格式***/
typedef struct
{
    union
    {
        struct
        {
            unsigned int	SDL:6;				//接收方ID、发送方ID，共计6bit。
            unsigned int	FrameType:10;		//帧类别
            unsigned int	Reservd:16;			//预留区域。
        } ID;//帧ID
        uint32_t u32Id;//帧ID
    } u32ID;	//帧ID
    uint8_t	u8DLC;				//帧长度
    uint8_t	u8DT[RFID_UPDATE_LEN];			//帧数据
}RFID_UPDATE_FRAME_TYPE;

/**配置文件同步帧类型定义**/
enum
{
    RFID_FT_UPDATE_DWL_VERSION = 0x1DF,//传输版本信息
    RFID_FT_UPDATE_WRITE_VERSION = 0x1DE,//写入版本信息
    RFID_FT_UPDATE_DWL_CODE = 0x1DD,//传输程序代码
    RFID_FT_UPDATE_WRITE_CODE = 0x1DC,//写入版本信息
    RFID_FT_UPDATE_END = 0x1DB	//更新过程结束
};

/*********************************************************************************
* 									CLASS
*********************************************************************************/
class wirless:public UartBus
{
public:
    wirless(const char*Name,int Baud);
    ~wirless();
    void ThreadCreat(void);           //应用启动
    virtual char UartTrsProtocolDeal(__uint8_t funcid ,stUartTrsQueue *pframe);      //无线发送协议处理
    virtual char UartRecvProtocolDeal(UartFrame *pframe);                           //无线接收协议处理
    void RemoteContralSendDataApi(STR_WLCAN_Frame *pframe);                         //遥控器发送数据接口
    void RemoteContralIrRecvCBFunc(char &ykqnum);                                   //遥控器红外对码接收处理
    inline __uint8_t GetLiuShuiNum(void);                                           //遥控器CAN帧获取流水号
    inline void SetWirlessSyncword(uint8_t mode);                                   //设置无线同步字
    char SendProgrameUpdateProc(char* name);                                        //程序更新
    static void *ProgrameUpdataPro(void *arg);                                      //程序更新线程
protected:
    static char WirlessRecvHeartDealCb(UartFrame *pframe,void*arg);                 //无线接收心跳数据处理函数
    static char WirlessRecvDataDealCb(UartFrame *pframe,void*arg);                  //无线接收数据处理函数
    static char WirlessRecvStartInitDealCb(UartFrame *pframe,void*arg);             //设置无线同步码

    inline void AddLiuShuiNum(void);              //遥控器CAN帧流水号自增
    inline void ClearLiuShuiNum(void);            //遥控器CAN帧流水号重置
    char RemoteContralHeartDeal(STR_WLCAN_Frame *pframe);               //遥控器心跳处理
    char RemoteContralIrRecvDeal(STR_WLCAN_Frame *pframe);              //红外对码接收处理
    char RemoteContralColibarayeDeal(STR_WLCAN_Frame *pframe);          //遥控器无线对码处理
    void RemoteContralCodeMatchingAck_Proc(uint8_t state);              //遥控器无线对码结果应答
    char RemoteContralDecondeDeal(STR_WLCAN_Frame *pframe);             //遥控器主动解除对码处理
    char RemoteContralActionCtrlDeal(STR_WLCAN_Frame *pframe);          //遥控器动作处理
    void RemoteContralActionSendAck(const uint8_t &actionType,const uint8_t &antionID);    //动作控制应答
    char RemoteContralActionInfoReqDeal(STR_WLCAN_Frame *pframe);       //遥控器同步配置信息请求
    char RemoteContralNeighboorCtrlReqDeal(STR_WLCAN_Frame *pframe);    //遥控器邻架控制请求
    void RemoteContralNeighborAckProc(void);                            //遥控器邻架控制请求应答
public:
    void RemoteContralOverrideWarnin(uint8_t ScID,const bool &status);  //预警控制
private:
    QList <DataProcType> WirlessProtocolFunc;    //无线数据处理方法
    unsigned char LiushuiNum;                   //遥控器流水号
public:
    V4YKQ_DATA V4ykq;                           //遥控器状态数据
    char OnlineStatus;                          //无线模块在线标志
    semt *UpdateSem;                            //程序更新信号量
    pthread_t UpdatePthreadID;              //程序更新线程id
    char Programename[100];                 //存放程序更新名
};

#endif // WIRLESS_H
