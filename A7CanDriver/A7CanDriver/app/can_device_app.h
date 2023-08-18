#ifndef __CAN_DEVICE_APP_H__
#define __CAN_DEVICE_APP_H__

#include <map>

#define EMVD_HEART_TIMER  3000
#define CXB_HEART_TIMER   3000
#define WL_HEART_TIMER    3000
#define ANGLE_HEART_TIMER 10000
#define HUB_HEART_TIMER   10000
#define ALARM_HEART_TMIER 10000


#define WL_REPORT_TIMER     1000
#define ANGLE_REPORT_TIMER  100
#define HUB_REPORT_TIMER    100
#define HIGH_REPORT_TMIER   100

using namespace std;

enum{
    DEVICE_OFFLINE,
    DEVICE_ONLINE
};

enum{
    DEVICE_STATE_NORMAL,
    DEVICE_STATE_INIT,
    DEVICE_STATE_PROGRAM
};


typedef enum{
//    STATE_DRIVER,
//    STATE_DEVICE,
//    STATE_SUBDEVICE,
    STATE_ISONLINE,
    STATE_HEARTTIMER,
    STATE_REPORTTIMER,
    STATE_REPORTSTATE,
    STATE_DEVICESTATUS,
    STATE_DEVICEVERSION,
    STATE_DEVICEBLVERSION
}DeviceStateType;


enum{
    STATE_MODE_NORMAL,
    STATE_MODE_REPORT
};

enum{
    DEVICENORMAL = 0,
    DEVICEINIT = 1,
    DEVICEPROGRAM = 2,
};


typedef struct
{
//    unsigned short driver;//驱动
//    unsigned short device;//设备
//    unsigned short subDevice;//子设备
    unsigned short isOnline;//是否在线
    unsigned short HeartTimer;
    unsigned short ReportTimer;
    unsigned short ReportState;
    unsigned short deviceStatus;//外设当前状态（初始化、正常、更新程序等）
    unsigned short deviceVersion[4];
    unsigned short deviceBLVersion[4];
}
st_DeviceStatusMsg;


//设备RID,TID
enum
{
    ID_SC,
    ID_EMVD = 1,						//电磁阀驱动器1
    ID_CXB = 2,							//程序棒2
    ID_ANGLE = 3,						//角度传感器5
    ID_HIGH = 4,						//高度4
    ID_ALARM = 5,						//声光报警器6
    ID_PERSON = 6,                      //人员7
    ID_IRRX = 7,						//红外接收
    ID_WIRELESS = 9,
    ID_MINIHUB1 = 10,
    ID_MINIHUB2 = 11,
    ID_MINIHUB3 = 12,
    ID_HIGHANGLE = 13,                  //角高一体
    ID_ALARMANGLE= 14,                  //多功能声光

    ID_ALARM_BL = 0x8C,
    ID_PERSON_BL = 0x87,
    ID_HIGH_BL = 0x8B,
    ID_DEVICEMAX
};


//设备RID,TID
enum
{
    V2ID_SC = 0,				 //支架控制器
    V2ID_EMVD = 1,				 //电磁阀驱动器1
    V2ID_HUB =2,				 //HUB汇接器 ghy
    V2ID_WL = 3,				 //红外接收
    V2ID_CXB = 4,				 //程序棒2
    V2ID_ANGLE = 5,				 //角度传感器5
    V2ID_HIGH = 5,            	 //高度4
    V2ID_SS = 6,				 //支架服务器5
    V2ID_ALARM = 7,		      	 //声光报警器6
    V2ID_PERSON = 7,             //人员7
    V2ID_DEVICEMAX
};



typedef enum{
    APP_EMVD_IO,
    APP_EMVD_STATE,
    APP_EMVD_CHECKIO,
    APP_EMVD_FLAG,
    APP_EMVD_ACTIONTIMER
}EMVD_APP;

enum
{
    EMVD_OPEN = 1,
    EMVD_CLOSE = 2
};

//电磁阀
typedef struct
{
        unsigned int iovalue;//action
        unsigned short state;//阀检测状态
        unsigned int checkio;
        unsigned short flag;
        unsigned short actionTimer;//动作心跳维持时间
        st_DeviceStatusMsg deviceStatusMsg;

}EmvdMsgInfo;


