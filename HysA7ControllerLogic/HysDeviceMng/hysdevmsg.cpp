#include "hysdevmsg.h"
#include "devicemngapi.h"
#include "./HysSystem/hyssystem.h"
#include "./HysAppInteraction/hysappinteraction.h"
#include <QDir>


/*****/
bool configaddrflag = false;

/*****/
int HysMsgRxDriverStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    return 0;
}

int HysMsgRxTransFileStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    qDebug()<<"***************************HysMsgRxTransFileStatus,******************"<<len;
    HysAppInteraction::getInston()->InsertSendMsgProc(MSG_TRANS_FILE_RESULT_REPORT,V4APP_DEV_TYPE_SC,numb,data,len);

    return 0;
}

int HysMsgRxRecvFileOkStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    qDebug()<<"***************************HysMsgRxTransFileStatus,******************"<<len;
    HysAppInteraction::getInston()->InsertSendMsgProc(MSG_RECV_FILE_RESULT_REPORT,V4APP_DEV_TYPE_SC,numb,data,len);

    return 0;
}

int HysMsgRxRecvFileIngDetailStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    return 0;
}

int HysMsgRxReportAddrConfigMode(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    u_int16_t mode ;
    u_int16_t errflag;
    HysSystem *pSystem = HysSystem::getInston();

    qDebug()<<"*************HysMsgRxReportAddrConfigMode********len******"<<len<<*(u_int8_t *)data<<*(u_int8_t *)(data+2);
    if(len != 0x04)
        return -1;
    memmove(&mode,data,0x02);
    memmove(&errflag,data+2,0x02);
    if(mode > MSG_ADDR_CONFIG_MODE_EXIT)
        return -1;
    if(errflag > MSG_ADDR_CONFIG_RESULT_TIMEOUT)
        return -1;

    if(mode == MSG_ADDR_CONFIG_MODE_ENTER)
    {
        pSystem->SetSystemSelfStateSlaveMode(SCSTATUS_SET_IP);
        pSystem->WriteLogApi(SYSTEM_EVENT_ADDR_CONFIG,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_ENTER,nullptr,0);
    }
    else if(mode == MSG_ADDR_CONFIG_MODE_EXIT)
    {
        pSystem->SetSystemSelfStateSlaveMode(SCSTATUS_NORMAL);

        if(errflag == MSG_ADDR_CONFIG_RESULT_SUCCEED)
            pSystem->WriteLogApi(SYSTEM_EVENT_ADDR_CONFIG,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_SUCCESS,nullptr,0);
        else if(errflag == MSG_ADDR_CONFIG_RESULT_TIMEOUT)
            pSystem->WriteLogApi(SYSTEM_EVENT_ADDR_CONFIG,LOG_TYPE_SYSTEM_ALARM,LOG_STATUS_OVERTIME,nullptr,0);
    }
    return 0;
}

