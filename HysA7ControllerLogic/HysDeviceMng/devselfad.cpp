#include "devselfad.h"
#include "v4hysapptype.h"
#include "./Sensor/sensor.h"
#include "./HysSystem/hyssystem.h"
#include "hysdevmsg.h"

/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;

/******/
static int DevFrameSelfAdOriginValueReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    if(len < 0x02)
        return -1;
    if((point != 0 )&&(point > SELF_AD_POINT_BOARD_CURRENT))
         return -1;

//    qDebug()<<"***************DevFrameSelfAdOriginValueReportProc********"<<point<<len<<*(u_int16_t *)data;
    HysSystem *pSystem = HysSystem::getInston();
    if(point == 0)
    {
        for(u_int16_t i = 0; i < len;i++,i++)
        {
            if((i/2+1) <= SELF_AD_POINT_SENSOR_4)
                SensorInsertStateApi(driverid,devid,childdevid,i/2+1,*(u_int16_t *)(data+i));
            else if((i/2+1) == SELF_AD_POINT_BOARD_VOLTAGE)
                pSystem->InsertSystemOtherDevStatus(HYS_GZM_OTHERDEV_SELF_INF,1,*(u_int16_t *)data);
            else if((i/2+1) == SELF_AD_POINT_BOARD_CURRENT)
                pSystem->InsertSystemOtherDevStatus(HYS_GZM_OTHERDEV_SELF_INF,2,*(u_int16_t *)data);

            else
                ;
        }
    }
    else
    {
        if(point <= SELF_AD_POINT_SENSOR_4)
            SensorInsertStateApi(driverid,devid,childdevid,point, *(u_int16_t *)data);
        else if(point == SELF_AD_POINT_BOARD_VOLTAGE)
            pSystem->InsertSystemOtherDevStatus(HYS_GZM_OTHERDEV_SELF_INF,1,*(u_int16_t *)data);
        else if(point == SELF_AD_POINT_BOARD_CURRENT)
        {
            pSystem->InsertSystemOtherDevStatus(HYS_GZM_OTHERDEV_SELF_INF,2,*(u_int16_t *)data);
        }
        else
            ;
    }
    return 0;
}

/******/
int DevSelfAdInit(void)
{
    DevFrameProcMap.insert(V4DRIVER_ORIGIN_VALUE_REPORT,DevFrameSelfAdOriginValueReportProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);
    return 0;
}

int DevSelfAdFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

/***参数修改时，运行参数的同步***/
int DevSelfAdMsgRunParamInit(void)
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
    HysMsgTxInitDevInfProc(DRIVER_AD,EXTDEV_TYPE_SELF_AD,0x01,0x01,buf,4);

    return 0;
}
