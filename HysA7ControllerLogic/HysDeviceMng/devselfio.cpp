#include "devselfio.h"
#include "v4hysapptype.h"
#include "./HysSystem/hyssystem.h"
#include "hysdevmsg.h"

/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;


/******/
static int DevFrameSelfIoOriginValueReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    qDebug()<<"********DevFrameSelfIoOriginValueReportProc******************"<<point<<*(u_int8_t *)data;
    bool flag;
    if(point > SELF_IO_POINT_LOCK)
        return -1;
    if(point == 0)
        return -1;
    if(len < 0x02)
        return -1;


    if(*(u_int16_t *)data == 0)
    {
        flag = false;
    }
    else if(*(u_int16_t *)data == 1)
    {
        flag = true;
    }
    else
        return -1;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t scID = pParamMng->GetRunValueGlobalCurrentSupportID();

    HysSystem *pSystem = HysSystem::getInston();
    if(point == SELF_IO_POINT_STOP)
        pSystem->SetSysStopStateStopFlag(scID,flag);
    else if(point == SELF_IO_POINT_LOCK)
        pSystem->SetSysStopStateLockFlag(scID,flag);
    else
        ;

    return 0;
}

/******/
int DevSelfIoInit(void)
{
    DevFrameProcMap.insert(V4DRIVER_ORIGIN_VALUE_REPORT,DevFrameSelfIoOriginValueReportProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);
    return 0;
}

/******/
int DevSelfIoFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

/***参数修改时，运行参数的同步***/
int DevSelfIoMsgRunParamInit(void)
{
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t controllerType = pParamMng->GetSystemValueControllerArearType();
    u_int16_t controllerID = pParamMng->GetRunValueGlobalCurrentSupportID();
    globalParam_DevicePeopleType temp;
    pParamMng->GetRunValueGlobalDevicePeopleParam(temp);

    u_int8_t buf[20];
    memmove(buf,&controllerType,0x02);
    memmove(buf+2,&controllerID,0x02);

    /******/
    HysMsgTxInitDevInfProc(DRIVER_IO,EXTDEV_TYPE_SELF_IO,0x01,0x01,buf,4);

    return 0;
}
