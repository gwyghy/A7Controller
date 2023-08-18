#include "devblueteeth.h"
#include "v4hysapptype.h"
#include "hysdevmsg.h"

/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;


/***l蓝牙部分的功能待定***/
static int DevFramBlueTeethOriginValueReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    return -1;
}

/******/
int DevBlueTeethInit(void)
{
    DevFrameProcMap.insert(V4DRIVER_ORIGIN_VALUE_REPORT,DevFramBlueTeethOriginValueReportProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);
    return 0;
}

/******/
int DevBlueTeethFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

/***参数修改时，运行参数的同步***/
int DevBlueTeethMsgRunParamInit(void)
{
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t controllerType = pParamMng->GetSystemValueControllerArearType();
    u_int16_t controllerID = pParamMng->GetRunValueGlobalCurrentSupportID();
    u_int16_t Enabled = 0x01;
    u_int8_t buf[20];

    memmove(buf,&controllerType,0x02);
    memmove(buf+2,&controllerID,0x02);
    memmove(buf+4,&Enabled,0x02);

    HysMsgTxInitDevInfProc(DRIVER_UART4,EXTDEV_TYPE_BLUE_TEETH,0x01,0x01,buf,6);

    return 0;
}