int HysMsgRxReportAddrConfigResult(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    SystemParamIpType sysip;
    SystemParamNeibDevInfType neighbour;
    u_int16_t NewIpAddr1 = 0;
    u_int16_t NewIpAddr2 = 0;
    u_int16_t NewIpAddr3 = 0;
    u_int16_t NewIpAddr4 = 0;
    u_int16_t NewVlanLeft = 0;
    u_int16_t NewVlanRight = 0;

    qDebug()<<"****************HysMsgRxReportAddrConfigResult*len***%***new ip is*****"<<len<<*(u_int8_t *)(data+6);
    if(len != 52)//增加了子网掩码以及网关，共计16个字节
        return -1;
#if 0
    configaddrflag = true;
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    pParamMng->GetSystemValueSystemIp(sysip);
    pParamMng->GetSystemValueNeibDevInfType(neighbour);//邻架设备的具体MAC地址等

    /***VLAN****/
    memmove(&NewVlanLeft,data+24,0x02);
    memmove(&NewVlanRight,data+26,0x02);
    if(NewVlanLeft != sysip.VlanLeft)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_VLAN_LEFT_LINE_SET,NewVlanLeft,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewVlanRight != sysip.VlanRight)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_VLAN_RIGHT_LINE_SET,NewVlanRight,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);

    for(u_int16_t i = 0; i < 6; i++)
    {
        memmove(&NewVlanLeft,data+28+i*2,0x02);
        if(((neighbour.NeighbDevInf[0].Addr1 != NewVlanLeft) && (i == 0))\
          ||((neighbour.NeighbDevInf[0].Addr2 != NewVlanLeft) && (i == 1))\
          ||((neighbour.NeighbDevInf[0].Addr3 != NewVlanLeft) && (i == 2))\
          ||((neighbour.NeighbDevInf[0].Addr4 != NewVlanLeft) && (i == 3))\
          ||((neighbour.NeighbDevInf[0].Addr5 != NewVlanLeft) && (i == 4))\
          ||((neighbour.NeighbDevInf[0].Addr6 != NewVlanLeft) && (i == 5)))
            pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_DEV_LEFT_SET,i+1,NewVlanLeft,\
                                                   V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    }
    for(u_int16_t i = 0; i < 6; i++)
    {
        memmove(&NewVlanRight,data+40+i*2,0x02);
        if(((neighbour.NeighbDevInf[1].Addr1 != NewVlanRight) && (i == 0))\
          ||((neighbour.NeighbDevInf[1].Addr2 != NewVlanRight) && (i == 1))\
          ||((neighbour.NeighbDevInf[1].Addr3 != NewVlanRight) && (i == 2))\
          ||((neighbour.NeighbDevInf[1].Addr4 != NewVlanRight) && (i == 3))\
          ||((neighbour.NeighbDevInf[1].Addr5 != NewVlanRight) && (i == 4))\
          ||((neighbour.NeighbDevInf[1].Addr6 != NewVlanRight) && (i == 5)))
            pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_DEV_RIGHT_SET,i+1,NewVlanRight,\
                                                   V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    }

    /***网关***/
    memmove(&NewIpAddr1,data+16,0x02);
    memmove(&NewIpAddr2,data+18,0x02);
    memmove(&NewIpAddr3,data+20,0x02);
    memmove(&NewIpAddr4,data+22,0x02);
#if 0
    if(NewIpAddr1 != sysip.IpGateWay1)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET+5,NewIpAddr1,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewIpAddr2 != sysip.IpGateWay2)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET+6,NewIpAddr2,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewIpAddr3 != sysip.IpGateWay3)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET+7,NewIpAddr3,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewIpAddr4 != sysip.IpGateWay4)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET+8,NewIpAddr4,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
#endif

    /***子网掩码***/
    memmove(&NewIpAddr1,data+8,0x02);
    memmove(&NewIpAddr2,data+10,0x02);
    memmove(&NewIpAddr3,data+12,0x02);
    memmove(&NewIpAddr4,data+14,0x02);

#if 0
    if(NewIpAddr1 != sysip.IpMask1)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET+1,NewIpAddr1,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewIpAddr2 != sysip.IpMask2)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET+2,NewIpAddr2,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewIpAddr3 != sysip.IpMask3)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET+3,NewIpAddr3,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewIpAddr4 != sysip.IpMask4)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET+4,NewIpAddr4,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
#endif

    /***IP地址***/
    memmove(&NewIpAddr1,data,0x02);
    memmove(&NewIpAddr2,data+2,0x02);
    memmove(&NewIpAddr3,data+4,0x02);
    memmove(&NewIpAddr4,data+6,0x02);
#if 0
    if(NewIpAddr1 != sysip.IpAddr1)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR1_LINE_SET,NewIpAddr1,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewIpAddr2 != sysip.IpAddr2)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR2_LINE_SET,NewIpAddr2,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    if(NewIpAddr3 != sysip.IpAddr3)
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR3_LINE_SET,NewIpAddr3,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
#endif
    if(NewIpAddr4 != sysip.IpAddr4)
    {
        pParamMng->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET,NewIpAddr4,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
        pParamMng->InsertRunParamChangeProc(SC_PARAM_MENU_GLOBAL_SUPPORT_SET,SC_PARAM_CURRENT_SC_ID_LINE_SET,\
                                            NewIpAddr4,V4APP_PARAMCHANGGE_METHOD_LOCAL,JSON_PARAMINPUT_UNSIGNED_INT16);
    }
#endif
    return 0;
}

