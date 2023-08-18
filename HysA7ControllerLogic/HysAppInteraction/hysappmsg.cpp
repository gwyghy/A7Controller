/***********/
#include "hysappmsg.h"
#include "HysDeviceMng/devctrl.h"
#include "libhysa7parammng.h"
#include "hysappinteraction.h"
#include "Action/actionctrl.h"
#include "HysSystem/hyssystem.h"
#include "HysDeviceMng/hysdevmsg.h"
#include </usr/include/string.h>
#include <QDateTime>
#include <QDir>

/***************数据采集控制相关，1-100***************/
int HysAppMsgRxCtrlDevProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"********logic recv******HysAppMsgRxCtrlDevProc,start"<<len<<"data is:";
    qDebug()<<*(static_cast<u_int8_t *>(data))<<*(static_cast<u_int8_t *>(data+1))\
            <<*(static_cast<u_int8_t *>(data+2))<<*(static_cast<u_int8_t *>(data+3))<<*(static_cast<u_int8_t *>(data+4));
    qDebug()<<*(static_cast<u_int8_t *>(data+5))<<*(static_cast<u_int8_t *>(data+6))<<*(static_cast<u_int8_t *>(data+7))<<*(static_cast<u_int8_t *>(data+8));

    if(len == 12)
         qDebug()<<*(static_cast<u_int8_t *>(data+9))<<*(static_cast<u_int8_t *>(data+10))<<*(static_cast<u_int8_t *>(data+11));

    u_int16_t SelfScid = 0;
    u_int8_t ActType;
    u_int8_t ActId;
    SysCtrlInfoType  ctrlInfoTemp;
    ActionCtrl *pActCtrl = ActionCtrl::getInston();
    HysAppInteraction * pAppIter = HysAppInteraction::getInston();

    int ret = 0;

    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len < 0x09)
        ret = -1;
    if((devtype != V4APP_DEV_TYPE_SC))
        ret = -1;

    ActType = *(static_cast<u_int8_t *>(data));
    if((len < 12)&&(ActType == ENUM_GROUP_ACTION))
        ret = -1;

    /***增加成组参数的判定***/
    if(ActType == ENUM_GROUP_ACTION)
    {
        if((*(u_int8_t *)(data +11) != TriggerDirectType_LEFT)&&(*(u_int8_t *)(data +11) != TriggerDirectType_RIGHT))
            ret = -1;
    }
    /***增加成组参数的判定***/
    if(ActType == ENUM_GROUP_ACTION)
    {
        u_int8_t startscid = *(u_int8_t *)(data +9);
        u_int8_t endscid = *(u_int8_t *)(data +10);
        if(startscid > pParam->GetRunValueGlobalMaxSupportID())
            *(u_int8_t *)(data +9) = pParam->GetRunValueGlobalMaxSupportID();
        if(startscid < pParam->GetRunValueGlobalMinSupportID())
            *(u_int8_t *)(data +9) = pParam->GetRunValueGlobalMinSupportID();
        if(endscid > pParam->GetRunValueGlobalMaxSupportID())
            *(u_int8_t *)(data +10) = pParam->GetRunValueGlobalMaxSupportID();
        if(endscid < pParam->GetRunValueGlobalMinSupportID())
            *(u_int8_t *)(data +10) = pParam->GetRunValueGlobalMinSupportID();
    }
    //架间传输的数据长度为成组控制部分的数据长度，特此说明。

    /******/
    if((ret == 0)&&(numb == SelfScid))
    {
        ActId = *(static_cast<u_int8_t *>(data+1));
        memset(&ctrlInfoTemp,0,sizeof(SysCtrlInfoType));
        /****/
        switch(ActType)
        {
            case ENUM_GROUP_ACTION:
                ctrlInfoTemp.ParamLength = *(static_cast<u_int8_t *>(data+4));//架间传输的数据长度为成组控制部分的数据长度
                memcpy(ctrlInfoTemp.ParamBuf, data +9,ctrlInfoTemp.ParamLength);
            case ENUM_SINGLE_ACTION:
            case ENUM_COMBINATION_ACTION:
            case ENUM_ALARM:
                ctrlInfoTemp.actionType = static_cast<u8>(ActType);
                ctrlInfoTemp.actionID   = static_cast<u8>(ActId);
                ctrlInfoTemp.ctrl.CtrlCmd  = *(static_cast<u_int8_t *>(data+2));
                ctrlInfoTemp.ctrl.CtrlMode = *(static_cast<u_int8_t *>(data+3));
                ctrlInfoTemp.ctrl.CtrlSourceAddr = SelfScid;
                ctrlInfoTemp.ctrl.CtrlSourceType = ActionCtrl_SourceType_CONTROLLER;
                memmove(&ctrlInfoTemp.ctrl.UserID,data+7,0x02);
                ret = pActCtrl->InsCtrlBufQueue(&ctrlInfoTemp);
                break;

           default:
                break;
        }
    }

    if((ret == 0)&&(numb != SelfScid)\
      &&(numb >= pParam->GetRunValueGlobalMinSupportID())&&(numb <= pParam->GetRunValueGlobalMaxSupportID()))//规避群发动作的风险
    {
        qDebug()<<"********logic send to v4driver******HysAppMsgRxCtrlDevProc,start"<<len<<"data is:";
        qDebug()<<*(static_cast<u_int8_t *>(data))<<*(static_cast<u_int8_t *>(data+1))\
                <<*(static_cast<u_int8_t *>(data+2))<<*(static_cast<u_int8_t *>(data+3))<<*(static_cast<u_int8_t *>(data+4));
        qDebug()<<*(static_cast<u_int8_t *>(data+5))<<*(static_cast<u_int8_t *>(data+6))<<*(static_cast<u_int8_t *>(data+7))<<*(static_cast<u_int8_t *>(data+8));
        if(len == 12)
             qDebug()<<*(static_cast<u_int8_t *>(data+9))<<*(static_cast<u_int8_t *>(data+10))<<*(static_cast<u_int8_t *>(data+11));

        UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(numb),V4APP_STATUS_CTRL_DEV,data,len);
    }
    if(ret == 0)
        data[2] = 1;
    else
        data[2] = 0;
    pAppIter->InsertSendMsgProc(MSG_APP_CTRL_DEV_ACK,devtype,numb,data,3);

    return ret;
}

int HysAppMsgRxManualHeatBeatProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxManualHeatBeatProc,start";

    u_int16_t SelfScid = 0;
    ActionCtrl *pActCtrl = ActionCtrl::getInston();

    int ret = 0;

    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len == 0)
        ret = -1;
    if((devtype != V4APP_DEV_TYPE_SC))
        ret = -1;
    if(numb != SelfScid)
        ret = -1;

    if(ret == 0)
    {
        pActCtrl->SetManualActionSupportNum(SelfScid);
        for(uint i = 0;i < len && i < MANUALACTIONINFO_LEN;i++)
        {
            if((*(static_cast<u_int8_t *>(data))) == 0)
                break;
            if(false == pActCtrl->SetManualActionClearBuf(i,*(data+i)))//序号0开始
                break;
        }
    }
    else
        UdpAppProcCtrl(static_cast<uchar>(devtype),static_cast<uchar>(numb),V4APP_STATUS_MANUAL_ACT_HEARTBEAT,data,len);

    return 0;
}

int HysAppMsgRxFollowOnInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxFollowOnInquireProc,start"<<*(static_cast<u_int8_t *>(data))<<*(static_cast<u_int8_t *>(data+1));

    u_int16_t SelfScid = 0;
    int ret = 0;

    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len != 5)
        ret = -1;
    if((devtype != V4APP_DEV_TYPE_SC))
        ret = -1;
    if(numb != SelfScid)
        ret = -1;

    if(ret == 0)
    {
        LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
        u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();
        UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SS_REPORT),static_cast<uchar>(serverID),V4APP_FOLLOWON_INQUIRE,data,len);
    }
    return ret;
}

int HysAppMsgRxAutoBackFlushOnInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxAutoBackFlushOnInquireProc,start"<<*(static_cast<u_int8_t *>(data));

    u_int16_t SelfScid = 0;

    int ret = 0;

    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len != 2)
        ret = -1;
    if((devtype != V4APP_DEV_TYPE_SC))
        ret = -1;
    if(numb != SelfScid)
        ret = -1;

    if(ret == 0)
    {
        LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
        u_int16_t serverID = pParamMng->GetRunValueGlobalSysServerAddr();
        UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SS_REPORT),static_cast<uchar>(serverID),V4APP_AUTOBACKFLUSH_ONOFF_INQUIRE,data,len);
    }
    return ret;
}

int HysAppMsgRxKeyLedStatusCtrlProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    /*****/
    QDateTime time;
    time = QDateTime::currentDateTime();
//    qDebug()<<time.toString("yyyy-MM-dd hh:mm:ss.zzz")<<"**************HysAppMsgRxKeyLedStatusCtrlProc,start,len,data0,data120"<<len<<*(u_int8_t *)data<<*(u_int8_t *)(data+120);

    u_int16_t SelfScid = 0;

    int ret = 0;

    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len != 240)
        ret = -1;
    if((devtype != V4APP_DEV_TYPE_SC))
        ret = -1;
    if(numb != SelfScid)
        ret = -1;

    if(ret == 0)
    {
        DevCtrlInquire(DRIVER_UART2,EXTDEV_TYPE_KEYBOARD,0,V4DRIVER_STATUS_CTRL_DEV,data,len);
    }
    return ret;
}

int HysAppMsgRxEndKeyStatusProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxEndKeyStatusProc,start"<<len<<*data;

    u_int16_t SelfScid = 0;
    u_int8_t keyflag = 0;
    int ret = 0;

    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();
    memmove(&keyflag,data,0x01);

    if(len != 1)
        ret = -1;
    if((devtype != V4APP_DEV_TYPE_SC))
        ret = -1;
    if(numb != SelfScid)
        ret = -1;
    if(keyflag != 1)
        ret = -1;

    if(ret == 0)
    {
        HysSystem *pSystem = HysSystem::getInston();
        pSystem->SetSysStopStateEndFlag(SelfScid,1);
    }

    return ret;
}

/**************参数传输相关，101-200***************/
int HysAppMsgRxDisplayRunParamInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    u_int16_t SelfScid = 0;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();

    if(len != 0x09)
        return -1;
    if((devtype != V4APP_DEV_TYPE_SC)&&(devtype != V4APP_DEV_TYPE_SS_REPORT))
        return -1;
    if((numb > V4APP_DEV_ID_MAX)||(numb < V4APP_DEV_ID_MIN))
        return -1;

    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();
    if((devtype == V4APP_DEV_TYPE_SS_REPORT)||(SelfScid != numb))
    {
        UdpAppProcCtrl(static_cast<uchar>(devtype),static_cast<uchar>(numb),V4APP_DISPLAY_SS_PARAM_INQUIRE,data,len);
        return 0;
    }

    /****获取本架控制器的运行参数****/
    long long int menuid = 0;
    u_int8_t method = 0;
    long long int real_menuid = 0;
    memmove(&menuid,data,0x08);
    memmove(&method,data+8,0x01);
    qDebug("******FrameDisplayScrRunParamInquireProc,begin:  %llx,  %d",menuid,method);
    /**first_menuid需进行特殊处理**/
    menuid >>= 8;
    menuid |= SC_PARAM_MENU_FIRST_DEFAULT_SET;

    QList<ParamBasicRemoteTransType>list;
    list.clear();

    if(method == V4APP_DISPLAY_PARAM_MENU_TEXT)
    {
        pParam->InquireRunMenuTextProc(menuid,list);
    }
    else if(method == V4APP_DISPLAY_PARAM_MENU_NEXT_LIST)
    {
        pParam->InquireRunMenuNextListProc(menuid,list);
    }
    else if(method == V4APP_DISPLAY_PARAM_MENU_CONTENT)
    {
        if(pParam->InquireRunMenuContentProc(menuid,list,real_menuid) == -1)//yangyue0722
            real_menuid = 0;
    }
    else
        ;

    u_int8_t pBuf[2048];//此处的2048长度应该可以。最长的菜单项个数不会超过78个
    memset(pBuf,0x00,sizeof(pBuf));

    u_int16_t length = 0;
    memmove(pBuf,&menuid,0x08);
    memmove(pBuf+8,&method,0x01);
    /**real_menuid需进行特殊处理**/
    real_menuid <<= 8;
    memmove(pBuf+9,&real_menuid,0x08);

    pBuf[17] = static_cast<u_int8_t>(list.size());

    for(int i = 0; i < list.size();i++)
    {
        memmove(pBuf+18+static_cast<uint>(i)*sizeof(ParamBasicRemoteTransType),&(list.at(i).ParamName),sizeof(ParamBasicRemoteTransType));
    }
    length = static_cast<u_int16_t>(list.size())*sizeof(ParamBasicRemoteTransType)+sizeof(long long int)*2+sizeof(u_int8_t)*2;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_DISPLAY_DEV_PARAM_RESPONSE,devtype,numb,pBuf,length);
    qDebug("******FrameDisplayScRunParamInquireProc,result: %llx,  %d, %d, %llx, %d",menuid,method,list.size(),real_menuid,length);
    return 0;
}

int HysAppMsgRxReportRunParamChangedProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************FrameParamSetRunReportProc,start";
    u_int16_t SelfScid = 0;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len%18 != 0)
        return -1;
    if((devtype != V4APP_DEV_TYPE_SC)&&(devtype != V4APP_DEV_TYPE_SS_REPORT))
        return -1;
    if((numb > V4APP_DEV_ID_MAX)||(numb < V4APP_DEV_ID_MIN))
        return -1;
    if((numb > V4APP_DEV_SS_SLAVE)&&(devtype == V4APP_DEV_TYPE_SS_REPORT))
        return -1;

    if(devtype == V4APP_DEV_TYPE_SS_REPORT)
    {
        UdpAppProcCtrl(static_cast<uchar>(devtype),static_cast<uchar>(numb),V4APP_REPORTSS_PARAM_CHANGGED,data,len);
        return 0;
    }
    else if((devtype == V4APP_DEV_TYPE_SC)&&(SelfScid != numb))
    {
        return -1;
    }

    long long int menuid = 0;
    u_int16_t linenumber = 0;
    u_int16_t paramvalue = 0;
    u_int16_t method = 0;
    u_int16_t inputtype = 0;

    memmove(&menuid,data,0x08);
    menuid >>= 8;
    menuid |= SC_PARAM_MENU_FIRST_DEFAULT_SET;//进行特殊处理。因为Json是从V4参数开始查找的
    if(menuid == SC_PARAM_MENU_FIRST_DEFAULT_SET)
        return -1;
    memmove(&method,data+8,0x02);
    if(method > V4APP_PARAMCHANGGE_METHOD_GLOBAL)
        return -1;
    /**偏移10为长度，无用**/
    memmove(&linenumber,data+12,0x02);
    linenumber += 1;//进行特殊处理，控制器上报的数据从0开始
    if(method == V4APP_PARAMCHANGGE_METHOD_GLOBAL)
    {
        /***判断是否为设置架号的参数修改***/
        if((menuid != SC_PARAM_MENU_GLOBAL_SUPPORT_SET)\
            ||(linenumber != SC_PARAM_CURRENT_SC_ID_LINE_SET))
            UdpAppProcCtrl(static_cast<uchar>(devtype),V4APP_DEV_ID_ALL,V4APP_PARAM_SET_REPORT,data,len);
    }

    int ret = 0;
    for(int i = 0; i < len; i+= 18)
    {
        memmove(&menuid,data+i,0x08);
        menuid >>= 8;
        menuid |= SC_PARAM_MENU_FIRST_DEFAULT_SET;//进行特殊处理。因为Json是从V4参数开始查找的
        if(menuid == SC_PARAM_MENU_FIRST_DEFAULT_SET)
            break;
        memmove(&method,data+i+8,0x02);
        /**偏移10为长度，无用**/
        memmove(&linenumber,data+i+12,0x02);
        linenumber += 1;//进行特殊处理，控制器上报的数据从0开始
        memmove(&paramvalue,data+i+14,0x02);
        memmove(&inputtype,data+i+16,0x02);

        ret = pParam->InsertRunParamChangeProc(menuid,linenumber,paramvalue,method,inputtype);
        /***判断是否为设置架号的参数修改***/
        if((menuid == SC_PARAM_MENU_GLOBAL_SUPPORT_SET)\
            &&(linenumber == SC_PARAM_CURRENT_SC_ID_LINE_SET))
        {
            pParam->InsertSystemParamChangeProc(SYSTEM_PARAM_MENU_IP_ADDR_SET,SYSTEM_PARAM_IP_ADDR4_LINE_SET,\
                                                paramvalue,method,inputtype);
        }
    }
    /***进行应答***/
    memmove(data+8,&ret,0x02);
    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORTSS_PARAM_CHANGGED_RESPONSE,devtype,numb,data,10);
    qDebug("******HysAppMsgRxReportRunParamChangedProc,result: %llx,  %d, line = %d, ret = %d",menuid, method,linenumber,ret);

    return 0;
}

int HysAppMsgRxDisplaySystemParamInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    u_int16_t SelfScid = 0;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();

    if(len != 0x09)
        return -1;
    if((devtype != V4APP_DEV_TYPE_SC)&&(devtype != V4APP_DEV_TYPE_SS_REPORT))
        return -1;
    if((numb > V4APP_DEV_ID_MAX)||(numb < V4APP_DEV_ID_MIN))
        return -1;

    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();
    if((devtype == V4APP_DEV_TYPE_SS_REPORT)||(SelfScid != numb))
    {
        UdpAppProcCtrl(static_cast<uchar>(devtype),static_cast<uchar>(numb),V4APP_DISPLAY_SS_PARAM_INQUIRE,data,len);
        return 0;
    }

    /****获取本架控制器的运行参数****/
    long long int menuid = 0;
    u_int8_t method = 0;
    long long int real_menuid = 0;
    memmove(&menuid,data,0x08);
    memmove(&method,data+8,0x01);
    qDebug("******FrameDisplayScrSystemParamInquireProc,begin:  %llx,  %d",menuid,method);

    QList<ParamBasicRemoteTransType>list;
    list.clear();

    if(method == V4APP_DISPLAY_PARAM_MENU_TEXT)
    {
        pParam->InquireSystemMenuTextProc(menuid,list);
    }
    else if(method == V4APP_DISPLAY_PARAM_MENU_NEXT_LIST)
    {
        pParam->InquireSystemMenuNextListProc(menuid,list);
    }
    else if(method == V4APP_DISPLAY_PARAM_MENU_CONTENT)
    {
        if(pParam->InquireSystemMenuContentProc(menuid,list,real_menuid) == -1)//yangyue0722
           real_menuid = 0;
    }
    else
        ;
    u_int8_t pBuf[2048];//此处的2048长度应该可以。最长的菜单项个数不会超过78个
    memset(pBuf,0x00,sizeof(pBuf));

    u_int16_t length = 0;

    memmove(pBuf,&menuid,0x08);
    memmove(pBuf+8,&method,0x01);
    memmove(pBuf+9,&real_menuid,0x08);
    pBuf[17] = static_cast<u_int8_t>(list.size());

    for(int i = 0; i < list.size();i++)
    {
        memmove(pBuf+18+static_cast<uint>(i)*sizeof(ParamBasicRemoteTransType),&(list.at(i).ParamName),sizeof(ParamBasicRemoteTransType));
    }
    length = static_cast<u_int16_t>(list.size())*sizeof(ParamBasicRemoteTransType)+sizeof(long long int)*2+sizeof(u_int8_t)*2;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_DISPLAY_SYSTEM_PARAM_RESPONSE,devtype,numb,pBuf,length);
    qDebug("******FrameDisplayScSystemParamInquireProc,result: %llx,  %d, %d, %llx, %d",menuid,method,list.size(),real_menuid,length);
    return 0;
}

int HysAppMsgRxReportSystemParamChangedProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxReportSystemParamChangedProc,start";
    u_int16_t SelfScid = 0;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len%18 != 0)
        return -1;
    if((devtype != V4APP_DEV_TYPE_SC)&&(devtype != V4APP_DEV_TYPE_SS_REPORT))
        return -1;
    if((numb > V4APP_DEV_ID_MAX)||(numb < V4APP_DEV_ID_MIN))
        return -1;
    if((numb > V4APP_DEV_SS_SLAVE)&&(devtype == V4APP_DEV_TYPE_SS_REPORT))
        return -1;

    if(devtype == V4APP_DEV_TYPE_SS_REPORT)
    {
        UdpAppProcCtrl(static_cast<uchar>(devtype),static_cast<uchar>(numb),V4APP_REPORTSS_SYSTEM_CHANGGED,data,len);
        return 0;
    }
    else if((devtype == V4APP_DEV_TYPE_SC)&&(SelfScid != numb))
    {
        return -1;
    }

    long long int menuid = 0;
    u_int16_t linenumber = 0;
    u_int16_t paramvalue = 0;
    u_int16_t method = 0;
    u_int16_t inputtype = 0;

    memmove(&method,data+8,0x02);//本地/全面
    if(method > V4APP_PARAMCHANGGE_METHOD_GLOBAL)
       return -1;
    if(method == V4APP_PARAMCHANGGE_METHOD_GLOBAL)
       UdpAppProcCtrl(static_cast<uchar>(devtype),V4APP_DEV_ID_ALL,V4APP_REPORTSS_SYSTEM_CHANGGED,data,len);

    int ret = 0;
    for(int i = 0; i < len ;i+=18)
    {
        memmove(&menuid,data+i,0x08);
        memmove(&method,data+i+8,0x02);//本地/全面
        /**偏移10为长度，无用**/
        memmove(&linenumber,data+i+12,0x02);
        linenumber += 1;//进行特殊处理，控制器上报的数据从0开始
        memmove(&paramvalue,data+i+14,0x02);
        memmove(&inputtype,data+i+16,0x02);

        ret = pParam->InsertSystemParamChangeProc(menuid,linenumber,paramvalue,method,inputtype);
        /***判断是否为设置架号的参数修改***/
        if((menuid == SYSTEM_PARAM_MENU_IP_ADDR_SET )\
            &&(linenumber == SYSTEM_PARAM_IP_ADDR4_LINE_SET))
        {
            pParam->InsertRunParamChangeProc(SC_PARAM_MENU_GLOBAL_SUPPORT_SET,SC_PARAM_CURRENT_SC_ID_LINE_SET ,\
                                                paramvalue,method,inputtype);
        }
    }

    /***进行应答***/
    memmove(data+8,&ret,0x02);
    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORTSS_SYSTEM_CHANGGED_RESPONSE,devtype,numb,data,10);
    qDebug("******HysAppMsgRxReportRunParamChangedProc,result: %llx,  %d",menuid, method);

    return 0;
}

/*************网络维护及文件传输相关，201-300.以下为自定义*****************/
int HysAppMsgRxServiceResetSc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceResetSc,start"<<len<<*data<<*(data+1)<<*(data+2);
    #ifdef PLATFORM_ARM
        system("reboot");
    #endif
    return 0;
}

int HysAppMsgRxServiceTransEmvdIoConf(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceTransEmvdIoConf,start"<<len<<*data<<*(data+1)<<*(data+2);
    return 0;
}

int HysAppMsgRxServiceTransSensorConf(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceTransSensorConf,start"<<len<<*data<<*(data+1)<<*(data+2);
    return 0;
}

int HysAppMsgRxServiceTransAssistKeyConf(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceTransAssistKeyConf,start"<<len<<*data<<*(data+1)<<*(data+2);
    return 0;
}
int HysAppMsgRxServiceTransKeyConf(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceTransKeyConf,start"<<len<<*data<<*(data+1)<<*(data+2);
    return 0;
}

int HysAppMsgRxServiceTransGlobalParam(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceTransGlobalParam,start"<<len<<*data<<*(data+1)<<*(data+2);
    return 0;
}

int HysAppMsgRxServiceTransAllParam(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceTransAllParam,start"<<len<<*data<<*(data+1)<<*(data+2);
    return 0;
}

int HysAppMsgRxServiceRestoreDefaultParam(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceTransRestoreDefaultParam,start"<<len<<*data<<*(data+1)<<*(data+2);
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    pParamMng->SystemConfigValueFileRestoreDefault();
    pParamMng->RunConfigValueFileRestoreDefault();

    return 0;
}

/*****/
int HysAppMsgRxServiceOperateInfProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxServiceOperateInfProc,start"<<len<<*data<<*(data+1)<<*(data+2);

    if(len != 3)
        return -1;

    u_int16_t changitem = 0;
    u_int16_t changemethod = 0;
    memmove(&changitem,data,0x01);
    memmove(&changemethod,data+1,0x02);

    if(changemethod != CHANGED_METHOD_SELF)
    {
        if((changitem == MSG_SERVICE_OPERATE_RESET_SC)\
          ||(changitem == MSG_SERVICE_OPERATE_RESTORE_DEFAULT_PARAM))
        {
            numb = 0;
            if(changitem == MSG_SERVICE_OPERATE_RESET_SC)
                changitem = AppProcServerType_reset_controller;
            else
                changitem = AppProcServerType_reset_defaultParam;
            UdpAppProcCtrl(static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(numb),V4APP_SERVER_OPERRATE_REQUEST,(u_int8_t *)(&changitem),2);
        }
        else
        {
            return 0;
        }
    }

    if((changemethod == CHANGED_METHOD_RIGHT)\
        ||(changemethod == CHANGED_METHOD_LEFT)\
        ||(devtype != V4APP_DEV_TYPE_SC))
        return  -1;

    switch(changitem)
    {
        case  MSG_SERVICE_OPERATE_RESET_SC:
            HysAppMsgRxServiceResetSc(devtype,numb,data,len);
            break;

        case  MSG_SERVICE_OPERATE_TRANS_EMVD_IO_CONFIG:
            HysAppMsgRxServiceTransEmvdIoConf(devtype,numb,data,len);
            break;

        case  MSG_SERVICE_OPERATE_TRANS_SENSOR_CONFIG:
            HysAppMsgRxServiceTransSensorConf(devtype,numb,data,len);
            break;

        case  MSG_SERVICE_OPERATE_TRANS_ASSIST_KEY:
            HysAppMsgRxServiceTransAssistKeyConf(devtype,numb,data,len);
            break;

        case  MSG_SERVICE_OPERATE_TRANS_KEY_CONFIG:
            HysAppMsgRxServiceTransKeyConf(devtype,numb,data,len);
            break;

        case  MSG_SERVICE_OPERATE_TRANS_GLOBAL_PARAM:
            HysAppMsgRxServiceTransGlobalParam(devtype,numb,data,len);
            break;

        case  MSG_SERVICE_OPERATE_TRANS_ALL_PARAM:
            HysAppMsgRxServiceTransAllParam(devtype,numb,data,len);
            break;

        case  MSG_SERVICE_OPERATE_RESTORE_DEFAULT_PARAM:
            HysAppMsgRxServiceRestoreDefaultParam(devtype,numb,data,len);
            break;
    }
    return 0;
}

