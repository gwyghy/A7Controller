#include "devemvd.h"
#include "v4hysapptype.h"
#include "HysSystem/hyssystem.h"
#include "HysAppInteraction/hysappinteraction.h"
#include <QList>

/******/
static QMap<ushort,HysDevFrameProcFuncType> DevFrameProcMap;

/******/
QList<u_int16_t> emvd_act_list;//电磁阀驱动器正在动作的动作列表,用于与EMVD间动作心跳的维持
QMutex mMutex;

/*****/
static int DevFrameEmvdGetSingleResponseProc(uchar driverid, uchar devid, uchar childdevid, ushort point, u_int8_t *data, int len)
{
    if(len == 0x00)
        return -1;

    HysScExtDevType extdevstate;
    u_int8_t buf[6] = {0};
    u_int16_t SelfScid = LibHysA7ParamMng::getInston()->GetRunValueGlobalCurrentSupportID();
    u_int16_t singleactid = 0;
    QList<SingleActionParamType> singlevaluelist;

    HysSystem::getInston()->GetExtDevState(2,extdevstate);
    LibHysA7ParamMng::getInston()->GetRunValueSingleActParam(singlevaluelist);

    for(int i = 0 ;i < singlevaluelist.size();i++)
    {
        if(point == singlevaluelist.at(i).baseParam.pointID.point)
        {
            singleactid = singlevaluelist.at(i).baseParam.ID;
            break;
        }
    }

    buf[0] = static_cast<u_int8_t>(singleactid);
    buf[1] = 0;

    if(extdevstate.Status == OnlineType_ONLINE)
    {
        buf[2] = SENSOR_FAULT_STATUS_NORMAL;
        buf[3] = 0;
        buf[4] = data[0];
        buf[5] = data[1];

        HysAppInteraction * pAppIter = HysAppInteraction::getInston();
        pAppIter->InsertSendMsgProc(MSG_CHECK_EMVD_POINT_STATUS_RESPONSE,\
                                    V4APP_DEV_TYPE_SC,SelfScid,\
                                    buf,6);
        qDebug()<<"*******DevFrameEmvdGetSingleResponseProc,emvd check status******"<<MSG_CHECK_EMVD_POINT_STATUS_RESPONSE<<SelfScid\
               <<buf[0]<<buf[1]<<buf[2]<<buf[3]<<buf[4]<<buf[5];
    }
    else
    {
        buf[2] = SENSOR_FAULT_STATUS_FAULT;
        buf[3] = 0;
        buf[4] = EmvdPointStatusType_TimeOut;
        buf[5] = 0;

        HysAppInteraction * pAppIter = HysAppInteraction::getInston();
        pAppIter->InsertSendMsgProc(MSG_CHECK_EMVD_POINT_STATUS_RESPONSE,\
                                    V4APP_DEV_TYPE_SC,SelfScid,\
                                    buf,6);
    }
    return 0;
}

/******/
int DevEmvdInit(void)
{
    emvd_act_list.clear();
    DevFrameProcMap.insert(V4DRIVER_STATUS_GET_SINGLE_RESPONSE,DevFrameEmvdGetSingleResponseProc);
    DevFrameProcMap.insert(V4DRIVER_REPORT_EXTDEV_STATUS,DevFrameDriverExtdevStatusReportProc);
    return 0;
}

/******/
int DevEmvdFrameProc(uchar driverid, uchar devid, uchar childdevid, ushort point, ushort frametype, u_int8_t *data, int len)
{
    if(DevFrameProcMap.contains(frametype))
        return DevFrameProcMap[frametype](driverid,devid,childdevid,point,data,len);
    return -1;
}

/*****/
int DevEmvdInsertRealTimeActList(u_int16_t pointid)
{
    if(pointid == 0)
        return -1;
    if(emvd_act_list.contains(pointid) == true)
        return -1;
    if(pointid > DEV_POINT_32)
        return -1;
    if(emvd_act_list.size() > DEV_POINT_32)
        return -1;

    mMutex.lock();
    emvd_act_list.append(pointid);
    mMutex.unlock();

    return 0;
}

/******/
int DevEmvdDeleteRealTimeActList(u_int16_t pointid)
{
    if(pointid == 0)
        return -1;
    if(emvd_act_list.contains(pointid) == false)
        return -1;

    mMutex.lock();
    emvd_act_list.removeAll(pointid);
    mMutex.unlock();
    return 0;
}

int DevEmvdClearRealTimeActList(void)
{
    mMutex.lock();
    emvd_act_list.clear();
    mMutex.unlock();
    return 0;
}

int DevEmvdGetRealTimeActList(QList<u_int16_t> &list)
{
    if(emvd_act_list.size() == 0)
        return -1;

    mMutex.lock();
    list = emvd_act_list;
    mMutex.unlock();

    return 0;
}
