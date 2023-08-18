#include "hysappinteraction.h"
#include "./HysDeviceMng/devglobaltype.h"
#include "sys/time.h"
#include "hysappmsg.h"
#include <QDateTime>

/*****/
#define APP_MSG_KEY_NUMBER  0x07531511
#define APP_MSG_SEND_TYPE   1
#define APP_MSG_RECV_TYPE   2
#define APP_MSG_RECV_DT_TIMEOUT 100

/****/
HysAppInteraction *HysAppInteraction::that = NULL;

/****************/
HysAppInteraction::HysAppInteraction()
{
    m_AppMsg = new AppMsg(APP_MSG_KEY_NUMBER,APP_MSG_SEND_TYPE,APP_MSG_RECV_TYPE);
    m_AppMsg->create_object();
}

HysAppInteraction::~HysAppInteraction()
{
    delete m_AppMsg;
}

HysAppInteraction *HysAppInteraction::getInston()
{
    if( that == NULL)
    {
        that = new HysAppInteraction();
    }
    return that;
}

void HysAppInteraction::Init(void)
{
    msgproctab.clear();
    /***数据采集控制相关，1-100**/
    msgproctab.insert(MSG_APP_CTRL_DEV,                     HysAppMsgRxCtrlDevProc);
    msgproctab.insert(MSG_MANUAL_ACT_HEARTBEAT,             HysAppMsgRxManualHeatBeatProc);
    msgproctab.insert(MSG_APP_FOLLOWON_INQUIRE,             HysAppMsgRxFollowOnInquireProc);
    msgproctab.insert(MSG_APP_AUTOBACKFLUSH_ONOFF_INQUIRE,  HysAppMsgRxAutoBackFlushOnInquireProc);
    msgproctab.insert(MSG_APP_KEY_LED_STATUS_CTRL,          HysAppMsgRxKeyLedStatusCtrlProc);
    msgproctab.insert(MSG_APP_STOP_KEY_STATUS,              HysAppMsgRxEndKeyStatusProc);

    /**************参数传输相关，101-200***************/
    msgproctab.insert(MSG_DISPLAY_DEV_PARAM_INQUIRE,        HysAppMsgRxDisplayRunParamInquireProc);
    msgproctab.insert(MSG_REPORTSS_PARAM_CHANGGED,          HysAppMsgRxReportRunParamChangedProc);
    msgproctab.insert(MSG_DISPLAY_SYSTEM_PARAM_INQUIRE,     HysAppMsgRxDisplaySystemParamInquireProc);
    msgproctab.insert(MSG_REPORTSS_SYSTEM_CHANGGED,         HysAppMsgRxReportSystemParamChangedProc);

    /*************参数修改及文件传输相关，201-300.以下为自定义*****************/
    msgproctab.insert(MSG_SERVICE_OPERATE_INF,              HysAppMsgRxServiceOperateInfProc);
    msgproctab.insert(MSG_DOWNLOAD_DEV_PRG_CXB,             HysAppMsgRxDownloadDevPrgCxbProc);
    msgproctab.insert(MSG_DOWNLOAD_DEV_PRG_REMOTE,          HysAppMsgRxDownloadDevPrgRemoteProc);
    msgproctab.insert(MSG_UPDATE_DEV_PRG,                   HysAppMsgRxUpdateDevPrgProc);

    /*************网络维护相关，301~.以下为自定义*****************/
    msgproctab.insert(MSG_CHECK_COMM_AREA_INQUIRE,          HysAppMsgCheckCommAreaInquireProc);
    msgproctab.insert(MSG_CHECK_DATA_TRANS_INF_INQUIRE,     HysAppMsgCheckDataTransInfInquireProc);
    msgproctab.insert(MSG_CHECK_EMVD_POINT_STATUS_INQUIRE,  HysAppMsgCheckEmvdPointStatusInquireProc);

}

/***立即发送**/
int HysAppInteraction::InsertSendMsgProc(u_int16_t  frametype,u_int16_t  devtype, u_int16_t devid, uint8_t *pbuf,u_int16_t length)
{
    if(length > APP_MSG_ENTITY_LENGTH)
        return -1;
    if(frametype > MSG_TYPE_MAX)
        return -1;

    sMsgUnit sendmsg;
    sendmsg.source.app = APP_MSG_SEND_TYPE;
    sendmsg.dest.app = APP_MSG_RECV_TYPE;//应用的编号与设备类型之间差一，需特殊处理
    sendmsg.type = frametype;
    QDateTime nowTime = QDateTime::currentDateTime();
    qint64 dt = nowTime.toMSecsSinceEpoch();
    memcpy(sendmsg.data,&dt,sizeof(quint64));
    memmove(sendmsg.data+8,&devtype,0x02);
    memmove(sendmsg.data+10,&devid,0x02);
    memmove(sendmsg.data+12,pbuf,length);
    m_AppMsg->SendMsg(&sendmsg,length+12);

    return 0;
}

void HysAppInteraction::ReceiveMsgProc(void)
{
    sMsgUnit recvMsg;
    u_int16_t length = 0;
    bool returnflag = false;
    MsgEntity msgentitydata;
    quint64 recvdt;
    quint64 nowdt;
    while(1)
    {
        returnflag = m_AppMsg->ReceiveMsg(&recvMsg,&length,RECV_WAIT);
        if((returnflag == true)&&(recvMsg.dest.app == LOGIC_PROGRAM_ID))
        {
            QDateTime nowTime = QDateTime::currentDateTime();
            qint64 nowdt = nowTime.toMSecsSinceEpoch();
            memcpy(&recvdt,recvMsg.data,sizeof(quint64));
            if((nowdt-recvdt) > APP_MSG_RECV_DT_TIMEOUT)
            {
                qDebug()<<"ReceiveMsgProc,timw out"<<recvMsg.dest.app;
                continue;
            }
            memmove(&msgentitydata.dt,recvMsg.data,(length>APP_MSG_ENTITY_LENGTH)?APP_MSG_ENTITY_LENGTH:length);
            if(msgproctab.contains(recvMsg.type))
                msgproctab[recvMsg.type](msgentitydata.devtype, msgentitydata.devid, msgentitydata.data, length-12);
        }
        else
        {
            qDebug()<<"********************ReceiveMsgProc,wrong*************";
        }
    }
}