int HysAppMsgRxDownloadDevPrgCxbProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxDownloadDevPrgCxbProc,start";
    u_int16_t SelfScid = 0;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len != 0x06)
        return -1;
    if((devtype != V4APP_DEV_TYPE_SC))
        return -1;
    if(numb != SelfScid)
        return -1;

    HysMsgTxDownLoadFileProc(DRIVER_CAN1,EXTDEV_TYPE_CXB,1,0,data,len);
    return 0;
}

int HysAppMsgRxDownloadDevPrgRemoteProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxDownloadDevPrgCxbProc,start";
    u_int16_t SelfScid = 0;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len != 0x06)
        return -1;

    HysMsgTxDownLoadFileProc(DRIVER_NET,V4APP_DEV_TYPE_SC,static_cast<uchar>(SelfScid),0,data,len);
    return 0;
}

/***进程更新请求****/
int HysGetUpdataPrgSrcAddrAndDestAddr(u_int16_t DownPrgType ,u_int16_t DownDevType, u_int64_t DownPrgFromDir, QString &srcaddr, QString &destaddr)
{
    char SrcAddr[300] = {0};//源地址
    char DestAddr[300] = {0};//目标地址
    u_int8_t sendbuf[600] = {0};
    QString filepath;
    QString filename;
    u_int16_t DriverTypeId;
    u_int16_t DevTypeId;

    u_int16_t TargetDeviceNumer = 0;
    u_int16_t SelfScid = 0;

    if(DownPrgType > UPDATE_PRG_PRGTYPE_MAX)
         return -1;
    if(DownDevType > EXTDEV_TYPE_MAX)
        return -1;
    if(DownPrgFromDir > UPDATE_PRG_METHOD_RIGHT)
        return -1;

    if(0 != DevGeneralGetUpdateFilePath(DownPrgType,DownDevType,filepath,filename,DriverTypeId,DevTypeId))
        return -1;

    memset(SrcAddr,0x00,sizeof (SrcAddr));
    memset(DestAddr,0x00,sizeof (DestAddr));
    memset(sendbuf,0x00,sizeof (sendbuf));

    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if((DownPrgFromDir == UPDATE_PRG_METHOD_RIGHT) || (DownPrgFromDir == UPDATE_PRG_METHOD_LEFT))
    {
        if((DownPrgFromDir == UPDATE_PRG_METHOD_RIGHT)&&(pParam->GetRunValueGlobalSupportDirect() == 1))
            TargetDeviceNumer =  SelfScid+1;
        else if((DownPrgFromDir == UPDATE_PRG_METHOD_LEFT)&&(pParam->GetRunValueGlobalSupportDirect() == 0))
            TargetDeviceNumer =  SelfScid+1;
        else
            TargetDeviceNumer =  SelfScid-1;
        if((TargetDeviceNumer < pParam->GetRunValueGlobalMinSupportID())\
            ||(TargetDeviceNumer > pParam->GetRunValueGlobalMaxSupportID()))
            return -1;
    }
    else if(DownPrgFromDir == UPDATE_PRG_METHOD_ALL)
        TargetDeviceNumer = V4APP_DEV_ID_ALL;
    else
        TargetDeviceNumer = SelfScid;

    /***源地址***/
    if((DownPrgFromDir == UPDATE_PRG_METHOD_ALL)||(DownPrgFromDir == UPDATE_PRG_METHOD_SELF))
    {
        SystemDownloadFromIpType form;//更新来源
        memset(&form.DownLoadFrom,0x00,sizeof (SystemDownloadFromIpType));
        pParam->GetSystemValueDownloadFromIp(form);
        if((form.DownLoadFrom == SYSTEM_DOWNLOAD_FROM_SUPPORT_SERVER)&&(DownPrgType == UPDATE_PRG_PRGTYPE_PROGRAM))
        {
            sprintf(SrcAddr,"%s@%d.%d.%d.%d:%s%s",QString(SS_REMETE_LOGIN_USER).toLocal8Bit().data(),form.SSIpAddr1,form.SSIpAddr2,form.SSIpAddr3,form.SSIpAddr4,\
                    QString(SS_APP_PROGRAM_DOWN_FILE_PATH).toLocal8Bit().data(),\
                    filename.toLocal8Bit().data());
        }
        else if((form.DownLoadFrom == SYSTEM_DOWNLOAD_FROM_SUPPORT_SERVER)&&(DownPrgType == UPDATE_PRG_PRGTYPE_BOOT))
        {
            sprintf(SrcAddr,"%s@%d.%d.%d.%d:%s%s",QString(SS_REMETE_LOGIN_USER).toLocal8Bit().data(),form.SSIpAddr1,form.SSIpAddr2,form.SSIpAddr3,form.SSIpAddr4,\
                    QString(SS_BOOT_PROGRAM_DOWN_FILE_PATH).toLocal8Bit().data(),\
                    filename.toLocal8Bit().data());
        }
        else if((form.DownLoadFrom == SYSTEM_DOWNLOAD_FROM_SUPPORT_SERVER)&&(DownPrgType == UPDATE_PRG_PRGTYPE_PROGRAM))
        {
            sprintf(SrcAddr,"%s@%d.%d.%d.%d:%s%s",QString(MD_REMETE_LOGIN_USER).toLocal8Bit().data(),form.SSIpAddr1,form.SSIpAddr2,form.SSIpAddr3,form.SSIpAddr4,\
                    QString(MD_APP_PROGRAM_DOWN_FILE_PATH).toLocal8Bit().data(),\
                    filename.toLocal8Bit().data());
        }
        else
        {
            sprintf(SrcAddr,"%s@%d.%d.%d.%d:%s%s",QString(MD_REMETE_LOGIN_USER).toLocal8Bit().data(),form.SSIpAddr1,form.SSIpAddr2,form.SSIpAddr3,form.SSIpAddr4,\
                    QString(MD_BOOT_PROGRAM_DOWN_FILE_PATH).toLocal8Bit().data(),\
                    filename.toLocal8Bit().data());
        }
    }
    else//左右架的源地址
    {
        SystemParamIpType common;
        pParam->GetSystemValueSystemIp(common);
        sprintf(SrcAddr,"%s@%d.%d.%d.%d:%s%s",QString(SC_REMETE_LOGIN_USER).toLocal8Bit().data(),\
                common.IpAddr1,common.IpAddr2,common.IpAddr3,TargetDeviceNumer,\
                filepath.toLocal8Bit().data(),\
                filename.toLocal8Bit().data());
    }
    /***目的地址***/
    sprintf(DestAddr,"%s%s",filepath.toLocal8Bit().data(),filename.toLocal8Bit().data());

    if(DevGeneralIsUpdateFilePathAvalid(SrcAddr,300,DestAddr,300) == false)
        return -1;

    if(DevTypeId != EXTDEV_TYPE_SC)//控制器文件夹太大，不能清除
    {
        QDir dir(QString::fromLocal8Bit(DestAddr));
        QFileInfoList fileList = dir.entryInfoList(QDir::Files|QDir::NoDotAndDotDot); // 获取所有的文件信息
        foreach (QFileInfo file, fileList){ //遍历文件信息
            if (file.isFile()){ // 是文件，删除
                file.dir().remove(file.fileName());
            }
        }
    }


    memmove(sendbuf,SrcAddr,sizeof (SrcAddr));
    memmove(sendbuf+sizeof (SrcAddr),DestAddr,sizeof (DestAddr));
    /******/

    if((DriverTypeId == DRIVER_NET)&&(DevTypeId != EXTDEV_TYPE_SC))//非控制器的设备，需进行单独下载并更新
    {
        //UdpAppProcCtrl(static_cast<uchar>(DevTypeId),static_cast<uchar>(TargetDeviceNumer),V4APP_RSYNC_FILE_INQUIRE,sendbuf,600);
        HysMsgTxRSyncFileProc(DRIVER_NET,static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(SelfScid),0,sendbuf,600);
        return 0;
    }
    if(DownPrgFromDir == UPDATE_PRG_METHOD_ALL)
        UdpAppProcCtrl(static_cast<uchar>(DevTypeId),static_cast<uchar>(TargetDeviceNumer),V4APP_RSYNC_FILE_INQUIRE,sendbuf,600);

    HysMsgTxRSyncFileProc(DRIVER_NET,static_cast<uchar>(V4APP_DEV_TYPE_SC),static_cast<uchar>(SelfScid),0,sendbuf,600);

    #if 0//for test,测试文件同步
        /******/
        u_int8_t sendbuf2[602] = {0};
        memmove(sendbuf2,sendbuf,sizeof (sendbuf));
        sendbuf2[600] = 100;
        sendbuf2[601] = 0;
        HysMsgRsyncFileCheckResult(1,1,179,sendbuf2,602);//for test
    #endif
    return 0;
}