int HysMsgRxReportErrStatus(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    u_int16_t devtypetemp;
    u_int16_t devid;
    u_int16_t errflag;
    u_int16_t errstatus;

    u_int16_t status ;

    if(len != 0x08)
        return -1;
    memmove(&devtypetemp,data,0x02);
    memmove(&devid,data+2,0x02);
    memmove(&errflag,data+4,0x02);
    memmove(&errstatus,data+6,0x02);

    if(errstatus == MSG_ERR_STATUS_HAPPEN)
    {
        status = 1;
    }
    else if(errstatus == MSG_ERR_STATUS_RESTORE)
    {
        status = 0;
    }
    else
        return -1;

    /*****/
    if((devtypetemp == (V4APP_DEV_TYPE_SS_REPORT&0xFF))&&(devid == 1)&&(errflag ==  MSG_ERR_NEIGHBOUR_DEVIICE_COMM_ERR))
    {
        HysSystem::getInston()->SetRunScSelfServerCommErr(status);
    }

    /*****/
    if(devtypetemp == V4APP_DEV_TYPE_SC)
    {
        uint dir = 0;//0左1右
        if(devid == (LibHysA7ParamMng::getInston()->GetRunValueGlobalCurrentSupportID()+1))
        {
            if(LibHysA7ParamMng::getInston()->GetRunValueGlobalSupportDirect() == 1)
                dir = 1;
            else
                dir = 0;
        }
        else if(devid == (LibHysA7ParamMng::getInston()->GetRunValueGlobalCurrentSupportID()-1))
        {
            if(LibHysA7ParamMng::getInston()->GetRunValueGlobalSupportDirect() == 1)
                dir = 0;
            else
                dir = 1;
        }
        else
            return -1;

        /*****/
        if(errflag ==  MSG_ERR_GLOBAL_PARAM_DISAGREE)
        {
            if(devid == 0)
            {
                HysSystem::getInston()->SetSystemSelfStateAllGlobalParamSame(status);
            }
            else
            {
                if(dir == 0)
                    HysSystem::getInston()->SetSystemSelfStateLeftGlobalParamSame(status);
                else
                    HysSystem::getInston()->SetSystemSelfStateRightGlobalParamSame(status);
            }
        }

        /*****/
        if(errflag ==  MSG_ERR_NEIGHBOUR_DEVIICE_COMM_ERR)
        {
            if(devid == 0)
            {
                return -1;
            }
            else
            {
                if(dir == 0)
                    HysSystem::getInston()->SetSystemSelfStateLeftCommErr(status);
                else
                    HysSystem::getInston()->SetSystemSelfStateRightCommErr(status);
            }
        }

        /****MSG_ERR_NEIGHBOUR_DEVIICE_DISAGREE**暂不处理**/
    }
    return 0;
}

int HysMsgRxCommAreaCheckResult(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    u_int16_t errstatus;
    u_int16_t starttype;
    u_int16_t startid;
    u_int16_t endtype;
    u_int16_t endid;


    if(len != 10)
        return -1;

    memmove(&errstatus,data,0x02);
    memmove(&starttype,data+2,0x02);
    memmove(&startid,data+4,0x02);
    memmove(&endtype,data+6,0x02);
    memmove(&endid,data+8,0x02);

    if((startid < V4APP_DEV_ID_MIN)||(startid > V4APP_DEV_ID_MAX))
        return -1;
    if((endid < V4APP_DEV_ID_MIN)||(endid > V4APP_DEV_ID_MAX))
        return -1;

    HysRunCommArea temp;
    temp.StartScId = startid;
    temp.EndScId = endid;
    HysSystem::getInston()->SetRunCommAreaState(temp);

    return 0;
}

