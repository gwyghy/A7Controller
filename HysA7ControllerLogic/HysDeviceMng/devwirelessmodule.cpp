#include "devwirelessmodule.h"
#include "./HysSystem/hyssystem.h"
#include "libhysa7parammng.h"
#include "./Action/actionctrl.h"
#include "hysdevmsg.h"
#include "devctrl.h"

/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;
static u_int16_t   WirelessledState = DEV_POINT_CTRL_OFF;//蓝灯

/**处理无线总线的数据***/
static int DevWirelessModuleOriginValueWirelessBus(u_int8_t *data, int len)
{
    return 0;
}

/*****/
static int DevWirelessModuleCtrlDevAppProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    u_int16_t DestScNumber = 0;//被控设备的架号
    SysCtrlInfoType  ctrlInfoTemp;
    WirelessModuleFuncKey_DataType keystatus;
    ActionCtrlType srcdata;
    u_int16_t ActId ;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    u_int16_t SelfAddr = pParam->GetRunValueGlobalCurrentSupportID();
    ActionCtrl *pActCtrl = ActionCtrl::getInston();
    HysSystem *pSystem = HysSystem::getInston();
    u_int8_t paramlength = 0;
    /****/
    u_int8_t ActDirection;//成组动作执行的方向

    qDebug()<<"****************DevWirelessModuleCtrlDevAppProc***len,point,data0,data1*"<<len<<point<<*(u_int8_t *)(data)<<*(u_int8_t *)(data+1);

    u_int16_t CtrlFlag = pSystem->GetRunScCtrlScWirelessCtrlFlag();
    /***查找此时是否已经处于对码成功的状态***/
    if((CtrlFlag == WIRELESS_MODULE_CODE_MATCH_NONE)\
        ||(CtrlFlag == WIRELESS_MODULE_CODE_MATCH_UNMATCED))
        return -1;

    /***无论数据是否正确***/
    if(WirelessledState == DEV_POINT_CTRL_OFF)
        WirelessledState = DEV_POINT_CTRL_ON;
    else
        WirelessledState = DEV_POINT_CTRL_OFF;

    DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                   SELF_IO_POINT_LED_MATCH_WIRELESS,(u_int8_t *)&WirelessledState,0x02);

    /*****/
    if((point != WIRELESS_MODULE_POINT_SINGLE_ACT)\
        &&(point != WIRELESS_MODULE_POINT_ASSEMBLE_ACT)\
        &&(point != WIRELESS_MODULE_POINT_GRP_ACT)\
        &&(point != WIRELESS_MODULE_POINT_ALARM)\
        &&(point != WIRELESS_MODULE_POINT_FUNCTION_KEY))
        return -1;

    if((point != WIRELESS_MODULE_POINT_FUNCTION_KEY)\
        &&(point != WIRELESS_MODULE_POINT_GRP_ACT))
    {
        if(len < (sizeof (ActionCtrlType)+0x04))
            return -1;
        memmove(&DestScNumber,data,0x02);//
        memmove(&ActId,data+2,0x02);
        memmove(&srcdata.CtrlCmd,data+0x04,sizeof (ActionCtrlType));
        paramlength = 0;
    }

    if(point == WIRELESS_MODULE_POINT_GRP_ACT)
    {
        if(len < (sizeof (ActionCtrlType)+0x04+0x06+0x02))
            return -1;
        else
        {
            memmove(&DestScNumber,data,0x02);//
            memmove(&ActId,data+2,0x02);
            memmove(&srcdata.CtrlCmd,data+0x04,sizeof (ActionCtrlType));
            paramlength = 3;
        }
    }

    if(point != WIRELESS_MODULE_POINT_FUNCTION_KEY)
    {
        /***增加执行参数的判定***/
        if(DestScNumber == V4APP_DEV_ID_ALL)
            return -1;
        if(DestScNumber > pParam->GetRunValueGlobalMaxSupportID())
            return -1;
        if(DestScNumber < pParam->GetRunValueGlobalMinSupportID())
            return -1;
    }


    /****非本架数据直接发送走****/
    if((point <= WIRELESS_MODULE_POINT_ALARM)&&(DestScNumber != SelfAddr))
    {
        u_int8_t sendBuf[20];

        memset(sendBuf,0x00,sizeof(sendBuf));
        memmove(sendBuf,&point,0x01);
        memmove(sendBuf+0x01,&ActId,0x01);
        memmove(sendBuf+0x02,&srcdata.CtrlCmd,0x01);
        memmove(sendBuf+0x03,&srcdata.CtrlMode,0x01);
        memmove(sendBuf+0x04,&paramlength,0x01);//长度
        memmove(sendBuf+0x05,&srcdata.CtrlSourceAddr,0x01);
        memmove(sendBuf+0x06,&srcdata.CtrlSourceType,0x01);
        memmove(sendBuf+0x07,&srcdata.UserID,0x02);
        if(point == WIRELESS_MODULE_POINT_GRP_ACT)
        {
            memmove(sendBuf+0x09,data +16,0x01);
            memmove(sendBuf+0x0A,data +18,0x01);
            /***成组执行方向，与架间协议不一致，需进行转换***/
            ActDirection = *(u8 *)(data +20);
            if(ActDirection == MINE_RUN_LITTLE_TO_BIG)
               ActDirection = pParam->GetRunValueGlobalSupportDirect();
            else
               ActDirection = (pParam->GetRunValueGlobalSupportDirect() == 0)?1:0;//0左1右
            memcpy(sendBuf+0x0B, &ActDirection,0x01);//执行方向

            UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(DestScNumber),V4APP_STATUS_CTRL_DEV,sendBuf,12);
            qDebug()<<"*************DevWirelessModuleCtrlDevAppProc******grp***type***id**"<<DestScNumber<<point<<ActId;
            qDebug()<<"*************DevWirelessModuleCtrlDevAppProc******start***end***dir**"<<*(u_int8_t*)(sendBuf+0x09)<<*(u_int8_t*)(sendBuf+0x0A)<<*(u_int8_t*)(sendBuf+0x0B);
        }
        else
        {
            UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(DestScNumber),V4APP_STATUS_CTRL_DEV,sendBuf,9);
        }
        return 0;
    }

    if(point == WIRELESS_MODULE_POINT_FUNCTION_KEY)
    {
        if(len < sizeof (WirelessModuleFuncKey_DataType))
            return -1;
        memmove(&keystatus.KeyNumber,data,sizeof (WirelessModuleFuncKey_DataType));
        if(keystatus.KeyNumber != WIRELESS_MODULE_FUNCTION_KEY_ID_END)
            return -1;
        if(keystatus.KeyStatus > WIRELESS_MODULE_FUNCTION_KEY_STATUS_PREEEED)
            return -1;
    }

    memset(&ctrlInfoTemp,0,sizeof(SysCtrlInfoType));

    switch(point)
    {
        case WIRELESS_MODULE_POINT_GRP_ACT:
            ctrlInfoTemp.ParamLength = *((u8 *)data +14)/2;
            memcpy(ctrlInfoTemp.ParamBuf , data +16,0x01);//起始架号
            memcpy(ctrlInfoTemp.ParamBuf +1,data +18,0x01);//结束架号
            ActDirection = *(u8 *)(data +20);
            if(ActDirection == MINE_RUN_LITTLE_TO_BIG)
               ActDirection = pParam->GetRunValueGlobalSupportDirect();
            else
               ActDirection = (pParam->GetRunValueGlobalSupportDirect() == 0)?1:0;//0左1右
            memcpy(ctrlInfoTemp.ParamBuf+2, &ActDirection,0x01);//执行方向
        case WIRELESS_MODULE_POINT_SINGLE_ACT:
        case WIRELESS_MODULE_POINT_ASSEMBLE_ACT:
        case WIRELESS_MODULE_POINT_ALARM:
            ctrlInfoTemp.actionType = static_cast<u8>(point);
            ctrlInfoTemp.actionID   = static_cast<u8>(ActId);
            ctrlInfoTemp.ctrl.CtrlCmd  = static_cast<u8>(srcdata.CtrlCmd);
            ctrlInfoTemp.ctrl.CtrlMode = static_cast<u8>(srcdata.CtrlMode);
            ctrlInfoTemp.ctrl.CtrlSourceAddr = static_cast<u8>(SelfAddr);
            ctrlInfoTemp.ctrl.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ctrlInfoTemp.ctrl.UserID = 0;
            pActCtrl->InsCtrlBufQueue(&ctrlInfoTemp);
            break;

        case WIRELESS_MODULE_POINT_FUNCTION_KEY:
             pSystem->SetSysStopStateEndFlag(V4APP_DEV_ID_YKQ,keystatus.KeyStatus);
            break;

        default:
            break;

    }

    return 0;
}