/***进程更新请求****/
int HysAppMsgRxUpdateDevPrgProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    qDebug()<<"**************HysAppMsgRxUpdateDevPrgProc,start,devtype,numb,len"<<devtype<<numb<<len;
    u_int16_t SelfScid = 0;
    u_int16_t DownPrgType = 0;//更新程序类型
    u_int16_t DownDevType = 0;//更新设备类型
    u_int64_t DownPrgFromDir = 0;//更新程序时，获取的方向
    QString srcaddr;
    QString destaddr;

    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();

    memmove(&DownPrgType,data,0x02);
    memmove(&DownDevType,data+2,0x02);
    memmove(&DownPrgFromDir,data+4,0x02);
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();

    if(len < 0x06)
        return -1;
    if((devtype != V4APP_DEV_TYPE_SC))
        return -1;
    if(numb != SelfScid)
        return -1;
    if(DownPrgType > UPDATE_PRG_PRGTYPE_MAX)
         return -1;
    if(DownDevType > EXTDEV_TYPE_MAX)
        return -1;
    if(DownPrgFromDir > UPDATE_PRG_METHOD_RIGHT)
        return -1;

    /***进行应答***/
    HysAppInteraction::getInston()->InsertSendMsgProc(MSG_UPDATE_DEV_PRG_ACK,devtype,numb,data,len);

    /***进行处理***/
    return HysGetUpdataPrgSrcAddrAndDestAddr(DownPrgType,DownDevType,DownPrgFromDir,srcaddr,destaddr);
}

int HysAppMsgCheckCommAreaInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    if(len != 2)
        return -1;

    u_int16_t mode = 0;
    memmove(&mode,data,0x02);
    if(mode != 1)
        return -1;

    HysRunCommArea temp;
    temp.StartScId = 0xFFFF;
    temp.EndScId = 0XFFFF;
    HysSystem::getInston()->SetRunCommAreaState(temp);

    u_int16_t SelfScid = 0;
    LibHysA7ParamMng *pParam = LibHysA7ParamMng::getInston();
    SelfScid = pParam->GetRunValueGlobalCurrentSupportID();
    HysMsgTxEnterCommAreaCheckProc(DRIVER_NET,V4APP_DEV_TYPE_SC,static_cast<uchar>(SelfScid),0,data,len);
    return 0;
}

int HysAppMsgCheckDataTransInfInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    u_int16_t mode = 0;
    u_int8_t buf[6] = {0};

    if(len != 2)
        return -1;

    memmove(&mode,data,0x02);
    if(mode != 1)
        return -1;

    /****/
    buf[0] = ENUM_DEVICE;
    buf[1] = 0;
    buf[2] = 0;
    UdpAppProcCtrl(V4APP_DEV_TYPE_DYX,0,V4APP_STATUS_GET_MULTIPLE_INQUIRE,\
                   buf,3);
    return 0;
}

int HysAppMsgCheckEmvdPointStatusInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    u_int16_t mode = 0;
    HysScExtDevType extdevstate;
    u_int8_t buf[6] = {0};

    qDebug()<<"*****HysAppMsgCheckEmvdPointStatusInquireProc,000****"<<devtype<<numb\
           <<*(u_int8_t *)data<<len;


    u_int16_t SelfScid = LibHysA7ParamMng::getInston()->GetRunValueGlobalCurrentSupportID();

    if((devtype != V4APP_DEV_TYPE_SC))
        return -1;
    if(numb != SelfScid)
        return -1;

    if(len != 2)
        return -1;

    memmove(&mode,data,0x02);
    qDebug()<<"*****HysAppMsgCheckEmvdPointStatusInquireProc,111****"<<devtype<<numb\
           <<*(u_int8_t *)data<<len<<mode;
    if(mode == 0)
        return -1;

    HysSystem::getInston()->GetExtDevState(2,extdevstate);
    SingleActionParamType *pSingleparam = LibHysA7ParamMng::getInston()->GetRunValueSingleActParam(mode);

    if(pSingleparam != nullptr)
        qDebug()<<"*****HysAppMsgCheckEmvdPointStatusInquireProc,222****"<<devtype<<numb\
               <<*(u_int8_t *)data<<len<<mode<<pSingleparam->baseParam.pointID.point<<extdevstate.Status;
    else
        qDebug()<<"*****HysAppMsgCheckEmvdPointStatusInquireProc,err**pSingleparam == nullptr**";

    if(extdevstate.Status == OnlineType_OFFLINE)
    {
        buf[0] = static_cast<u_int8_t>(mode);
        buf[1] = 0;
        buf[2] = SENSOR_FAULT_STATUS_FAULT;
        buf[3] = 0;
        buf[4] = EmvdPointStatusType_TimeOut;
        buf[5] = 0;

        HysAppInteraction * pAppIter = HysAppInteraction::getInston();
        pAppIter->InsertSendMsgProc(MSG_CHECK_EMVD_POINT_STATUS_RESPONSE,\
                                    devtype,numb,\
                                    buf,6);
        qDebug()<<"*****HysAppMsgCheckEmvdPointStatusInquireProc,ack1**extdevstate.Status == SENSOR_FAULT_STATUS_FAULT**";

    }
    else if((pSingleparam == nullptr)||(pSingleparam->baseParam.pointID.point == 0))
    {
        buf[0] = static_cast<u_int8_t>(mode);
        buf[1] = 0;
        buf[2] = SENSOR_FAULT_STATUS_NORMAL;
        buf[3] = 0;
        buf[4] = EmvdPointStatusType_NotSet;
        buf[5] = 0;

        HysAppInteraction * pAppIter = HysAppInteraction::getInston();
        pAppIter->InsertSendMsgProc(MSG_CHECK_EMVD_POINT_STATUS_RESPONSE,\
                                    devtype,numb,\
                                    buf,6);
        qDebug()<<"*****HysAppMsgCheckEmvdPointStatusInquireProc,ack2**(pSingleparam == nullptr)||(pSingleparam->baseParam.pointID.point == 0)**";
    }
    else
    {
        uchar programid = 0;
        uchar hardwareid = 0;

        DevCtrlGetProgramAndHardWareInf(DRIVER_CAN2,programid,hardwareid);
        DeviceMngApi *pDevApi = DeviceMngApi::getInston();
        pDevApi->writeshmp(programid,hardwareid|EXTDEV_TYPE_EMVD,\
                           1,static_cast<uchar>(pSingleparam->baseParam.pointID.point),\
                           V4DRIVER_STATUS_GET_SINGLE_INQUIRE,buf,0);
        qDebug()<<"*****HysAppMsgCheckEmvdPointStatusInquireProc send****"<<programid<<(hardwareid|EXTDEV_TYPE_EMVD)\
               <<1<<pSingleparam->baseParam.pointID.point<<V4DRIVER_STATUS_GET_SINGLE_INQUIRE;
    }
    return 0;
}

/**************数据发送相关*******************/
/**服务器参数的新修改***/
int HysAppMsgSendSSDisplayResponseProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len)
{
    if(devtype != V4APP_DEV_TYPE_SS_REPORT)
        return -1;
    if(numb > V4APP_DEV_SS_SLAVE)
        return -1;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_DISPLAY_DEV_PARAM_RESPONSE,\
                                devtype,numb,\
                                data,len);
    return 0;
}

/********上报服务类参数修改(含密码)******/
int HysAppMsgSendServiceParamChanedProc(void)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    GlobalParam_InCommonUse common;
    pParamMng->GetRunValueGlobalInCommonUse(common);

    /*****/
    u_int8_t Buf[1024];
    memset(Buf,0x00,sizeof (Buf));
    Buf[0] = (sizeof (GlobalParam_InCommonUse)&0x00FF);
    Buf[1] = (sizeof (GlobalParam_InCommonUse)&0xFF00)>>8;
    memmove(Buf+2,&common.currentSupportID,sizeof (GlobalParam_InCommonUse));

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORT_SERVICE_PARAM_CHANGGED,\
                                V4APP_DEV_TYPE_SC,pParamMng->GetRunValueGlobalCurrentSupportID(),\
                                Buf,sizeof (GlobalParam_InCommonUse)+2);

    return 0;
}