int HysMsgRsyncFileCheckResult(u_int8_t childdevtype, u_int8_t devtype, ushort numb, u_int8_t *data, int len)
{
    char SrcAddr[300] = {0};//源地址
    char DestAddr[300] = {0};//目标地址
    u_int16_t percent = 0;
    char sendbuf[500] = {0};
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    u_int16_t SelfScid = pParam->GetRunValueGlobalCurrentSupportID();
    u_int16_t DownPrgType = 0;
    u_int16_t DownDevType = 0;
    u_int16_t DriverTypeId = 0;
    u_int16_t DevTypeId = 0;

    if(len < 602)
        return -1;
    memset(SrcAddr,0x00,sizeof (SrcAddr));
    memset(DestAddr,0x00,sizeof (DestAddr));

    memmove(SrcAddr,data,sizeof (SrcAddr));
    memmove(DestAddr,data+300,sizeof (DestAddr));
    memmove(&percent,data+600,sizeof (u_int16_t));

    qDebug()<<"****************HysMsgRsyncFileCheckResult****%********"<<percent;
    memset(sendbuf,0x00,sizeof (sendbuf));

    sendbuf[0] = UPDATE_PRG_PRGTYPE_PROGRAM;//默认程序类型
    sendbuf[1] = 0;
    sendbuf[2] = childdevtype;
    sendbuf[3] = 0;
    sendbuf[4] = numb&0x00FF;
    sendbuf[5] = 0;
    sendbuf[6] = percent&0x00FF;
    sendbuf[7] = 0;
    HysAppInteraction::getInston()->InsertSendMsgProc(MSG_RECV_FILE_RESULT_REPORT,V4APP_DEV_TYPE_SC,SelfScid,(u_int8_t *)sendbuf,8);

    if(percent == 100)
    {
        if(false == DevGeneralGetUpdateFilePathRelatedInf(DestAddr,300,DownPrgType,DownDevType,DriverTypeId,DevTypeId))
            return -1;

        switch (DownDevType)
        {
            case EXTDEV_TYPE_SC:
                if(DownPrgType == UPDATE_PRG_PRGTYPE_PROGRAM)
                {
                    memset(sendbuf,0x00,sizeof (sendbuf));
                    sprintf(sendbuf,"chmod -R 777 %s",DestAddr);
                    system(sendbuf);
                    qDebug("update sc file: %s", sendbuf);

                    memset(sendbuf,0x00,sizeof (sendbuf));
                    sprintf(sendbuf,"%sinstall.sh",DestAddr);
                    qDebug("update sc file: %s", sendbuf);
                    #ifdef PLATFORM_ARM
                        system(sendbuf);

                        memset(sendbuf,0x00,sizeof (sendbuf));
                        sprintf(sendbuf,"reboot");
                        qDebug("update sc file: %s", sendbuf);
                        system(sendbuf);
                    #endif
                }
                else
                {
                    sprintf(sendbuf,"cd %s",DestAddr);
                    qDebug("update sc file: %s", sendbuf);
                    system(sendbuf);
                    /*引导具体脚本待定
                    sprintf(sendbuf,"./install");
                    qDebug("update sc file: %s", sendbuf);*/
                    #ifdef PLATFORM_ARM
                        system(sendbuf);
                    #endif
                }
                break;

            case EXTDEV_TYPE_DATA_CONVERTER:
                {
                    QDir dir(QString::fromLatin1(DestAddr));
                    QFileInfoList list = dir.entryInfoList();
                    if(list.size() == 0)
                        break;
                    sendbuf[0] = DownPrgType;
                    sendbuf[1] = 0;
                    sendbuf[2] = DevTypeId;
                    sendbuf[3] = 0;
                    sendbuf[4] = 0;
                    sendbuf[5] = 0;
                    sprintf(sendbuf+6,"%s%s",\
                            DestAddr,\
                            list.last().fileName().toLocal8Bit().data());
                    qDebug("send to driver :%d%s", DriverTypeId,sendbuf);
                    HysMsgTxTransFileProc(DriverTypeId,static_cast<uchar>(V4APP_DEV_TYPE_DYX),static_cast<uchar>(0),0,(u_int8_t *)sendbuf,sizeof (sendbuf));
                }
                break;


            default:
                {
                    QDir dir2(QString::fromLatin1(DestAddr));
                    QFileInfoList list2 = dir2.entryInfoList(QDir::Files|QDir::NoDotAndDotDot);
                    if(list2.size() == 0)
                        break;
                    sendbuf[0] = DownPrgType;
                    sendbuf[1] = 0;
                    sendbuf[2] = DevTypeId;
                    sendbuf[3] = 0;
                    sendbuf[4] = 0;
                    sendbuf[5] = 0;
                    sprintf(sendbuf+6,"%s%s",\
                            DestAddr,\
                            list2.last().fileName().toLocal8Bit().data());
                    qDebug("send to driver :%d,%d %d %d %d %d %d", DriverTypeId,sendbuf[0],sendbuf[1],sendbuf[2],sendbuf[3],sendbuf[4],sendbuf[5]);
                    HysMsgTxTransFileProc(DriverTypeId,static_cast<uchar>(DevTypeId),static_cast<uchar>(0),0,(u_int8_t *)sendbuf,sizeof (sendbuf));
                }
                break;
        }
    }
    return 0;
}