/*****/
static int DevWirelessModuleManualActHeatbeatAppProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    if(len < 0x04)
        return -1;
    u_int16_t CtrlFlag = HysSystem::getInston()->GetRunScCtrlScWirelessCtrlFlag();
    /***查找此时是否已经处于对码成功的状态***/
    if((CtrlFlag == WIRELESS_MODULE_CODE_MATCH_NONE)\
        ||(CtrlFlag == WIRELESS_MODULE_CODE_MATCH_UNMATCED))
        return -1;

    /***无论数据是否正确***/
    if(WirelessledState == DEV_POINT_CTRL_OFF)
        WirelessledState = DEV_POINT_CTRL_ON;
    else
        WirelessledState = DEV_POINT_CTRL_OFF;

    DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                   SELF_IO_POINT_LED_MATCH_WIRELESS,(u_int8_t *)&WirelessledState,0x02);

    ActionCtrl *pActCtrl = ActionCtrl::getInston();
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    u_int16_t SelfAddr = pParam->GetRunValueGlobalCurrentSupportID();
    u_int16_t CtrlDevID = 0;
    memmove(&CtrlDevID,data,0x02);

    /***此处有待于考证**/
    if(CtrlDevID == SelfAddr)
    {
        pActCtrl->SetManualActionSupportNum(SelfAddr);

        pActCtrl->ClearManualActionClearBuf();
        for(u_int16_t i = 0; i < len && i < MANUALACTIONINFO_LEN; i++,i++)
        {
            if(*(u_int8_t *)(data+i) == 0)
                break;
            if(pActCtrl->SetManualActionClearBuf(i,*(u_int8_t *)(data+2+i)) == false)//序号0开始
                break;
        }
    }
    else if((CtrlDevID != 0)&&(CtrlDevID >= pParam->GetRunValueGlobalMinSupportID())&&(CtrlDevID <= pParam->GetRunValueGlobalMaxSupportID()))
    {
        UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(CtrlDevID),V4APP_STATUS_MANUAL_ACT_HEARTBEAT,data+2,len-2);
    }
    else
        ;

    return 0;
}

