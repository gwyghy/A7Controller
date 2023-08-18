#include "v4hysapp.h"
#include "hysdevmsg.h"
#include "devglobaltype.h"
#include "HysSystem/hyssystem.h"
#include "devicemngapi.h"
#include "Action/actionctrl.h"
#include "HysAppInteraction/hysappinteraction.h"
#include "libhysa7parammng.h"
#include "HysAppInteraction/hysappmsg.h"

/******************设备控制*************************/
/******/
int FrameGetSingleInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 2)
        return -1;
    u_int8_t cdevtype = data[0];
    u_int8_t cdevid   = data[1];
    switch (cdevtype) {
    case ENUM_SINGLE_ACTION:
    {
        uchar buf[4+sizeof(HysScSingleActType)-2];
        memset(buf,0,4+sizeof(HysScSingleActType)-2);
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(HysScSingleActType)-2;//应答数据长度
        buf[index++] = 0;
        HysScSingleActType hysScSingleActType;
        if(HysSystem::getInston()->GetSingleActState((u_int16_t)cdevid,hysScSingleActType) == false)
            return -1;
        memcpy(buf+index,&hysScSingleActType.Status,sizeof(HysScSingleActType)-2);
        index += sizeof(HysScSingleActType)-2;
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_COMBINATION_ACTION:
    {
        uchar buf[4+sizeof(CombActionStateType)-2];
        memset(buf,0,4+sizeof(CombActionStateType)-2);
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(CombActionStateType)-2;//应答数据长度
        buf[index++] = 0;
        CombActionStateType* combActionStateType = HysSystem::getInston()->GetCombActState(cdevid);
        if(combActionStateType == NULL)
            return -1;
        memcpy(buf+index,&combActionStateType->RunState,sizeof(CombActionStateType)-2);
        index += sizeof(CombActionStateType)-2;//?
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_GROUP_ACTION:
    {
        uchar buf[4+sizeof(groupActionStateType)-2];
        memset(buf,0,4+sizeof(groupActionStateType)-2);
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(groupActionStateType)-2;//应答数据长度
        buf[index++] = 0;
        groupActionStateType* goupActionStateType = HysSystem::getInston()->GetGrpActState(cdevid);
        if(goupActionStateType == NULL)
            return -1;
        memcpy(buf+index,&goupActionStateType->RunState,sizeof(groupActionStateType)-2);
        index += sizeof(groupActionStateType)-2;//?
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_SENSOR:
    {
        uchar buf[4+sizeof(HysScSensorType)-2];
        memset(buf,0,4+sizeof(HysScSensorType)-2);
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(HysScSensorType)-2;//应答数据长度
        buf[index++] = 0;
        HysScSensorType* hysScSensorType = HysSystem::getInston()->GetSensorState(cdevid);
        if(hysScSensorType == NULL)
            return -1;
        memcpy(buf+index,&hysScSensorType->OriginalValue,sizeof(HysScSensorType)-2);
        index += sizeof(HysScSensorType)-2;//?
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_PART:
    {
        uchar buf[4+sizeof(u_int16_t)];
        memset(buf,0,4+sizeof(u_int16_t));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(u_int16_t);//应答数据长度
        buf[index++] = 0;
        u_int16_t partType = 0;
        if(HysSystem::getInston()->GetPartState(cdevid,partType) == false)
            return -1;
        memcpy(buf+index,&partType,sizeof(u_int16_t));
        index += sizeof(u_int16_t);//?
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_DEVICE:
    {
        uchar buf[4+sizeof(HysScExtDevType)-2];
        memset(buf,0,4+sizeof(HysScExtDevType)-2);
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(HysScExtDevType)-2;//应答数据长度
        buf[index++] = 0;
        HysScExtDevType hysScExtDevType;
        if(HysSystem::getInston()->GetExtDevState(cdevid,hysScExtDevType) == false)
            return -1;
        memcpy(buf+index,&hysScExtDevType.Enabled,sizeof(HysScExtDevType)-2);
        index += sizeof(HysScExtDevType)-2;//?
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_SYSTEM:
    {
        uchar buf[4+sizeof(HysScSystemInfType)];
        memset(buf,0,4+sizeof(HysScSystemInfType));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(HysScSystemInfType);//应答数据长度
        buf[index++] = 0;
        HysScSystemInfType hysScSystemInfType;
        if(HysSystem::getInston()->GetSystemSelfState(hysScSystemInfType) == false)
            return -1;
        memcpy(buf+index,&hysScSystemInfType,sizeof(HysScSystemInfType));
        index += sizeof(HysScSystemInfType);//?
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    default:
        break;
    }
    return 0;
}

int FrameGetMultiInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 3)
        return -1;
    u_int8_t cdevtype = data[0];
    u_int8_t cdevid_start   = data[1];
    u_int8_t cdevid_end   = data[2];
    if(cdevid_start > cdevid_end)
    {
        u_int8_t buf = cdevid_start;
        cdevid_start = cdevid_end;
        cdevid_end = buf;
    }
    switch (cdevtype) {
    case ENUM_SINGLE_ACTION:
    {
        if(cdevid_start ==0 || cdevid_end == 0)
        {
            cdevid_start =1;
            cdevid_end = HYS_SINGLEACT_NUMB_MAX;
        }
        uchar buf[4+(sizeof(HysScSingleActType)-2)*(cdevid_end-cdevid_start+1)];
        memset(buf,0,4+(sizeof(HysScSingleActType)-2)*(cdevid_end-cdevid_start+1));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid_start;
        buf[index++] = cdevid_end;
        buf[index++] = sizeof(HysScSingleActType)-2;//应答数据长度
        buf[index++] = 0;
        for (int cdevid = cdevid_start; cdevid <= cdevid_end; ++cdevid)
        {
            HysScSingleActType hysScSingleActType;
            if(HysSystem::getInston()->GetSingleActState((u_int16_t)cdevid,hysScSingleActType) == false)
                return -1;
            memcpy(buf+index,&hysScSingleActType.Status,sizeof(HysScSingleActType)-2);
            index += sizeof(HysScSingleActType)-2;
        }
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_COMBINATION_ACTION:
    {
        if(cdevid_start ==0 || cdevid_end == 0)
        {
            cdevid_start =1;
            cdevid_end = HYS_ASSEMBLELEACT_NUMB_MAX;
        }
        uchar buf[4+(sizeof(CombActionStateType)-2)*(cdevid_end-cdevid_start+1)];
        memset(buf,0,4+(sizeof(CombActionStateType)-2)*(cdevid_end-cdevid_start+1));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid_start;
        buf[index++] = cdevid_end;
        buf[index++] = sizeof(CombActionStateType)-2;//应答数据长度
        buf[index++] = 0;
        for (int cdevid = cdevid_start; cdevid <= cdevid_end; ++cdevid)
        {
            CombActionStateType* combActionStateType = HysSystem::getInston()->GetCombActState(cdevid);
            if(combActionStateType == NULL)
                return -1;
            memcpy(buf+index,&combActionStateType->RunState,sizeof(CombActionStateType)-2);
            index += sizeof(CombActionStateType)-2;//?
        }
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_GROUP_ACTION:
    {
        if(cdevid_start ==0 || cdevid_end == 0)
        {
            cdevid_start =1;
            cdevid_end = HYS_GRPACT_NUMB_MAX;
        }
        uchar buf[4+(sizeof(groupActionStateType)-2)*(cdevid_end-cdevid_start+1)];
        memset(buf,0,4+(sizeof(groupActionStateType)-2)*(cdevid_end-cdevid_start+1));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid_start;
        buf[index++] = cdevid_end;
        buf[index++] = sizeof(groupActionStateType)-2;//应答数据长度
        buf[index++] = 0;
        for (int cdevid = cdevid_start; cdevid <= cdevid_end; ++cdevid)
        {
            groupActionStateType* goupActionStateType = HysSystem::getInston()->GetGrpActState(cdevid);
            if(goupActionStateType == NULL)
                return -1;
            memcpy(buf+index,&goupActionStateType->RunState,sizeof(groupActionStateType)-2);
            index += sizeof(groupActionStateType)-2;//?
        }
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_SENSOR:
    {
        if(cdevid_start ==0 || cdevid_end == 0)
        {
            cdevid_start =1;
            cdevid_end = HYS_SENSOR_NUMB_MAX;
        }
        //        uchar buf[4+(sizeof(HysScSensorType)-2)*(cdevid_end-cdevid_start)];
        //        memset(buf,0,4+(sizeof(HysScSensorType)-2)*(cdevid_end-cdevid_start));
        uchar buf[500];
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid_start;
        buf[index++] = cdevid_end;
        buf[index++] = sizeof(HysScSensorType)-2;//应答数据长度
        buf[index++] = 0;
        for (int cdevid = cdevid_start; cdevid <= cdevid_end; ++cdevid)
        {
            HysScSensorType* hysScSensorType = HysSystem::getInston()->GetSensorState(cdevid);
            if(hysScSensorType == NULL)
                return -1;
            memcpy(buf+index,&hysScSensorType->OriginalValue,sizeof(HysScSensorType)-2);
            index += sizeof(HysScSensorType)-2;//?
        }
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_PART:
    {
        if(cdevid_start ==0 || cdevid_end == 0)
        {
            cdevid_start =1;
            cdevid_end = HYS_PART_NUMB_MAX;
        }
        uchar buf[4+(sizeof(u_int16_t))*(cdevid_end-cdevid_start+1)];
        memset(buf,0,4+(sizeof(u_int16_t))*(cdevid_end-cdevid_start+1));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid_start;
        buf[index++] = cdevid_end;
        buf[index++] = sizeof(u_int16_t);//应答数据长度
        buf[index++] = 0;

        for (int cdevid = cdevid_start; cdevid <= cdevid_end; ++cdevid)
        {
            u_int16_t partType = 0;
            if(HysSystem::getInston()->GetPartState(cdevid,partType) == false)
                return -1;
            memcpy(buf+index,&partType,sizeof(u_int16_t));
            index += sizeof(u_int16_t);//?
        }
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_DEVICE:
    {
        if(cdevid_start ==0 || cdevid_end == 0)
        {
            cdevid_start =1;
            cdevid_end = HYS_EXTDEV_NUB_MAX;
        }
        uchar buf[4+(sizeof(HysScExtDevType)-2)*(cdevid_end-cdevid_start)];
        //        memset(buf,0,4+(sizeof(HysScExtDevType)-2)*(cdevid_end-cdevid_start));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid_start;
        buf[index++] = cdevid_end;
        buf[index++] = sizeof(HysScExtDevType)-2;//应答数据长度
        buf[index++] = 0;
        for (int cdevid = cdevid_start; cdevid <= cdevid_end; ++cdevid)
        {
            HysScExtDevType hysScExtDevType;
            if(HysSystem::getInston()->GetExtDevState(cdevid,hysScExtDevType) == false)
                return -1;
            memcpy(buf+index,&hysScExtDevType.Enabled,sizeof(HysScExtDevType)-2);
            index += sizeof(HysScExtDevType)-2;//?
        }
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_SYSTEM:
    {
        uchar buf[4+sizeof(HysScSystemInfType)];
        memset(buf,0,4+sizeof(HysScSystemInfType));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = 0;
        buf[index++] = 0;
        buf[index++] = sizeof(HysScSystemInfType);//应答数据长度
        buf[index++] = 0;
        HysScSystemInfType hysScSystemInfType;
        if(HysSystem::getInston()->GetSystemSelfState(hysScSystemInfType) == false)
            return -1;
        memcpy(buf+index,&hysScSystemInfType,sizeof(HysScSystemInfType));
        index += sizeof(HysScSystemInfType);//?
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    case 8://人员
    {
        if(cdevid_start ==0 || cdevid_end == 0)
        {
            cdevid_start =1;
            cdevid_end = HYS_PERSON_NUB_MAX;
        }
        uchar buf[4+(sizeof(PersonLableType))*(cdevid_end-cdevid_start+1)];
        memset(buf,0,4+(sizeof(PersonLableType))*(cdevid_end-cdevid_start+1));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid_start;
        buf[index++] = cdevid_end;
        buf[index++] = sizeof(PersonLableType);//应答数据长度
        buf[index++] = 0;

        for (int cdevid = cdevid_start; cdevid <= cdevid_end; ++cdevid)
        {
            PersonLableType personLableType;
            if(HysSystem::getInston()->GetSystemPersonState(cdevid,personLableType) == false)
                return -1;
            memcpy(buf+index,&personLableType,sizeof(PersonLableType));
            index += sizeof(PersonLableType);//?
        }
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_STATUS_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    default:
        qDebug()<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<cdevtype;
        break;
    }
    return 0;
}

int FrameStatusReportProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 3)
        return -1;
    u8 rpttype = data[0];
    u8 reddevtype = data[1];
    u8 idstatus = data[2];
    //     u16 length = (data[4]<<8) | data[3];
    //     if(len != length + 5)
    //         return -1;
    switch (rpttype)
    {
    case REPORTENUM_ACTION:
    {

    }
        break;
    case REPORTENUM_EMERGENCY:
    {
        qDebug()<<"****V4app.cpp*****FrameStatusReportProc***stop***"<<numb<<idstatus;
        HysSystem::getInston()->SetSysStopStateStopFlag(numb,idstatus);
    }
        break;
    case REPORTENUM_BISUO:
    {
        qDebug()<<"****V4app.cpp*****FrameStatusReportProc***lock***"<<numb<<idstatus;
        HysSystem::getInston()->SetSysStopStateLockFlag(numb,idstatus);
    }
        break;
    case REPORTENUM_STOP:
    {
        qDebug()<<"****V4app.cpp*****FrameStatusReportProc***end***"<<numb<<idstatus;
        HysSystem::getInston()->SetSysStopStateEndFlag(numb,idstatus);
    }
        break;
    case REPORTENUM_FAIL:
    {

    }
        break;
    case REPORTENUM_SENSOR:
    {

    }
        break;
    case REPORTENUM_DEVICE:
    {

    }
        break;
    case REPORTENUM_PERSON:
    {

    }
        break;
    case REPORTENUM_SYSTEM:
    {

    }
        break;
    default:
        break;
    }

    return 0;
}

/******/
int FrameCtrlDevProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 5)
        return -1;
    u_int8_t cdevtype       = data[0];
    u_int8_t cdevid         = data[1];
    u_int8_t ctlcmd         = data[2];
    u_int8_t ctltype        = data[3];
    u_int8_t ctlparamlen    = data[4];
    if(cdevtype > ENUMTYPE_MAX)
        return -1;
    SysCtrlInfoType sysCtrlInfoType;
    sysCtrlInfoType.actionType = cdevtype;
    sysCtrlInfoType.actionID = cdevid;
    sysCtrlInfoType.ctrl.CtrlCmd = ctlcmd;
    sysCtrlInfoType.ctrl.CtrlMode = ctltype;

    /********************************************************/
    if(len > 6)
        sysCtrlInfoType.ctrl.CtrlSourceAddr = data[5];
    else
        sysCtrlInfoType.ctrl.CtrlSourceAddr = 0;
    if(len > 7)
         sysCtrlInfoType.ctrl.CtrlSourceType = data[6];
    else
         sysCtrlInfoType.ctrl.CtrlSourceType = 0;
    if(len > 9)
         sysCtrlInfoType.ctrl.UserID = (data[7]|(data[8]<<8));
    else
         sysCtrlInfoType.ctrl.UserID = 0;
    sysCtrlInfoType.ParamLength = ctlparamlen;
    if(ctlparamlen <= CTRL_PARAM_LEN_MAX)
    {
        sysCtrlInfoType.ParamLength = ctlparamlen;
        memcpy(sysCtrlInfoType.ParamBuf,data+9, ctlparamlen);
    }
    else
        sysCtrlInfoType.ParamLength = 0;
    /********************************************************/
    ActionCtrl::getInston()->InsCtrlBufQueue(&sysCtrlInfoType);
    uchar buf[3];
    memset(buf,0,3);
    int index = 0;
    buf[index++] = cdevtype;
    buf[index++] = cdevid;
    buf[index++] = 1;
    DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                         V4APP_STATUS_CTRL_DEV_ACK,buf,index);
    return 0;
}
int FrameManulHeartBeatProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 1)
        return -1;
    len = len>MANUALACTIONINFO_LEN?MANUALACTIONINFO_LEN:len;
    for (int var = 1; var <= len; ++var)
    {
        ActionCtrl::getInston()->SetManualActionClearBuf(var,data[var-1]);
    }
    return 0;
}

/******/
int FrameDownloadSysStatusProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len <= 4 && len-4 > sizeof(V4AppSsSystemInfDownType))
        return -1;
    if(data[0] != ENUM_SYSTEM || data[1] != 0 || data[3] != 0)
        return -1;
    V4AppSsSystemInfDownType v4AppSsSystemInfDownType;
    HysSystem::getInston()->GetSystemSsInfDownState(v4AppSsSystemInfDownType);
    if(memcmp(&v4AppSsSystemInfDownType,data+4,len-4)==0)
        return 0;
    memcpy(&v4AppSsSystemInfDownType,data+4,len-4);
    HysSystem::getInston()->SetSystemSsInfDownState(v4AppSsSystemInfDownType);
    return 0;
}
int FrameFollowOnResponseProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 1)
        return -1;
    uchar followcnt = data[0];
    if(followcnt > 20 ||  ((2*followcnt) != (len-1)))
        return -1;
    HysAppInteraction::getInston()->InsertSendMsgProc(MSG_APP_FOLLOWON_RESPONSE,devtype,numb,data,len);
    return 0;
}

/******/
//int FrameDownloadAutoBackFlushOnOffProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
//{

//}

int FrameDownloadAutoBackFlushStatusProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len<4)
        return -1;
    u16 autoBackFlushOnOffFlag = *((u16*)data);
    u16 autoBackFlushNumb = *((u16*)(data+2));
    HysRunBackflushData hysRunBackflushData;
    hysRunBackflushData.BackFlushEnabled = autoBackFlushOnOffFlag;
    hysRunBackflushData.BackFlushScId = autoBackFlushNumb;
    HysSystem::getInston()->SetRunBackFlushState(hysRunBackflushData);
    return 0;
}

int FrameTranDevStatusProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 4 || (*((u16*)(data+2))) != len-4)
        return -1;
    u16 cdevtype = *((u16*)data);
    cdevtype = cdevtype << 4;
    u16 datalen = *((u16*)(data+2));
    if(datalen >HYS_GZM_OTHER_DEV_POINT_MAX)
        return -1;
    HysGzmOtherDevStatus hysGzmOtherDevStatus;
    memset(hysGzmOtherDevStatus.PointValue,0,HYS_GZM_OTHER_DEV_POINT_MAX*sizeof(uint16_t));
    hysGzmOtherDevStatus.StartPointId = cdevtype;
    memcpy(hysGzmOtherDevStatus.PointValue,data+4,datalen);
    HysSystem::getInston()->InsertSystemOtherDevStatus(hysGzmOtherDevStatus);
    return 0;
}

int FrameTerminalInfReportProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 4 || (*((u16*)(data+2))) != len-4)
        return -1;
    u16 cdevtype = *((u16*)data);
    if(cdevtype != 1)//1:架间协议为光电终端设备，在控制器中需要转换
        return -1;
    cdevtype = HYS_GZM_OTHERDEV_TERMINAL_INF;
    u16 datalen = *((u16*)(data+2));
    if(datalen >HYS_GZM_OTHER_DEV_POINT_MAX)
        return -1;
    HysGzmOtherDevStatus hysGzmOtherDevStatus;
    memset(hysGzmOtherDevStatus.PointValue,0,HYS_GZM_OTHER_DEV_POINT_MAX*sizeof(uint16_t));
    hysGzmOtherDevStatus.StartPointId = cdevtype;
    memcpy(hysGzmOtherDevStatus.PointValue,data+4,datalen);
    HysSystem::getInston()->InsertSystemOtherDevStatus(hysGzmOtherDevStatus);
    return 0;
}

