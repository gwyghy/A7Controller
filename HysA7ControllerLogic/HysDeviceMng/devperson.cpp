#include "devperson.h"
#include "v4hysapptype.h"
#include "libhysa7parammng.h"
#include "devctrl.h"
#include "./HysSystem/hyssystem.h"
#include "hysdevmsg.h"

/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;


/*****/
static int DevFramePersonOriginValueReportProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    u_int8_t temp[20];
    PersonLableDevType tempPerson;

//    qDebug()<<"******************DevFramePersonOriginValueReportProc**********"<<driverid<<childdevid<<point<<len<<*(u_int16_t *)data<<*(u_int16_t *)(data+2)<<*(u_int16_t *)(data+4)<<*(u_int16_t *)(data+6);
    if(len < sizeof(PersonLableDevType))
        return -1;

    memmove(&tempPerson.PersonLableId,data,len);

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();

    HysSystem *pSystem = HysSystem::getInston();
    u_int8_t restoreid = 0;
    if((tempPerson.PersonLableId >= UWB_IR_SEND_NUMB_MIN)&&(tempPerson.PersonLableId <= UWB_IR_SEND_NUMB_MAX))
        restoreid = 1;
    else
        restoreid =  pSystem->SetSystemPersonState(tempPerson);

    /***立即上报标签信息，煤机标签或人员呼救***/
    if(((tempPerson.PersonLableId >= UWB_IR_SEND_NUMB_MIN)&&(tempPerson.PersonLableId <= UWB_IR_SEND_NUMB_MAX))\
        ||((tempPerson.PersonLableStatus&PERSON_STATUS_HELP_FLAG) == PERSON_STATUS_HELP_FLAG))
    {
        temp[0] = REPORTENUM_PERSON;
        temp[1] = ENUM_DEVICE;//此处所写数据，在服务器端，会进行特殊处理。特此说明
        temp[2] = restoreid;
        u_int16_t length =sizeof(PersonLableDevType);
        memcpy(&temp[3],&length,2);
        memcpy(&temp[5],data,length);

        UdpAppProcCtrl(V4APP_DEV_TYPE_SS_REPORT,serverID,V4APP_STATUS_REPORT,\
                           temp,5+length);
    }

    return -1;
}

/******/
int DevPersonInit(void)
{
    DevFrameProcMap.insert(V4DRIVER_ORIGIN_VALUE_REPORT,DevFramePersonOriginValueReportProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);
    return 0;
}

/******/
int DevPersonFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

/***参数修改时，运行参数的同步***/
int DevPersonMsgRunParamInit(void)
{
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    u_int16_t controllerType = pParamMng->GetSystemValueControllerArearType();
    u_int16_t controllerID = pParamMng->GetRunValueGlobalCurrentSupportID();
    globalParam_DevicePeopleType temp;
    pParamMng->GetRunValueGlobalDevicePeopleParam(temp);

    u_int8_t buf[20];
    memmove(buf,&controllerType,0x02);
    memmove(buf+2,&controllerID,0x02);
    memmove(buf+4,&temp.CheckRange,0x02);
    memmove(buf+6,&temp.ReportInterval,0x02);
    memmove(buf+8,&temp.SendPower,0x02);

    HysMsgTxInitDevInfProc(DRIVER_CAN1,EXTDEV_TYPE_PERSON,0x01,0x01,buf,10);
    HysMsgTxInitDevInfProc(DRIVER_CAN2,EXTDEV_TYPE_PERSON,0x01,0x01,buf,10);

    return 0;
}
