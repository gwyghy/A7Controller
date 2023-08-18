#include "devkey.h"
#include "v4hysapptype.h"
#include "devctrl.h"
#include "hysdevmsg.h"

/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;


/******/
static int DevFrameKeyOriginValueReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    if(point > SELF_IO_POINT_LOCK)
        return -1;
    if(point == 0)
        return -1;
    if(len < 0x02)
        return -1;

    if(*(u_int16_t *)data != DEV_POINT_CTRL_ON)
        return -1;

    u_int16_t ctrlmode = SELF_IO_POINT_BUZZER_KEY_PRESSED;
    DevCtrlInquire(DRIVER_IO,EXTDEV_TYPE_SELF_IO,1,\
                   SELF_IO_POINT_BUZZER,(u_int8_t *)&ctrlmode,0x02);

    return 0;
}

/******/
int DevKeyInit(void)
{
    DevFrameProcMap.insert(V4DRIVER_ORIGIN_VALUE_REPORT,DevFrameKeyOriginValueReportProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);
    return 0;
}

/******/
int DevKeyFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

/***参数修改时，运行参数的同步***/
int DevKeyMsgRunParamInit(void)
{
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t controllerType = pParamMng->GetSystemValueControllerArearType();
    u_int16_t controllerID = pParamMng->GetRunValueGlobalCurrentSupportID();
    globalParam_DevicePeopleType temp;
    pParamMng->GetRunValueGlobalDevicePeopleParam(temp);

    u_int8_t buf[20];
    memset(buf,0x00,sizeof (buf));
    memmove(buf,&controllerType,0x02);
    memmove(buf+2,&controllerID,0x02);

    /******/
    HysMsgTxInitDevInfProc(DRIVER_UART2,EXTDEV_TYPE_KEYBOARD,0x01,0x01,buf,6);

    return 0;
}