/*****/
static int DevWirelessModuleOriginValueReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    u_int16_t strength;
    SysCtrlInfoType  ctrlInfoTemp;
    HysSystem *pSystem = HysSystem::getInston();
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    ActionCtrl *pActCtrl = ActionCtrl::getInston();

    qDebug()<<"******DevWirelessModuleOriginValueReportProc*len,point,data0,data1*"<<len<<point<<*(u_int8_t *)(data)<<*(u_int8_t *)(data+1);
    if(len < 0x02)
        return -1;

    if(point == WIRELESS_MODULE_POINT_CODE_MATCH)
    {
        u_int16_t NowStatus;
        u_int16_t ledState;
        memmove(&NowStatus,data,0x02);
        if(NowStatus > WIRELESS_MODULE_CTRL_NEIGBOUR_SC)
            return -1;
        if(pParamMng->GetRunValueGlobalDeviceInfredCodeEnabled() != 0)
            pSystem->SetRunScCtrlScWirelessCtrlFlag(NowStatus);
        if((NowStatus == WIRELESS_MODULE_CODE_MATCH_WIRELESS)||(NowStatus == WIRELESS_MODULE_CTRL_SELF_SC)\
                ||(NowStatus == WIRELESS_MODULE_CTRL_NEIGBOUR_SC))
        {
            /*****/
            ledState = DEV_POINT_CTRL_ON;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_LED_MATCH_WIRELESS,(u_int8_t *)&ledState,0x02);

            ledState = SELF_IO_POINT_BUZZER_NEAR_SELECTED;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&ledState,0x02);
            ctrlInfoTemp.actionType = ENUM_ALARM;
            ctrlInfoTemp.actionID   = 1;
            ctrlInfoTemp.ctrl.CtrlCmd  = *(static_cast<u_int8_t *>(data+2));
            ctrlInfoTemp.ctrl.CtrlMode = *(static_cast<u_int8_t *>(data+3));
            ctrlInfoTemp.ctrl.CtrlSourceAddr = pParamMng->GetRunValueGlobalCurrentSupportID();
            ctrlInfoTemp.ctrl.CtrlSourceType = ActionCtrl_SourceType_CONTROLLER;
            ctrlInfoTemp.ctrl.UserID = 0;
            pActCtrl->InsCtrlBufQueue(&ctrlInfoTemp);
        }
        else if(NowStatus == WIRELESS_MODULE_CODE_MATCH_IR)
        {
            /*****/
            ledState = DEV_POINT_CTRL_ON;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_LED_MATCH_IR,(u_int8_t *)&ledState,0x02);
        }
        else if((NowStatus == WIRELESS_MODULE_CODE_MATCH_UNMATCED)||(NowStatus == WIRELESS_MODULE_CODE_MATCH_NONE))
        {
            /*****/
            ledState = DEV_POINT_CTRL_OFF;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_LED_MATCH_IR,(u_int8_t *)&ledState,0x02);
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_LED_MATCH_WIRELESS,(u_int8_t *)&ledState,0x02);
            ledState = SELF_IO_POINT_BUZZER_MODE_NONE;
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_BUZZER,(u_int8_t *)&ledState,0x02);
            WirelessledState = DEV_POINT_CTRL_OFF;
            /***控制灯的状态***/
            DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                           SELF_IO_POINT_LED_MATCH_WIRELESS,(u_int8_t *)&WirelessledState,0x02);
        }
    }
    else if(point == WIRELESS_MODULE_POINT_WIRELESS_BUS)
    {
        DevWirelessModuleOriginValueWirelessBus(data,len);//待定
        /***无论数据是否正确***/
        if(WirelessledState == DEV_POINT_CTRL_OFF)
            WirelessledState = DEV_POINT_CTRL_ON;
        else
            WirelessledState = DEV_POINT_CTRL_OFF;
        DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                       SELF_IO_POINT_LED_MATCH_WIRELESS,(u_int8_t *)&WirelessledState,0x02);

    }
    else if(point == WIRELESS_MODULE_POINT_WIRELESS_SIG_STRENGTH)
    {
        memmove(&strength,data,0x02);
        pSystem->SetRunScSelfWirelessSigStrength(strength);
    }
    else if(point == WIRELESS_MODULE_POINT_BLUE_SIG_STRENGTH)
    {
        memmove(&strength,data,0x02);
        pSystem->SetRunScSelfBuleTeethSigStrength(strength);
    }
    qDebug()<<"******DevWirelessModuleOriginValueReportProc,end*len,point,data0,data1*"<<len<<point<<*(u_int8_t *)(data)<<*(u_int8_t *)(data+1);

    return 0;
}