enum
{
    V2_ANGLE_HEART = 0x30,
    V2_HIGH_HEART = 0x48,
    V2_HIGHBL_HEART = 0x70,
    V2_ALARM_HEART = 0x30,
    V2_ALARMBL_HEART = 0x60,
    V2_PERSON_HEART = 0x31,
    V2_PERSONBL_HEART = 0x61
};


//角度
typedef struct
{
    unsigned short angle_x;
    unsigned short angle_y;
    unsigned short angle_z;
    st_DeviceStatusMsg deviceStatusMsg;
}
AngleMsgInfo;

typedef enum
{
    ANGLE_X,
    ANGLE_Y,
    ANGLE_Z
}ANGLE_APP;


#define  UWBPERSONIDMIN     300
#define  UWBMINEMIN         289
#define  UWBMINEMAX         299
#define  UWBDYKMIN          250


enum
{
    PERSONINTO = 1,
    PROSONLEFT = 2,
    PERSONFAULT = 3
};

//人员
typedef struct
{
    unsigned short   person;//编号
    unsigned short   personPer;//权限
    unsigned short   personNum;//id
    unsigned short   personDis;//距离
    unsigned short   personTimeout;//数据超时时间
}
PersonAppMsg;

typedef struct
{
    std::map<unsigned short,PersonAppMsg>   personAppMsg;
    st_DeviceStatusMsg deviceStatusMsg;
}
PersonMsgInfo;

//红外接收
typedef struct
{
    unsigned short irCounts;
    st_DeviceStatusMsg deviceStatusMsg;
}
IRMsgInfo;

//高度
typedef struct
{
    unsigned short high;
    st_DeviceStatusMsg deviceStatusMsg;
}
HighMsgInfo;


enum
{
    ALARMMODE = 2,
    PEERMODE = 1
};


enum
{
    BLUE_RED_NEVER  = 0,
    BLUE_RED_ALWAYS = 1,
    RED_ALWAYS      = 3,
    BLUE_ALWAYS     = 2,
    RED20_BLUE20    = 4,
};

typedef struct
{
    unsigned short alarmMode;
    unsigned short peerMode;
    st_DeviceStatusMsg deviceStatusMsg;
}
AlarmMsgInfo;


typedef struct
{
    st_DeviceStatusMsg deviceStatusMsg;
}
CxbMsgInfo;


enum
{
    MINIHUB_ANGLOG1 = 0,
    MINIHUB_ANGLOG2 = 1,
    MINIHUB_ANGLOG3 = 2,
    MINIHUB_ANGLOG4 = 3,
    MINIHUB_X1      = 4,
    MINIHUB_Y1      = 5,
    MINIHUB_Z1      = 6,
    MINIHUB_X2      = 7,
    MINIHUB_Y2      = 8,
    MINIHUB_Z2      = 9,
    MINIHUB_X3      = 10,
    MINIHUB_Y3      = 11,
    MINIHUB_Z3      = 12,
    MINIHUB_X4      = 13,
    MINIHUB_Y4      = 14,
    MINIHUB_Z4      = 15,
};

//MINIHUB
typedef struct
{
    unsigned short analog1;
    unsigned short analog2;
    unsigned short analog3;
    unsigned short analog4;
    AngleMsgInfo  AngleMsg[4];
    st_DeviceStatusMsg deviceStatusMsg;
}
MiniHubMsgInfo;



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


enum
{
    WIRELESS_YKQNUM      = 0,
    WIRELESS_CODESTATE   = 1,
    WIRELESS_RSSI        = 2,
    WIRELESS_ACTIONTYPE  = 3,
    WIRELESS_ACTIONID    = 4,
    WIRELESS_YKQCRC      = 5,
    WIRELESS_CONTROLLNUM = 6,
    WIRELESS_CODEINGTIMER= 7,

};

//(0未对码1红外对码2无线对码3解除对码4本架控制过程中5邻隔架控制过程中


enum
{
    WIRELESS_STATE_NOMATCH       = 0,
    WIRELESS_STATE_IRMATCH       = 1,
    WIRELESS_STATE_WLMATCH       = 2,
    WIRELESS_STATE_DEMATCH       = 3,
    WIRELESS_STATE_SELFCONTROLL  = 4,
    WIRELESS_STATE_NERCONTROLL   = 5,
};


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
    unsigned short ContralSCNum;  //控制架号
    unsigned short ActionID;      //动作id
    unsigned short ContralCommand;//控制命令
    unsigned short ActionCtrlMode;//动作控制方式
    unsigned short CtrlSourceAddr;//控制源地址
    unsigned short CtrlSourceType;//控制源类型
    unsigned short UserID;        //用户id