//上报按键配置参数修改
int HysAppMsgSendKeyConfigParamChanedProc(void)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    globalParam_KeyType key;
    pParamMng->GetRunValueGlobalKeyConfParam(key);//键盘配置中的具体参数

    /*****/
    u_int8_t Buf[1024];
    u_int16_t lenth = 0;
    memset(Buf,0x00,sizeof (Buf));

    for(int i = 0; i < key.KeyConfigList.size(); i ++)
    {
        memmove(Buf+2+i*2,&(key.KeyConfigList.at(i)),0x02);
        lenth += 2;
    }
    Buf[0] = lenth&0xFF;
    Buf[1] = (lenth&0xFF00)>>8;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORT_KEYCONFIG_PARAM_CHANGGED,\
                                V4APP_DEV_TYPE_SC,pParamMng->GetRunValueGlobalCurrentSupportID(),\
                                Buf,lenth+2);
    return 0;
}
//上报辅助菜单参数修改
int HysAppMsgSendAssintConfigParamChanedProc(void)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    globalParam_AssistKeyType assist;
    pParamMng->GetRunValueGlobalAssistKeyParam(assist);//辅助动作中的具体参数

    /*****/
    u_int8_t Buf[1024];
    u_int16_t lenth = 0;
    memset(Buf,0x00,sizeof (Buf));

    for(int i = 0; i < assist.AssistKeyConfigList.size(); i ++)
    {
        memmove(Buf+2+i*2,&(assist.AssistKeyConfigList.at(i)),0x02);
        lenth += 2;
    }
    Buf[0] = lenth&0xFF;
    Buf[1] = (lenth&0xFF00)>>8;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORT_ASSINTCONFIG_PARAM_CHANGGED,\
                                V4APP_DEV_TYPE_SC,pParamMng->GetRunValueGlobalCurrentSupportID(),\
                                Buf,lenth+2);
    return 0;
}
//上报主界面显示参数修改
int HysAppMsgSendHomeDisplayParamChanedProc(void)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    globalParam_UIConfigType uiconf;
    pParamMng->GetRunValueGlobalUIConfigParam(uiconf);//主界面显示中的具体参数

    /*****/
    u_int8_t Buf[1024];
    u_int16_t lenth = 0;
    memset(Buf,0x00,sizeof (Buf));

    for(int i = 0; i < uiconf.UISensorConfigList.size(); i ++)
    {
        memmove(Buf+2+i*2,&(uiconf.UISensorConfigList.at(i)),0x02);
        lenth += 2;
    }
    Buf[0] = lenth&0xFF;
    Buf[1] = (lenth&0xFF00)>>8;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORT_HOMEDISPLAY_PARAM_CHANGGED,\
                                V4APP_DEV_TYPE_SC,pParamMng->GetRunValueGlobalCurrentSupportID(),\
                                Buf,lenth+2);
    return 0;
}
//上报成组配置菜单参数修改
int HysAppMsgSendGrpKeyConfParamChanedProc(void)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    globalParam_GroupAssistKeyType grpkeyconf;
    pParamMng->GetRunValueGlobalGroupAssistKeyParam(grpkeyconf);//成组按键配置中的具体参数

    /*****/
    u_int8_t Buf[1024];
    u_int16_t lenth = 0;
    memset(Buf,0x00,sizeof (Buf));

    for(int i = 0; i < grpkeyconf.GroupAssistKeyConfigList.size(); i ++)
    {
        memmove(Buf+2+i*2,&(grpkeyconf.GroupAssistKeyConfigList.at(i)),0x02);
        lenth += 2;
    }
    Buf[0] = lenth&0xFF;
    Buf[1] = (lenth&0xFF00)>>8;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORT_GRPKEYCONF_PARAM_CHANGGED,\
                                V4APP_DEV_TYPE_SC,pParamMng->GetRunValueGlobalCurrentSupportID(),\
                                Buf,lenth+2);
    return 0;
}
//上报单动作中，动作参数的修改
int HysAppMsgSendSingleActParamChanedProc(void)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    QList<SingleParam_InCommonUse> commonlist;
    pParamMng->GetRunValueSingleInCommonUse(commonlist);

    /*****/
    u_int8_t Buf[1024];
    u_int16_t lenth = 0;
    memset(Buf,0x00,sizeof (Buf));

    for(int i = 0; i < commonlist.size(); i ++)
    {
        memmove(Buf+2+i*12,&(commonlist.at(i).ID),0x02);
        memmove(Buf+2+i*12+2,&(commonlist.at(i).enable),0x02);
        memmove(Buf+2+i*12+4,&(commonlist.at(i).controlMode),0x02);
        memmove(Buf+2+i*12+6,&(commonlist.at(i).localControlMode),0x02);
        memmove(Buf+2+i*12+8,&(commonlist.at(i).mutexActionID1),0x02);
        memmove(Buf+2+i*12+10,&(commonlist.at(i).mutexActionID2),0x02);
        lenth += 12;
    }
    Buf[0] = lenth&0xFF;
    Buf[1] = (lenth&0xFF00)>>8;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORT_SINGLE_ACTPARAM_CHANGGED,\
                                V4APP_DEV_TYPE_SC,pParamMng->GetRunValueGlobalCurrentSupportID(),\
                                Buf,lenth+2);
    return 0;
}
//上报组合动作中，动作参数的修改
int HysAppMsgSendCommbActParamChanedProc(void)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    QList<CommbParam_InCommonUse> commonlist;
    pParamMng->GetRunValueCommbInCommonUse(commonlist);

    /*****/
    u_int8_t Buf[1024];
    u_int16_t lenth = 0;
    memset(Buf,0x00,sizeof (Buf));

    for(int i = 0; i < commonlist.size(); i ++)
    {
        memmove(Buf+2+i*10,&(commonlist.at(i).ID),0x02);
        memmove(Buf+2+i*10+2,&(commonlist.at(i).enable),0x02);
        memmove(Buf+2+i*10+4,&(commonlist.at(i).controlMode),0x02);
        memmove(Buf+2+i*10+6,&(commonlist.at(i).localControlMode),0x02);
        memmove(Buf+2+i*10+8,&(commonlist.at(i).actionPriority),0x02);
        lenth += 10;
    }
    Buf[0] = lenth&0xFF;
    Buf[1] = (lenth&0xFF00)>>8;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORT_COMMB_ACTPARAM_CHANGGED,\
                                V4APP_DEV_TYPE_SC,pParamMng->GetRunValueGlobalCurrentSupportID(),\
                                Buf,lenth+2);
    return 0;
}
//上报成组动作中，动作参数的修改
int HysAppMsgSendGrpActParamChanedProc(void)
{
    /*****/
    LibHysA7ParamMng *pParamMng = LibHysA7ParamMng::getInston();
    QList<GrpParam_InCommonUse> commonlist;
    pParamMng->GetRunValueGrpInCommonUse(commonlist);

    /*****/
    u_int8_t Buf[1024];
    u_int16_t lenth = 0;
    memset(Buf,0x00,sizeof (Buf));

    for(int i = 0; i < commonlist.size(); i ++)
    {
        memmove(Buf+2+i*10,&(commonlist.at(i).ID),0x02);
        memmove(Buf+2+i*10+2,&(commonlist.at(i).localControlMode),0x02);
        memmove(Buf+2+i*10+4,&(commonlist.at(i).safeGap),0x02);
        memmove(Buf+2+i*10+6,&(commonlist.at(i).scope),0x02);
        memmove(Buf+2+i*10+8,&(commonlist.at(i).groupDirct),0x02);
        lenth += 10;
    }
    Buf[0] = lenth&0xFF;
    Buf[1] = (lenth&0xFF00)>>8;

    HysAppInteraction * pAppIter = HysAppInteraction::getInston();
    pAppIter->InsertSendMsgProc(MSG_REPORT_GRP_ACTPARAM_CHANGGED,\
                                V4APP_DEV_TYPE_SC,pParamMng->GetRunValueGlobalCurrentSupportID(),\
                                Buf,lenth+2);
    return 0;
}