/***************参数修改****************************/
int FrameParamGetSingleResponseProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 2)
        return -1;
    u_int8_t cdevtype = data[0];
    u_int8_t cdevid   = data[1];
    switch (cdevtype)
    {
    case ENUM_SINGLE_ACTION:
    {
        u8 buf[4+sizeof(SingleActionParamType)];
        memset(buf,0,4+sizeof(SingleActionParamType));
        SingleActionParamType* singleActionParamType = LibHysA7ParamMng::getInston()->GetRunValueSingleActParam(cdevid);
        if(singleActionParamType == NULL)
            return -1;
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(SingleActionParamType);
        buf[index++] = 0;
        memcpy(buf+index,singleActionParamType,sizeof(SingleActionParamType));
        index += sizeof(SingleActionParamType);
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_PARAM_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_COMBINATION_ACTION:
    {
        u8 buf[4+sizeof(CombActionParamType)];
        memset(buf,0,4+sizeof(CombActionParamType));
        CombActionParamType* combActionParamType = LibHysA7ParamMng::getInston()->GetRunValueCombActParam(cdevid);
        if(combActionParamType == NULL )
            return -1;
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(CombActionParamType);
        buf[index++] = 0;
        memcpy(buf+index,combActionParamType,sizeof(CombActionParamType));
        index += sizeof(CombActionParamType);
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_PARAM_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_GROUP_ACTION:
    {
        u8 buf[4+sizeof(groupActionParamType)];
        memset(buf,0,4+sizeof(groupActionParamType));
        groupActionParamType* goupActionParamType = LibHysA7ParamMng::getInston()->GetRunValueGrpActParam(cdevid);
        if(goupActionParamType == NULL )
            return -1;
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(groupActionParamType);
        buf[index++] = 0;
        memcpy(buf+index,goupActionParamType,sizeof(groupActionParamType));
        index += sizeof(groupActionParamType);
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_PARAM_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_PART:
    {
        PartParamType* partParamType = LibHysA7ParamMng::getInston()->GetRunValuePartParam(cdevid);
        if(partParamType == NULL )
            return -1;
        u8 buf[4+sizeof(PartParamType)];
        memset(buf,0,4+sizeof(PartParamType));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = cdevid;
        buf[index++] = sizeof(PartParamType);
        buf[index++] = 0;
        memcpy(buf+index,partParamType,sizeof(PartParamType));
        index += sizeof(PartParamType);
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_PARAM_GET_SINGLE_RESPONSE,buf,index);
    }
        break;
    case ENUM_SYSTEM:
    {
        SysParamType sysParamType = LibHysA7ParamMng::getInston()->GetRunParamValue();
        u8 buf[4+sizeof(globalParamType)];
        memset(buf,0,4+sizeof(globalParamType));
        int index = 0;
        buf[index++] = cdevtype;
        buf[index++] = 0;
        buf[index++] = 0;
        buf[index++] = sizeof(globalParamType);
        buf[index++] = 0;
        memcpy(buf+index,&sysParamType.globalParam,sizeof(globalParamType));
        index += sizeof(globalParamType);
        DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                             V4APP_PARAM_GET_MULTIPLE_RESPONSE,buf,index);
    }
        break;
    default:
        break;
        return 0;
    }
    return 0;
}

int FrameParamGetMultiResponseProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    {
        if(len < 2)
            return -1;
        u_int8_t cdevtype = data[0];
        u_int8_t cdevid_start   = data[1];
        u_int8_t cdevid_end   = data[2];
        if(cdevid_end < cdevid_start)
            return -1;
        switch (cdevtype)
        {
        case ENUM_SINGLE_ACTION:
        {
            u8 buf[5+sizeof(SingleActionParamType)*(cdevid_end-cdevid_start+1)];
            memset(buf,0,5+sizeof(SingleActionParamType)*(cdevid_end-cdevid_start+1));
            int index = 0;
            buf[index++] = cdevtype;
            buf[index++] = cdevid_start;
            buf[index++] = cdevid_end;
            buf[index++] = sizeof(SingleActionParamType);
            buf[index++] = 0;
            for (int var = cdevid_start; var <= cdevid_end; ++var)
            {
                SingleActionParamType* singleActionParamType = LibHysA7ParamMng::getInston()->GetRunValueSingleActParam(var);
                if(singleActionParamType == NULL)
                    return -1;
                memcpy(buf+index,singleActionParamType,sizeof(SingleActionParamType));
                index += sizeof(SingleActionParamType);
            }
            DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                                 V4APP_PARAM_GET_SINGLE_RESPONSE,buf,index);
        }
            break;
        case ENUM_COMBINATION_ACTION:
        {
            u8 buf[5+sizeof(CombActionParamType)*(cdevid_end-cdevid_start+1)];
            memset(buf,0,5+sizeof(CombActionParamType)*(cdevid_end-cdevid_start+1));
            int index = 0;
            buf[index++] = cdevtype;
            buf[index++] = cdevid_start;
            buf[index++] = cdevid_end;
            buf[index++] = sizeof(CombActionParamType);
            buf[index++] = 0;
            for (int var = cdevid_start; var <= cdevid_end; ++var)
            {
                CombActionParamType* combActionParamType = LibHysA7ParamMng::getInston()->GetRunValueCombActParam(var);
                if(combActionParamType == NULL)
                    return -1;
                memcpy(buf+index,combActionParamType,sizeof(CombActionParamType));
                index += sizeof(CombActionParamType);
            }
            DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                                 V4APP_PARAM_GET_SINGLE_RESPONSE,buf,index);
        }
            break;
        case ENUM_GROUP_ACTION:
        {
            u8 buf[5+sizeof(groupActionParamType)*(cdevid_end-cdevid_start+1)];
            memset(buf,0,5+sizeof(groupActionParamType)*(cdevid_end-cdevid_start+1));
            int index = 0;
            buf[index++] = cdevtype;
            buf[index++] = cdevid_start;
            buf[index++] = cdevid_end;
            buf[index++] = sizeof(groupActionParamType);
            buf[index++] = 0;
            for (int var = cdevid_start; var <= cdevid_end; ++var)
            {
                groupActionParamType* goupActionParamType = LibHysA7ParamMng::getInston()->GetRunValueGrpActParam(var);
                if(goupActionParamType == NULL)
                    return -1;
                memcpy(buf+index,goupActionParamType,sizeof(groupActionParamType));
                index += sizeof(groupActionParamType);
            }
            DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                                 V4APP_PARAM_GET_SINGLE_RESPONSE,buf,index);
        }
            break;
        case ENUM_PART:
        {
            u8 buf[5+sizeof(PartParamType)*(cdevid_end-cdevid_start+1)];
            memset(buf,0,5+sizeof(PartParamType)*(cdevid_end-cdevid_start+1));
            int index = 0;
            buf[index++] = cdevtype;
            buf[index++] = cdevid_start;
            buf[index++] = cdevid_end;
            buf[index++] = sizeof(PartParamType);
            buf[index++] = 0;
            for (int var = cdevid_start; var <= cdevid_end; ++var)
            {
                PartParamType* partParamType = LibHysA7ParamMng::getInston()->GetRunValuePartParam(var);
                if(partParamType == NULL)
                    return -1;
                memcpy(buf+index,partParamType,sizeof(PartParamType));
                index += sizeof(PartParamType);
            }
            DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                                 V4APP_PARAM_GET_SINGLE_RESPONSE,buf,index);
        }
            break;
        case ENUM_SYSTEM:
        {
            SysParamType sysParamType = LibHysA7ParamMng::getInston()->GetRunParamValue();
            u8 buf[4+sizeof(globalParamType)];
            memset(buf,0,4+sizeof(globalParamType));
            int index = 0;
            buf[index++] = cdevtype;
            buf[index++] = 0;
            buf[index++] = 0;
            buf[index++] = sizeof(globalParamType);
            buf[index++] = 0;
            memcpy(buf+index,&sysParamType.globalParam,sizeof(globalParamType));
            index += sizeof(globalParamType);
            DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                                 V4APP_PARAM_GET_MULTIPLE_RESPONSE,buf,index);
        }
            break;
        default:
            break;
            return 0;
        }
    }
    return 0;
}

int FrameParamSetSingleInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 4 || (data[2]*2 > len-4))
        return -1;
    u_int8_t cdevtype    = data[0];
    u_int8_t cdevid      = data[1];
    u_int8_t paramlen    = data[2];
    u_int8_t paramoffset = data[3];

    return 0;
}

int FrameParamSetMultiInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    return 0;
}

int FrameParamCheckInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    //V4APP_PARAM_CHECK_RESPONSE

    return 0;
}

int FrameParamNoticeProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(((len%18) != 0)  && (len <= 0))
        return -1;
    long long int menuid = 0;
    for (int var = 0; var < len/18; ++var)
    {
        memmove(&menuid,data + 18 * var,0x08);
        menuid >>= 8;
        menuid |= SC_PARAM_MENU_FIRST_DEFAULT_SET;
        u16 paramtype = *((u16*)(data+8 + 18 * var));//1:全面修改 0：本地修改
        u16 paramlen  = *((u16*)(data+10 + 18 * var));
        u16 paramoffset  = *((u16*)(data+12 + 18 * var));
        paramoffset ++;
        u16 paramval  = *((u16*)(data+14 + 18 * var));
        u16 paraminputtype  = *((u16*)(data+16  + 18 * var));
        LibHysA7ParamMng::getInston()->InsertRunParamChangeProc(menuid,paramoffset,paramval,paramtype,paraminputtype);
    }
    return 0;
}


int FrameParamServerOperateRequestProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len != 2)
        return -1;
#ifdef PLATFORM_ARM
    u16 buf = *(u16*)data;
    if(buf == 1)//復位控制器
        system("reboot");
    else if(buf == 2)//出場設置
        LibHysA7ParamMng::getInston()->SystemConfigValueFileRestoreDefault();
#endif
    return 0;
}

int FrameParamSSDisplayResponseProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 18)
        return -1;
    HysAppInteraction::getInston()->InsertSendMsgProc(MSG_DISPLAY_DEV_PARAM_RESPONSE,devtype,numb,data,len);
    return 0;
}

int FrameParamReportParamChangedProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    qDebug()<<"*********net*****FrameParamReportParamChangedProc,start";
    if(len < 16 )
        return -1;
    u8 buf[len];
    memset(buf,0,len);
    long long int menuid = 0;
    memmove(&menuid,data,0x08);
    menuid >>= 8;
    menuid |= SC_PARAM_MENU_FIRST_DEFAULT_SET;
    memcpy(buf,&menuid,8);
    memcpy(buf+8,data+8,len-8);
    u16 paramtype = *((u16*)(data+8));//1:全面修改 0：本地修改
    u16 paramlen  = *((u16*)(data+10));
    u16 paramoffset  = *((u16*)(data+12));
    paramoffset ++;
    u16 paramval  = *((u16*)(data+14));
    u16 paraminputtype  = *((u16*)(data+len-2));
    int ret = LibHysA7ParamMng::getInston()->InsertRunParamChangeProc(menuid,paramoffset,paramval,paramtype,paraminputtype);
    qDebug("****net**FrameParamReportParamChangedProc,result: %llx,  %d, line = %d, ret = %d",menuid, paramtype,paramoffset,ret);
    return 0;
}

int FrameParamReportParamChangedAckProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 10 )
        return -1;
    HysAppInteraction::getInston()->InsertSendMsgProc(MSG_REPORTSS_PARAM_CHANGGED_RESPONSE,devtype,numb,data,len);
    return 0;
}

int FrameParamSetLascDataProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 17)
        return -1;
    long long int menuid = 0;
    memmove(&menuid,data,0x08);
    menuid >>= 8;
    menuid |= SC_PARAM_MENU_FIRST_DEFAULT_SET;
    u16 paramoffset  = *((u16*)(data+8));
    paramoffset ++;
    u16 sc_start  = *((u16*)(data+10));
    u16 sc_end  = *((u16*)(data+12));
    u16 funccode  = *((u16*)(data+14));
    u8 lascdata  = *(data+16);
    HysRunLascData hysRunLascData;
    HysSystem::getInston()->GetRunLascState(hysRunLascData);
    if(funccode == 1 || funccode == 2)
    {
        if(hysRunLascData.LascEnabled != funccode || hysRunLascData.LascData != lascdata)
        {
            hysRunLascData.LascEnabled = funccode;
            hysRunLascData.LascData = lascdata;
            HysSystem::getInston()->SetRunLascState(hysRunLascData);
        }
    }
    else if(funccode == 3)
        if(hysRunLascData.LascEnabled != funccode)
        {
            hysRunLascData.LascEnabled = funccode;
            hysRunLascData.LascData = lascdata;
            HysSystem::getInston()->SetRunLascState(hysRunLascData);
        }
    return 0;
}

int FrameParamSystemParamInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(len < 2)
        return -1;
    int offset = 12;
    u16 paramtotalcnt = *((u16*)data);
    if(paramtotalcnt * offset + 2 != len)
        return -1;
    u16 paramtype = 1;//1:全面修改 0：本地修改
    u16 paraminputtype  = 1;//???????????????????????????????????????
    for (int var = 0; var < paramtotalcnt; ++var)
    {
        long long int menuid = 0;
        memmove(&menuid,data+var * offset + 2,0x08);
        menuid >>= 8;
        menuid |= SC_PARAM_MENU_FIRST_DEFAULT_SET;
        u16 paramoffset  = *((u16*)(data+var * offset + 8));
        paramoffset ++;
        u16 paramval  = *((u16*)(data+var * offset + 10));
        LibHysA7ParamMng::getInston()->InsertSystemParamChangeProc(menuid,paramoffset,paramval,paramtype,paraminputtype);
    }
    return 0;
}

int FrameParamReportSystemParamChangedProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    if(((len%18) != 0)  && (len <= 0))
        return -1;
    long long int menuid = 0;
    for (int var = 0; var < len/18; ++var)
    {
        memmove(&menuid,data + 18 * var,0x08);
        u16 paramtype = *((u16*)(data+8 + 18 * var));//1:全面修改 0：本地修改
        u16 paramlen  = *((u16*)(data+10 + 18 * var));
        u16 paramoffset  = *((u16*)(data+12 + 18 * var));//行数。控制器为从0开始
        paramoffset ++;
        u16 paramval  = *((u16*)(data+14 + 18 * var));
        u16 paraminputtype  = *((u16*)(data+16  + 18 * var));

        /***判断是否为设置架号的参数修改,暂时不能全面设置架号***/
        if((menuid == SYSTEM_PARAM_MENU_IP_ADDR_SET )\
            &&(paramoffset == SYSTEM_PARAM_IP_ADDR4_LINE_SET))
        {
            continue;
        }
        LibHysA7ParamMng::getInston()->InsertSystemParamChangeProc(menuid,paramoffset,paramval,paramtype,paraminputtype);
    }
    return 0;
}

/***************网络维护****************************/
int FrameForceEnterSetIpStatusProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    // HysAppInteraction::getInston()->InsertSendMsgProc(MSG_DISPLAY_DEV_PARAM_RESPONSE,devtype,numb,buf,len);
    //???????????????????????缺少给界面发消息   强制进入配置模式
    DeviceMngApi::getInston()->writeshmp(UDP_PROGRAM_ID,childdevtype,devtype,numb,\
                                         V4APP_FORCE_ENTER_SET_IP_STATUS_ACK,data,len);

    return 0;
}

int FrameSetDevIpProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    //设置ip
    return 0;
}

int FrameNetHeartBeatProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    return 0;
}

int FrameNeighbourCommCheckProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    return 0;
}

int FrameNetRsyncFileInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len)
{
    u_int16_t SelfScid = 0;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len != 600)
        return -1;

    if(DevGeneralIsUpdateFilePathAvalid((char *)data,300,(char *)(data+300),300) == false)
        return -1;
    else
        HysMsgTxRSyncFileProc(DRIVER_NET,static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(SelfScid),0,data,len);
    return 0;
}

/**********************参数修改时，运行参数的同步***********/
//Saveflag是否需要保存
int BoradConfigIpMaskGateWay(SystemParamIpType IpParam, bool Saveflag)//用于设置板子的IP，掩码，网关，群发地址等
{
    /******/
    char ipBuf[100] = {0};
    sprintf(ipBuf, "ifconfig eth0 %d.%d.%d.%d netmask %d.%d.%d.%d",\
            IpParam.IpAddr1, IpParam.IpAddr2, IpParam.IpAddr3, IpParam.IpAddr4,\
            IpParam.IpMask1, IpParam.IpMask2, IpParam.IpMask3, IpParam.IpMask4);
    qDebug("ip %s", ipBuf);
    system(ipBuf);

    /*****/
    memset(ipBuf,0x00,sizeof (ipBuf));
    u_int8_t broadcast[4];
    if(IpParam.IpMask1 == 0)
        broadcast[0] = 255;
    else
        broadcast[0] = IpParam.IpAddr1;
    if(IpParam.IpMask2 == 0)
        broadcast[1] = 255;
    else
        broadcast[1] = IpParam.IpAddr2;
    if(IpParam.IpMask3 == 0)
        broadcast[2] = 255;
    else
        broadcast[2] = IpParam.IpAddr3;
    if(IpParam.IpMask4 == 0)
        broadcast[3] = 255;
    else
        broadcast[3] = IpParam.IpAddr4;

    sprintf(ipBuf, "ifconfig eth0 broadcast %d.%d.%d.%d",\
            broadcast[0], broadcast[1], broadcast[2], broadcast[3]);
    qDebug("broadcast %s", ipBuf);
    system(ipBuf);

    if(Saveflag == true)
    {
        sprintf(ipBuf, "eeprom net set ip %d.%d.%d.%d", IpParam.IpAddr1, IpParam.IpAddr2, IpParam.IpAddr3, IpParam.IpAddr4);
        qDebug("ip %s", ipBuf);
        #ifdef PLATFORM_ARM
            system(ipBuf);
        #endif

        sprintf(ipBuf, " eeprom net set netmask %d.%d.%d.%d", IpParam.IpMask1, IpParam.IpMask2, IpParam.IpMask3, IpParam.IpMask4);
        qDebug("netmask %s", ipBuf);
        #ifdef PLATFORM_ARM
            system(ipBuf);
        #endif

        //set gateway
        sprintf(ipBuf, " eeprom net set gateway %d.%d.%d.%d", IpParam.IpGateWay1, IpParam.IpGateWay2, IpParam.IpGateWay3, IpParam.IpGateWay4);
        qDebug("gateway %s", ipBuf);
        #ifdef PLATFORM_ARM
            system(ipBuf);
        #endif
    }

    return 0;
}