/******/
int DevWirelessModuleInit(void)
{
    DevFrameProcMap.insert(V4APP_STATUS_CTRL_DEV,DevWirelessModuleCtrlDevAppProc);
    DevFrameProcMap.insert(V4APP_STATUS_MANUAL_ACT_HEARTBEAT,DevWirelessModuleManualActHeatbeatAppProc);

    DevFrameProcMap.insert(V4DRIVER_ORIGIN_VALUE_REPORT,DevWirelessModuleOriginValueReportProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);
    return 0;
}

/******/
int DevWirelessModuleFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

/***通讯状态指示灯相关的处理***/
int DevWirelessModuleGetCommLedStatus(void)
{
    return WirelessledState;
}

/***通讯状态指示灯相关的处理***/
int DevWirelessModuleSetCommLedStatus(int ledstatus)
{
    if(ledstatus > DEV_POINT_CTRL_OFF)
        return -1;
    WirelessledState = ledstatus;
    /*****/
    DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                   SELF_IO_POINT_LED_MATCH_WIRELESS,(u_int8_t *)&WirelessledState,0x02);


    return WirelessledState;
}

/***参数修改时，运行参数的同步***/
int DevWirelessModuleMsgRunParamInit(void)
{
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t controllerType = pParamMng->GetSystemValueControllerArearType();
    u_int16_t controllerID = pParamMng->GetRunValueGlobalCurrentSupportID();
    u_int16_t jiahaodir = pParamMng->GetRunValueGlobalSupportDirect();
    u_int16_t mastertimer = pParamMng->GetRunValueGlobalMasterCtrlTime();

    GlobalParam_InCommonUse common;
    pParamMng->GetRunValueGlobalInCommonUse(common);
    u_int16_t gejiajiashu = common.GapSupportNum;
    u_int16_t Enabled = 0x01;
    u_int8_t buf[20];

    memmove(buf,&controllerType,0x02);
    memmove(buf+2,&controllerID,0x02);
    memmove(buf+4,&jiahaodir,0x02);
    memmove(buf+6,&Enabled,0x02);
    memmove(buf+8,&mastertimer,0x02);
    memmove(buf+10,&gejiajiashu,0x02);

    HysMsgTxInitDevInfProc(DRIVER_UART3,EXTDEV_TYPE_WIRELESS,0x01,0x01,buf,12);

    return 0;
}