//    仅在成祖动作控制时下属数据才有用
    unsigned short DataLength;        //数据长度
    unsigned short GroupBeginSC;      //成祖起始架号
    unsigned short GroupEndSc;        //成祖结束架号
    unsigned short GroupExecutionDirection;   //成祖方向

}RemoteActionCtrlDataType;



//WIRELESS
typedef struct
{
    unsigned short ykqNum;
    unsigned short codeState;
    unsigned short wirelessRSSI;
    unsigned short actionType;
    unsigned int actionID;
    unsigned short ykqCrc;
    unsigned short controllNum;
    unsigned short codeingTimer;
    st_DeviceStatusMsg deviceStatusMsg;
}
WirelessMsgInfo;


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





typedef unsigned int DeviceIndex;

typedef map<DeviceIndex,EmvdMsgInfo>    QEmvdMsgInfoMap;
typedef map<DeviceIndex,AngleMsgInfo>   QAngleMsgInfoMap;
typedef map<DeviceIndex,PersonMsgInfo>  QPersonMsgInfoMap;
typedef map<DeviceIndex,IRMsgInfo>      QIRMsgInfoMap;
typedef map<DeviceIndex,HighMsgInfo>    QHighMsgInfoMap;
typedef map<DeviceIndex,AlarmMsgInfo>   QAlarmMsgInfoMap;
typedef map<DeviceIndex,CxbMsgInfo>     QCxbMsgInfoMap;
typedef map<DeviceIndex,MiniHubMsgInfo> QMiniHubMsgInfoMap;
typedef map<DeviceIndex,WirelessMsgInfo> QWirelessMsgInfoMap;

class DeviceInfo{
private:
    static DeviceInfo* deviceInfo;

    QEmvdMsgInfoMap    emvdMap;
    QAngleMsgInfoMap   angleMap;
    QPersonMsgInfoMap  personMap;
    QIRMsgInfoMap      irMap;
    QHighMsgInfoMap    highMap;
    QAlarmMsgInfoMap   alarmMap;
    QCxbMsgInfoMap     cxbMap;
    QMiniHubMsgInfoMap mini1Map;
    QMiniHubMsgInfoMap mini2Map;
    QMiniHubMsgInfoMap mini3Map;
    QAngleMsgInfoMap   alarmAngleMap;
    QAngleMsgInfoMap   highAngleMap;
    QWirelessMsgInfoMap wirelessMap;

    DeviceInfo() {
       }
    ~DeviceInfo(){
    }
public:
    static DeviceInfo& getInstance();

    void insert(uint32_t key,EmvdMsgInfo value);
    int find_emvd_value(uint32_t key);
    void insert(uint32_t key, CxbMsgInfo value);
    int find_cxb_value(uint32_t key);
    void insert_angle(uint32_t key, AngleMsgInfo value);
    void insert_alarmAngle(uint32_t key, AngleMsgInfo value);
    void insert_highAngle(uint32_t key, AngleMsgInfo value);
    int find_angle_value(uint8_t device, uint32_t key);
    void insert(uint32_t key, PersonMsgInfo value);
    int find_person_value(uint32_t key);
    void insert(uint32_t key, AlarmMsgInfo value);
    int find_alarm_value(uint32_t key);
    void insert(uint32_t key, IRMsgInfo value);
    int find_ir_value(uint32_t key);
    void insert(uint32_t key, WirelessMsgInfo value);
    int find_wireless_value(uint32_t key);
    void insert(uint32_t key, HighMsgInfo value);
    int find_high_value(uint32_t key);
    void insert(uint8_t device, uint32_t key, MiniHubMsgInfo value);
    int find_minihub_value(uint8_t device, uint32_t key);