/***发送相关***/
int HysMsgTxTransFileProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len)
{
    u_int8_t pbuf[512];
    uchar programid = 0;
    uchar hardwareid = 0;

   if(-1 == DevCtrlGetProgramAndHardWareInf(driverid,programid,hardwareid))
       return -1;

    memset(pbuf,0x00,sizeof(pbuf));
    u_int8_t length = 0;
    pbuf[length++] = MSG_TRAN_FILE&0xFF;
    pbuf[length++] = (MSG_TRAN_FILE&0xFF00)>>8;
    pbuf[length++] = len&0x00FF;//长度
    pbuf[length++] = (len&0xFF00)>>8;//长度
    memmove(pbuf+length,data,static_cast<uint>(len));
    length += len;

    /****/
    if(programid != 0)
    {
        DeviceMngApi * pDevApi = DeviceMngApi::getInston();
        pDevApi->writemsgp(programid,hardwareid|devnumb,childdevnumb,\
                           static_cast<uchar>(point),pbuf,length);
        qDebug()<<"*****************HysMsgTxTransFileProc****************"<<programid<<(hardwareid|devnumb)<<childdevnumb<<point<<len<<","<<pbuf[4]<<pbuf[5]<<pbuf[6]<<pbuf[7];
    }
    return -1;
}

int HysMsgTxDownLoadFileProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len)
{
    u_int8_t pbuf[512];
    uchar programid = 0;
    uchar hardwareid = 0;

   if(-1 == DevCtrlGetProgramAndHardWareInf(driverid,programid,hardwareid))
       return -1;

    memset(pbuf,0x00,sizeof(pbuf));
    u_int8_t length = 0;
    pbuf[length++] = MSG_DOWNLOAD_FILE&0xFF;
    pbuf[length++] = (MSG_DOWNLOAD_FILE&0xFF00)>>8;
    pbuf[length++] = len&0x00FF;//长度
    pbuf[length++] = (len&0xFF00)>>8;//长度
    memmove(pbuf+length,data,static_cast<uint>(len));
    length += len;

    /****/
    if(programid != 0)
    {
        DeviceMngApi * pDevApi = DeviceMngApi::getInston();
        pDevApi->writemsgp(programid,hardwareid|devnumb,childdevnumb,\
                           static_cast<uchar>(point),pbuf,length);
    }
    return 0;
}

int HysMsgTxUpdatePrgFileProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len)
{
    u_int8_t pbuf[512];
    uchar programid = 0;
    uchar hardwareid = 0;

   if(-1 == DevCtrlGetProgramAndHardWareInf(driverid,programid,hardwareid))
       return -1;

    memset(pbuf,0x00,sizeof(pbuf));
    u_int8_t length = 0;
    pbuf[length++] = MSG_DOWNLOAD_FILE&0xFF;
    pbuf[length++] = (MSG_DOWNLOAD_FILE&0xFF00)>>8;
    pbuf[length++] = len&0x00FF;//长度
    pbuf[length++] = (len&0xFF00)>>8;//长度
    memmove(pbuf+length,data,static_cast<uint>(len));
    length += len;


    /****/
    if(programid != 0)
    {
        DeviceMngApi * pDevApi = DeviceMngApi::getInston();
        pDevApi->writemsgp(programid,hardwareid|devnumb,childdevnumb,\
                           static_cast<uchar>(point),pbuf,length);
    }
    return 0;
}

int HysMsgTxInitDevInfProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len)
{
    u_int8_t pbuf[512] = {0};
    uchar programid = 0;
    uchar hardwareid = 0;

   if(-1 == DevCtrlGetProgramAndHardWareInf(driverid,programid,hardwareid))
       return -1;

    memset(pbuf,0x00,sizeof(pbuf));
    int length = 0;
    pbuf[length++] = MSG_INITIAL_UDP_DRIVER&0xFF;
    pbuf[length++] = (MSG_INITIAL_UDP_DRIVER&0xFF00)>>8;
    pbuf[length++] = len&0xFF;//长度
    pbuf[length++] = (len&0xFF00)>>8;//长度
    memmove(pbuf+length,data,static_cast<uint>(len));
    length += len;

    /****/
    if(programid != 0)
    {
        DeviceMngApi * pDevApi = DeviceMngApi::getInston();
        pDevApi->writemsgp(programid,(hardwareid|devnumb),static_cast<uchar>(childdevnumb),\
                           static_cast<uchar>(point),pbuf,length);
        qDebug()<<"*****************HysMsgTxInitDevInfProc****************"<<programid<<(hardwareid|devnumb)<<childdevnumb<<point<<len;
    }
    return 0;
}