int DevNetMsgRunParamInit(void)
{
    SystemParamIpType IpParam;
    SystemParamNeibDevInfType NeighbourParam;
    u_int16_t ScAddDir;
    u_int16_t Data1 = 0;
    u_int16_t Data2 = 0;
    u_int16_t Data3 = 0;
    u_int16_t Data4 = 0;
    u_int8_t buf[200] = {0};
    u_int16_t length = 0;

    memset(&IpParam.AppArea,0x00,sizeof (SystemParamIpType));
    memset(&NeighbourParam.NeighbDevInf[0].Addr1,0x00,sizeof (SystemParamIpType));
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    pParamMng->GetSystemValueSystemIp(IpParam);
    pParamMng->GetSystemValueNeibDevInfType(NeighbourParam);
    ScAddDir = pParamMng->GetRunValueGlobalSupportDirect();

    /****2023.07.11,增加开机初始化IP及掩码网关等****/
    #ifdef PLATFORM_ARM
        BoradConfigIpMaskGateWay(IpParam,false);
    #endif
    /*****/
    memset(buf,0x00,sizeof (buf));
    memmove(buf,&IpParam.AppArea,sizeof (SystemParamIpType));
    length += sizeof (SystemParamIpType);
    memmove(buf+length,&NeighbourParam.NeighbDevInf[0].Addr1,sizeof (SystemParamNeibDevInfType));
    length += sizeof (SystemParamNeibDevInfType);
    memmove(buf+length,&ScAddDir,sizeof (u_int16_t));
    length += sizeof (u_int16_t);

    pParamMng->GetRunValueGlobalSysServerMasterAddr(Data1,Data2,Data3,Data4);
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data2,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data3,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data4,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    pParamMng->GetRunValueGlobalSysServerSlaveAddr(Data1,Data2,Data3,Data4);
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data2,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data3,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data4,sizeof (u_int16_t));
    length += sizeof (u_int16_t);

    Data1 = 1;//架间检测使能
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    Data1 = 5000;//架间检测时间
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    Data1 = pParamMng->GetRunValueAllGlobalParamCrc();//CRC
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);

    /***23.07.10，新增加版本信息****/
    buf[length] = A7CONTROLLER_LOGIC_VERSION_1;
    length++;
    buf[length] = A7CONTROLLER_LOGIC_VERSION_2;
    length++;
    buf[length] = A7CONTROLLER_LOGIC_VERSION_3;
    length++;
    buf[length] = A7CONTROLLER_LOGIC_VERSION_4;
    length++;

    /******/
    ushort scidmuber = 0;
    scidmuber = pParamMng->GetRunValueGlobalCurrentSupportID();
    HysMsgTxInitDevInfProc(DRIVER_NET,static_cast<u_int8_t>(IpParam.AppArea),V4APP_DEV_TYPE_SC,\
                           scidmuber, buf, length);

    return 0;
}

/**与udp同步参数，scid为此时的控制器架号***/
int DevNetMsgRunParamInit(u_int16_t scid)
{
    SystemParamIpType IpParam;
    SystemParamNeibDevInfType NeighbourParam;
    u_int16_t ScAddDir;
    u_int16_t Data1;
    u_int16_t Data2;
    u_int16_t Data3;
    u_int16_t Data4;
    u_int8_t buf[200];
    u_int16_t length = 0;

    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    pParamMng->GetSystemValueSystemIp(IpParam);
    pParamMng->GetSystemValueNeibDevInfType(NeighbourParam);
    ScAddDir = pParamMng->GetRunValueGlobalSupportDirect();

    /****2023.07.11,增加参数中途设置时,初始化IP及掩码网关等****/
    #ifdef PLATFORM_ARM
        BoradConfigIpMaskGateWay(IpParam,true);
    #endif
    /**特殊处理IP**/
    IpParam.IpAddr4 = scid;
    memmove(buf,&IpParam.AppArea,sizeof (SystemParamIpType));
    length += sizeof (SystemParamIpType);
    memmove(buf+length,&NeighbourParam.NeighbDevInf[0].Addr1,sizeof (SystemParamNeibDevInfType));
    length += sizeof (SystemParamNeibDevInfType);
    memmove(buf+length,&ScAddDir,sizeof (u_int16_t));
    length += sizeof (u_int16_t);

    pParamMng->GetRunValueGlobalSysServerMasterAddr(Data1,Data2,Data3,Data4);
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data2,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data3,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data4,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    pParamMng->GetRunValueGlobalSysServerSlaveAddr(Data1,Data2,Data3,Data4);
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data2,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data3,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    memmove(buf+length,&Data4,sizeof (u_int16_t));
    length += sizeof (u_int16_t);

    Data1 = 1;//架间检测使能
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    Data1 = 5000;//架间检测时间
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);
    Data1 = pParamMng->GetRunValueAllGlobalParamCrc();//CRC
    memmove(buf+length,&Data1,sizeof (u_int16_t));
    length += sizeof (u_int16_t);

    /***23.07.10，新增加版本信息****/
    buf[length] = A7CONTROLLER_LOGIC_VERSION_1;
    length++;
    buf[length] = A7CONTROLLER_LOGIC_VERSION_2;
    length++;
    buf[length] = A7CONTROLLER_LOGIC_VERSION_3;
    length++;
    buf[length] = A7CONTROLLER_LOGIC_VERSION_4;
    length++;

    /******/
    HysMsgTxInitDevInfProc(DRIVER_NET,IpParam.AppArea,V4APP_DEV_TYPE_SC,\
                           pParamMng->GetRunValueGlobalCurrentSupportID(),buf,length);

    return 0;
}
