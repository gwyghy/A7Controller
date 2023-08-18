#include "devselfinfred.h"
#include "v4hysapptype.h"
#include "./HysSystem/hyssystem.h"
#include "hysdevmsg.h"

/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;

/******/
static u_int16_t   ledState = DEV_POINT_CTRL_OFF;//蓝灯

/******/
static int DevFrameSelfInfredOriginValueReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    if(len != sizeof (INFRED_DATA_TYPE))
        return -1;

    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    if(pParamMng->GetRunValueGlobalDeviceInfredRecvEnabled() == 0)
        return -1;
    if(pParamMng->GetRunValueGlobalSysServerEnabled() == 0)
        return -1;

    HysSystem *pSystem = HysSystem::getInston();
    u_int16_t ServerCommErr = 0;
    if(pSystem->GetRunScSelfServerCommErr(ServerCommErr) == false)
        return -1;
    if(ServerCommErr != 0)
        return -1;

    INFRED_DATA_TYPE irdata;
    memmove(&irdata.InfredId,data,sizeof (INFRED_DATA_TYPE));
    if((irdata.InfredId > SELF_INFRED_ID_MAX)||(irdata.InfredId == 0))
        return -1;

    pSystem->SetSystemSelfStateIrSensorRecv(irdata);
    /*****/
    u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();
    u_int8_t temp[100];

    temp[0] = REPORTENUM_SYSTEM;
    temp[1] = ENUM_SYSTEM;//
    temp[2] = 0;
    u_int16_t length =sizeof(HysScSystemInfType);
    memcpy(&temp[3],&length,2);
    pSystem->GetSystemSelfState(*(HysScSystemInfType*)(temp+5));

    UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                       temp,5+length);

    /*****/
    if(ledState == DEV_POINT_CTRL_OFF)
        ledState = DEV_POINT_CTRL_ON;
    else
        ledState = DEV_POINT_CTRL_OFF;

    DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                   SELF_IO_POINT_LED_COMM_WIRELESS,(u_int8_t *)&ledState,0x02);

    /*****/
    memset(&irdata.InfredId,0x00,sizeof (INFRED_DATA_TYPE));
    pSystem->SetSystemSelfStateIrSensorRecv(irdata);
    return 0;
}

/******/
int DevSelfInfredInit(void)
{
    DevFrameProcMap.insert(V4DRIVER_ORIGIN_VALUE_REPORT,DevFrameSelfInfredOriginValueReportProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);
    return 0;
}

/******/
int DevSelfInfredFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

/***通讯状态指示灯相关的处理***/
int DevSelfInfredGetCommLedStatus(void)
{
    return ledState;
}

/***通讯状态指示灯相关的处理***/
int DevSelfInfredSetCommLedStatus(int ledstatus)
{
    if(ledstatus > DEV_POINT_CTRL_OFF)
        return -1;

    ledState = ledstatus;
    /*****/
    DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                   SELF_IO_POINT_LED_COMM_WIRELESS,(u_int8_t *)&ledState,0x02);


    return ledState;
}

/***参数修改时，运行参数的同步***/
int DevSelfInfredMsgRunParamInit(void)
{
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t controllerType = pParamMng->GetSystemValueControllerArearType();
    u_int16_t controllerID = pParamMng->GetRunValueGlobalCurrentSupportID();
     u_int16_t Enabled = 0x01;
    u_int8_t buf[20];

    memmove(buf,&controllerType,0x02);
    memmove(buf+2,&controllerID,0x02);
    memmove(buf+4,&Enabled,0x02);

    HysMsgTxInitDevInfProc(DRIVER_CAN1,EXTDEV_TYPE_IRRX,0x01,0x01,buf,6);
    HysMsgTxInitDevInfProc(DRIVER_CAN2,EXTDEV_TYPE_IRRX,0x01,0x01,buf,6);

    HysMsgTxInitDevInfProc(DRIVER_UART5,EXTDEV_TYPE_SELF_INFRED,0x01,0x01,buf,6);

    return 0;
}