/***进入配址模式相关的***/
int HysMsgTxEnterSetIpConfigProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len)
{
    u_int8_t pbuf[512];
    uchar programid = 0;
    uchar hardwareid = 0;

    if(-1 == DevCtrlGetProgramAndHardWareInf(driverid,programid,hardwareid))
       return -1;

    memset(pbuf,0x00,sizeof(pbuf));
    u_int8_t length = 0;
    pbuf[length++] = MSG_REPORT_ADDR_CONFIG_MODE&0xFF;
    pbuf[length++] = (MSG_REPORT_ADDR_CONFIG_MODE&0xFF00)>>8;
    pbuf[length++] = len&0xFF;//长度
    pbuf[length++] = (len&0xFF00)>>8;//长度
    memmove(pbuf+length,data,static_cast<uint>(len));
    length += len;

    /****/
    if(programid != 0)
    {
        DeviceMngApi * pDevApi = DeviceMngApi::getInston();
        pDevApi->writemsgp(programid,(hardwareid|devnumb),childdevnumb,\
                           static_cast<uchar>(point),pbuf,length);
        qDebug()<<"*****************HysMsgTxEnterSetIpConfigProc****************"<<programid<<(hardwareid|devnumb)<<childdevnumb<<point<<len;
    }
    return 0;
}

/***进入通讯范围检测相关的***/
int HysMsgTxEnterCommAreaCheckProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len)
{
    u_int8_t pbuf[512];
    uchar programid = 0;
    uchar hardwareid = 0;

    if(-1 == DevCtrlGetProgramAndHardWareInf(driverid,programid,hardwareid))
       return -1;

    memset(pbuf,0x00,sizeof(pbuf));
    u_int8_t length = 0;
    pbuf[length++] = MSG_ENTER_COMM_AREA_CHECK&0xFF;
    pbuf[length++] = (MSG_ENTER_COMM_AREA_CHECK&0xFF00)>>8;
    pbuf[length++] = len&0xFF;//长度
    pbuf[length++] = (len&0xFF00)>>8;//长度
    memmove(pbuf+length,data,static_cast<uint>(len));
    length += len;

    /****/
    if(programid != 0)
    {
        DeviceMngApi * pDevApi = DeviceMngApi::getInston();
        pDevApi->writemsgp(programid,(hardwareid|devnumb),childdevnumb,\
                           static_cast<uchar>(point),pbuf,length);
        qDebug()<<"*****************HysMsgTxEnterCommAreaCheckProc****************"<<programid<<(hardwareid|devnumb)<<childdevnumb<<point<<len;
    }
    return 0;
}

/***进入同步文件相关的***/
int HysMsgTxRSyncFileProc(u_int8_t driverid,u_int8_t devnumb, u_int8_t childdevnumb, ushort point, u_int8_t *data, int len)
{
    u_int8_t pbuf[1024];
    uchar programid = 0;
    uchar hardwareid = 0;

    if(-1 == DevCtrlGetProgramAndHardWareInf(driverid,programid,hardwareid))
       return -1;

    memset(pbuf,0x00,sizeof(pbuf));
    u_int16_t length = 0;
    pbuf[length++] = MSG_ENTER_RSYNC_FILE_CHECK&0xFF;
    pbuf[length++] = (MSG_ENTER_RSYNC_FILE_CHECK&0xFF00)>>8;
    pbuf[length++] = len&0xFF;//长度
    pbuf[length++] = (len&0xFF00)>>8;//长度
    memmove(pbuf+length,data,static_cast<uint>(len));
    length += len;

    /****/
    if(programid != 0)
    {
        DeviceMngApi * pDevApi = DeviceMngApi::getInston();
        pDevApi->writemsgp(programid,(hardwareid|devnumb),childdevnumb,\
                           static_cast<uchar>(point),pbuf,length);
        qDebug()<<"*****************HysMsgTxRSyncFileProc****************"<<programid<<(hardwareid|devnumb)<<childdevnumb<<point<<len;
    }
    return 0;
}

/***获取相关***/
/***获取是否通过消息形式，已完成了配地址功能***/
bool HysMsgGetAddrConfigResultFlagAndClear(void)
{
    bool temp = configaddrflag;
    configaddrflag = false;
    return temp;
}
