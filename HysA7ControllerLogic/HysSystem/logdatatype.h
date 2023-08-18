#ifndef LOGDATATYPE_H
#define LOGDATATYPE_H

/****日志存储的类型****/
typedef enum
{
    LOG_TYPE_HANDLE_SINGLE = 0, //单动作操作
    LOG_TYPE_HANDLE_COMB, 		//组合动作操作
    LOG_TYPE_HANDLE_GROUP,		//成组动作操作
    LOG_TYPE_SERVICE,    		//服务
    LOG_TYPE_ALARM,      		//传感器报警
    LOG_TYPE_SYSTEM_ALARM,      //系统报警
}LogTypeEnum;

/**运行日志中的，事件中，系统***/
typedef enum
{
    /***以下部分与共享内存中系统状态中定义一致***/
    SYSTEM_EVENT_STOP = 1,          //急停
    SYSTEM_EVENT_LOCK,              //闭锁
    SYSTEM_EVENT_END,               //停止
    SYSTEM_EVENT_HARDWARE_STOP,     //硬件急停
    SYSTEM_EVENT_HARDWARE_LOCK,     //硬件闭锁

    SYSTEM_EVENT_HARDWARE_END,      //硬件停止
    SYSTEM_EVENT_PERSON_LOCK,       //人员闭锁
    SYSTEM_EVENT_PROGRAM_RESET,     //程序复位
    SYSTEM_EVENT_B12V_ERR,          //B12V异常
    SYSTEM_EVENT_ADDR_CONFIG,       //配址模式

    SYSTEM_EVENT_LEFT_COMM_ERR,     //左邻架通信错误
    SYSTEM_EVENT_RIGHT_COMM_ERR,    //右邻架通信错误
    SYSTEM_EVENT_LEFT_PARAM_DISAGREE,       //左侧参数不一致
    SYSTEM_EVENT_RIGHT_PARAM_DISAGREE,      //右侧参数不一致
    SYSTEM_EVENT_NEIGHBOUR_PRG_DISAGREE,    //邻架版本不一致

    /***以下部分为特殊处理的***/
    SYSTEM_EVENT_NET_REININT,               //网络重新初始化
    SYSTEM_EVENT_HARD_SOFR_DISAGREEE,       //硬件与软件不符

    SYSTEM_EVENT_GZM_COMM_ERR,              //工作面通信异常
    SYSTEM_EVENT_GZM_CODER_ERR,             //系统编号错误
    SYSTEM_EVENT_SS_POSITON_ENABLE_DISAGREE,//SS位置或使能不符

    SYSTEM_EVENT_CODER_DIRECT_DISAGREE,     //架号增向不符
    SYSTEM_EVENT_CODER_AREA_PARAM_ERR,      //架号范围参数错误
    SYSTEM_EVENT_SC_SS_COMM_ERR,            //与服务器通信异常
    SYSTEM_EVENT_MIN_SC_DISAGREE,           //最小架号不一致
    SYSTEM_EVENT_MAX_SC_DISAGREE,           //最大架号不一致
    SYSTEM_EVENT_CODER_DIR_DISAGREE,         //架号增向不一致
    SYSTEM_EVENT_MAX
}SystemEventEnum;

/**运行日志中的，事件中，服务类***/
typedef enum
{
    SERVICE_EVENT_TRANS_GLOBAL_PARAM = 1,//传递全局参数
    SERVICE_EVENT_UPDATE_PROGRAM,       //更新程序
    SERVICE_EVENT_JSON_TEXT,            //配置文件启动
    SERVICE_EVENT_RESTORE_DEFAULT_PARAM,    //恢复出厂默认参数
    SERVICE_EVENT_MAX
}ServiceEventEnum;

/*运行日志中的，状态定义*/
typedef enum
{
    LOG_STATUS_START=0,         //直接启动(启动)
    LOG_STATUS_STOP ,           //停止
    LOG_STATUS_DELAY_START ,    //延时启动
    LOG_STATUS_SUSPEND,         //暂停

    LOG_STATUS_INPLACE,         //到位
    LOG_STATUS_DISABLE,         //禁止
    LOG_STATUS_OVERTIME,        //超时-暂时不加

    LOG_STATUS_SENSOR_NORMAL,   //正常
    LOG_STATUS_SENSOR_ALARM,    //报警

    LOG_STATUS_SUCCESS,         //成功
    LOG_STATUS_FAIL,            //失败

    LOG_STATUS_HAPPEN,          //"发生"
    LOG_STATUS_RESTORE,         //"恢复"

    LOG_STATUS_ENTER,           //"进入"
    LOG_STATUS_EXIT,            //"退出"

    LOG_STATUS_RESET_LOWPAWER,//低功耗复位
    LOG_STATUS_RESET_WWATCHDOG,//窗口看门狗
    LOG_STATUS_RESET_IWATCHDOG,//独立看门狗
    LOG_STATUS_RESET_SOFTWARE , //软件复位
    LOG_STATUS_RESET_PORRSTF,  //掉电复位

    LOG_STATUS_START_OUT,  //外部启动
    LOG_STATUS_START_IN   //内部启动

} LogStatusEnum;

/***系统状态相关的，事件定义**/
static const char *SystemLogEventTab[] =
{
    /***以下部分与共享内存中系统状态中定义一致***/
    "急停",
    "闭锁",
    "停止",
    "急停",//本架操作
    "闭锁",//本架操作

    "停止",//本架操作
    "人员闭锁",
    "程序复位",
    "B12V异常",
    "配址模式",

    "左邻架通信错误",
    "右邻架通信错误",
    "左侧参数不一致",
    "右侧参数不一致",
    "邻架版本不一致",

    /***以下部分为特殊处理的***/
    "网络重新初始化",
    "硬件与软件不符",

    "工作面通信异常",
    "系统编号错误",
    "SS位置或使能不符",

    "架号增向不符",
    "架号范围参数错误",
    "与服务器通信异常",
    "最小架号不一致",
    "最大架号不一致",
    "架号增向不一致",
    ""
};

/***系统状态相关的，事件定义**/
static const char *ServiceLogEventTab[] =
{
    "传全局参数",
    "更新程序",
    "配置文件启动",
    "恢复出厂设置",
    ""
};

/*运行日志中的，状态定义*/
static const char *RunLogStatusTab[] =
{
    "直接启动",
    "停止",
    "延时启动",
    "暂停",

    "到位",
    "禁止",
    "超时",

    "正常",
    "报警",

    "成功",
    "失败",

    "发生",
    "恢复",

    "进入",
    "退出",

    "低功耗复位",
    "窗口看门狗",
    "独立看门狗",
    "软件复位",
    "掉电复位",

    "外部启动",
    "内部启动"
};




#endif // LOGDATATYPE_H