    int setDeviceStateProc(uint8_t driver,uint8_t device,uint8_t subDevice,st_DeviceStatusMsg * deviceMsg);
    int setDeviceStateProc_MiniHub(uint8_t MiniHubIndex,uint32_t deviceIndex,uint8_t subDevice,st_DeviceStatusMsg* deviceMsg);
    int getDeviceStateProc(uint8_t driver,uint8_t device,uint8_t subDevice,st_DeviceStatusMsg* deviceMsg);
    int getDeviceStateProc(uint8_t driver,uint8_t device,uint8_t subDevice,DeviceStateType deviceType, uint16_t* value);
    int setDeviceStateProc(uint8_t driver,uint8_t device,uint8_t subDevice,DeviceStateType deviceType, uint16_t value);
    int getDeviceAppProc_EMVD(uint8_t driver,uint8_t device,uint8_t subDevice,EMVD_APP type,uint32_t *value);
    int setDeviceAppProc_EMVD(uint8_t driver,uint8_t device,uint8_t subDevice,EMVD_APP type,uint16_t point,uint8_t state);
    int setDeviceAppProc_MINIHUB(uint8_t MiniHubIndex, uint32_t DeviceIndex, uint8_t point, uint8_t *data, uint8_t len);
    int setDeviceAppProc_WL(uint8_t driver,uint8_t device,uint8_t subDevice,uint8_t type);
    int setDeviceAppProc_Wireless(uint8_t driver,uint8_t device,uint8_t subDevice,uint8_t type,uint32_t data);
    int getDeviceAppProc_Wireless(uint8_t driver,uint8_t device,uint8_t subDevice,uint8_t type,uint32_t * data);
    int getDeviceAppProc_ANGLE(uint8_t driver, uint8_t device, uint8_t subDevice, ANGLE_APP type, uint32_t* value);
    int setDeviceAppProc_ANGLE(uint8_t driver, uint8_t device, uint8_t subDevice, ANGLE_APP type, uint32_t value);
    int setDeviceAppProc_PERSON(uint8_t driver, uint8_t device, uint8_t subDevice, uint16_t type,uint16_t id, PersonAppMsg* perData);
    int setDeviceAppProc_ALARM(uint8_t driver, uint8_t typedevice, uint8_t subDevice, uint16_t point,uint8_t workMode);
    int getDeviceAppProc_ALARM(uint8_t driver, uint8_t device, uint8_t subDevice, uint16_t point, uint8_t * workMode);
    int setDeviceAppProc_HIGH(uint8_t driver, uint8_t device, uint8_t subDevice, uint32_t value);

    void setShmCallback(uint8_t appid,uint16_t type,uint8_t device,uint8_t childdevice,uint8_t point,uint8_t* data,int len);

    int queryKeyDataProc_emvd(uint32_t *deviceData);
    int queryKeyDataProc_angle(uint32_t *deviceData);
    int queryKeyDataProc_ir(uint32_t *deviceData);
    int queryKeyDataProc_alarm(uint32_t *deviceData);
    int queryKeyDataProc_person(uint32_t *deviceData);
    int queryKeyDataProc_alarmAngle(uint32_t *deviceData);
    int queryKeyDataProc_highAngle(uint32_t *deviceData);
    int queryKeyDataProc_high(uint32_t *deviceData);
    int queryKeyDataProc_MiniHub1(uint32_t *deviceData);
    int queryKeyDataProc_MiniHub2(uint32_t *deviceData);

    int queryKeyDataProc(uint8_t device, uint32_t *deviceData);


    int queryDeviceStateProc_ir(uint8_t timer);
    int queryDeviceStateProc_emvd(uint8_t timer);
    int queryDeviceStateProc_wireless(uint8_t timer);
    int queryDeviceStateProc_angle(uint8_t timer);
    int queryDeviceStateProc_alarmAngle(uint8_t timer);
    int queryDeviceStateProc_highAngle(uint8_t timer);
    int queryDeviceStateProc_high(uint8_t timer);
    int queryDeviceStateProc_alarm(uint8_t timer);
    int queryDeviceStateProc_person(uint8_t timer);
    int queryDeviceStateProc_MiniHub1(uint8_t timer);
    int queryDeviceStateProc_MiniHub2(uint8_t timer);
    int queryDeviceStateProc_MiniHub3(uint8_t timer);

    int SetAllDeviceStateProc_emvd(void);
    int SetAllDeviceStateProc_wireless(void);
    int SetAllDeviceStateProc_ir(void);
    int SetAllDeviceStateProc_angle(void);
    int SetAllDeviceStateProc_high(void);
    int SetAllDeviceStateProc_highAngle(void);
    int SetAllDeviceStateProc_alarmAngle(void);
    int SetAllDeviceStateProc_person(void);
    int SetAllDeviceStateProc_alarm(void);

};


#endif
