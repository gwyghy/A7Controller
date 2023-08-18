
#include <stddef.h>
#include "can_protocol.h"
#include "can_bus.h"
#include "zprint.h"
#include <qdebug.h>
#include "clist.h"
#include "can_device_app.h"

using namespace std;


/*******************************************************************************************
*功能描述：V4设备类型转成V2的设备类型
*入口参数：device取值：
*             设备类型
*返回值：  无
*******************************************************************************************/
uint8_t deviceToV2type(uint8_t device)
{
    uint8_t res;
    switch(device)
    {
        case ID_EMVD:
            res = V2ID_EMVD;
            break;
        case ID_ANGLE:
            res = V2ID_ANGLE;
            break;
        case ID_HIGH:
            res = V2ID_ANGLE;
            break;
        case ID_ALARM:
        case ID_ALARMANGLE:
            res = V2ID_ALARM;
            break;
        case ID_PERSON:
            res = V2ID_PERSON;
            break;
        case ID_CXB:
            res = V2ID_CXB;
            break;
        case ID_IRRX:
            res = V2ID_WL;
            break;
        case ID_MINIHUB1:
            res = V2ID_HUB;
            break;
    }
    return res;
}


/***********************************************************************************
 * 函数名：write_deldata_buf
 * 功能：添加协议帧的信息包括处理函数等 待添加错误管理
 *
 ***********************************************************************************/
int ncan_protocol::add_protocol_frame(unsigned int id, CANPROHEAD info)
{
    canidmap.insert(pair<FrameId, CANPROHEAD>(id, info));
    return 1;
}


int ncan_protocol::init_pro_frame(CANPROHEAD * info, int size)
{
    if(info != NULL)
    {
        for(int i = 0; i < size; i++)
            add_protocol_frame(((info[i].frametype<<16)|info[i].frameid), info[i]);
        return 0;
    }
    return -1;
}




int ncan_protocol::get_protocol_frameinfo(unsigned int id, unsigned int type, CANPROHEAD & info)
{

    V2_CAN_ID v2id;
    memcpy(&v2id, &id, sizeof(V2_CAN_ID));

//    if(v2id.RxID != 0)
//    {
//        return -1;
//    }
    map<FrameId, CANPROHEAD>::iterator proiter;

    uint32_t deviceType = 0;
    if(type == 1)
        deviceType = ((v2id.TxID<<16)|v2id.FrameType);
    else if(type == 0)
        deviceType = ((v2id.RxID<<16)|v2id.FrameType);
    else if(type == 2)//程序更新
        deviceType = (((v2id.RxID|V2ID_CXB)<<16)|v2id.TxID);


    proiter = canidmap.find(deviceType);

    if(proiter != canidmap.end())
    {
        info = proiter->second;

        return 0;
    }
//    printf("get pro fram 0x%x type %dfail\n", id, type);
    return -1;
}

CANPROHEAD * ncan_protocol::get_protocol_frameinfo(unsigned int id, unsigned int type)
{
    map<FrameId, CANPROHEAD>::iterator proiter;

    proiter = canidmap.find(id);

    if(proiter != canidmap.end())
    {
        return &(proiter->second);
    }
    return NULL;
}

/***********************************************************************************
 * 函数名：canpro_timeover_del
 * 功能：can协议对超时帧进行处理
 *
 ***********************************************************************************/
int canpro_time_callback(CANp_TIME_ET * tmpara)
{
    CAN_DATA_LIST  * midpoint = NULL;
    CAN_DATA_LIST  * copymid = NULL;
    CAN_DATA_LIST  * retval = NULL;

    pthread_mutex_lock(&tmpara->father->prolist.list_mut);

//    qDebug("time mute enter is %d !",list_mut.__align);
    midpoint = tmpara->father->prolist.rw_P;
    while(midpoint != NULL)
    {
//        zprintf3("fram is 0x%x overtime is %d run is %d\n", midpoint->p->canhead.frameid, midpoint->p->runtime, midpoint->p->canhead.overtime);
        midpoint->p->runtime += 10;
        if(midpoint->p->runtime  >= midpoint->p->canhead.overtime)
        {
            if(midpoint->p->sendnum >= midpoint->p->canhead.repeatnum)
            {
               copymid = midpoint;
               midpoint = midpoint->next;

              if(retval == NULL)
              {
                  tmpara->father->prolist.rw_P = copymid->next;
                  copymid->next = tmpara->father->prolist.free_P;
                  tmpara->father->prolist.free_P = copymid;
              }
              else
              {
                  retval->next = copymid->next;
                  copymid->next = tmpara->father->prolist.free_P;
                  tmpara->father->prolist.free_P = copymid;
              }
            }
            else
            {
                midpoint->p->runtime = 0;
                if(midpoint->p->canhead.repeatnum != 0xFF)
                    midpoint->p->sendnum++;
               if(midpoint->p->driver == LOGIC_CAN1)
                   tmpara->father->candrip->write_send_data(midpoint->p->candata);
                else if(midpoint->p->driver == LOGIC_CAN2)
                   tmpara->father->candrip1->write_send_data(midpoint->p->candata);
            }
        }
        else
        {
             retval = midpoint;
             midpoint = midpoint->next;
        }
    }

    pthread_mutex_unlock(&tmpara->father->prolist.list_mut);
    return 0;
}





/***********************************************************************************
 * 函数名：pro_rxmsg_callback
 * 功能：can协议接收处理回调函数
 *
 ***********************************************************************************/
int pro_rxmsg_callback(CanDriver * pro, CanFrame data)
{
    unsigned int frameid;
    int err = -1;
    CANPROHEAD   rxheadinfo;

    CANDATAFORM rxmeg = lawdata_to_prodata(data);
    if(rxmeg.IDE ==0)
        return err;
    if(rxmeg.DLC == 0)
        return err;

    ncan_protocol * midpro = (ncan_protocol*)pro->father_p;

    frameid = rxmeg.ExtId ;

    if((frameid&V4CANPROFRAME) != 0) //V4外围设备
    {
    }
    else  //V2外围设备
    {
        //V2外围设备更新程序
        if((frameid&V2CANPROFRAME_UPDATE) != 0) //V2更新程序
        {
            if(midpro->get_protocol_frameinfo(frameid, 2,rxheadinfo) == 0)
            {
                if(rxheadinfo.cancallbacfun != NULL)
                {
                    rxheadinfo.cancallbacfun(pro, rxmeg);
                    err = 0;
                }
            }
        }
        //V2其他数据
        else
        {
            if(midpro->get_protocol_frameinfo(frameid, 1,rxheadinfo) == 0)
            {
                if(rxheadinfo.cancallbacfun != NULL)
                {
                    rxheadinfo.cancallbacfun(pro, rxmeg);
                    err = 0;
                }
            }
            else
            {
                 err = -2;
            }
        }
    }
    return err;
}


int ncan_protocol::ncan_pro_init(CanDriver * dri,CanDriver * dri1)
{
    protm.add_event(0.01,canpro_time_callback,this);

    candrip = dri;
    dri->father_p = this;
    dri->canread.z_pthread_init(pro_rxmsg_callback, dri);

    candrip1 = dri1;
    dri1->father_p = this;
    dri1->canread.z_pthread_init(pro_rxmsg_callback, dri1);

    return 0;

}

/***********************************************************************************
 * 函数名：can_protocol_send
 * 功能：通过can协议发送can帧 参数为can数据帧结构
 *
 *
 ***********************************************************************************/
int ncan_protocol::can_protocol_send(uint8_t driver,CANDATAFORM  canprop)
{
    CANPRODATA   middata;
    CANPROHEAD   midprohead;
    int          reghead;

    reghead = get_protocol_frameinfo(canprop.ExtId, 0, midprohead);
    if(reghead == -1)
    {
        return -1;
    }
    memset(&middata, 0, sizeof(CANPRODATA));
    if(reghead == 0)                                                 //已经注册了该帧的协议头
    {
        middata.candata = canprop;
        middata.canhead = midprohead;
        middata.driver = driver;
        if(midprohead.sendprocessfunc!=NULL)
        {
            midprohead.sendprocessfunc(this, &middata.canhead,&middata.candata);
            canprop.DLC = middata.candata.DLC;
        }

        if(middata.sendnum < middata.canhead.repeatnum)
        {
            if(middata.canhead.repeatnum != 0xFF)
                middata.sendnum++;

            if(middata.canhead.overtime != -1)
            {
                prolist.buf_write_data(middata);
            }
        }
    }
//    zprintf3("*****fileType:%d*********************\n",middata.driver);
    if(middata.driver == LOGIC_CAN1)
       candrip->write_send_data(canprop);
    else if(middata.driver == LOGIC_CAN2)
       candrip1->write_send_data(canprop);

    return 0;
}


/***********************************************************************************
 * 函数名：can_protocol_send
 * 功能：通过can协议发送can帧 参数为can数据帧结构
 *
 *
 ***********************************************************************************/
int ncan_protocol::can_protocol_send_cxb(uint8_t driver,CANDATAFORM  canprop)
{
    CANPRODATA   middata;
    CANPROHEAD   midprohead;
    int          reghead;


    midprohead.ackframe = 0;
    midprohead.cancallbacfun = NULL;
    midprohead.frameid = 0;
    midprohead.frametype = 0;
    midprohead.overcallbacfun = NULL;
    midprohead.overtime = 10;
    midprohead.repeatnum = 2;
    midprohead.sendiffunc = NULL;
    midprohead.sendprocessfunc = NULL;
    reghead = 0;

    memset(&middata, 0, sizeof(CANPRODATA));
    if(reghead == 0)                                                 //已经注册了该帧的协议头
    {
        middata.candata = canprop;
        middata.canhead = midprohead;
        middata.driver = driver;
        if(midprohead.sendprocessfunc!=NULL)
        {
            midprohead.sendprocessfunc(this, &middata.canhead,&middata.candata);
            canprop.DLC = middata.candata.DLC;
        }

        if(middata.sendnum < middata.canhead.repeatnum)
        {
            if(middata.canhead.repeatnum != 0xFF)
                middata.sendnum++;

            if(middata.canhead.overtime != -1)
            {
                prolist.buf_write_data(middata);
            }
        }
    }
//    zprintf3("*****fileType:%d*********************\n",middata.driver);
    if(middata.driver == LOGIC_CAN1)
       candrip->write_send_data(canprop);
    else if(middata.driver == LOGIC_CAN2)
       candrip1->write_send_data(canprop);

    return 0;
}


int frame_full_func(CANPRODATA & f, CANPRODATA & c)
{
   if((f.candata.ExtId==c.candata.ExtId)
     &&(f.candata.DLC=c.candata.DLC))
   {
       return 0;
   }
   return -1;
}

int ncan_protocol::pro_del_buf_frame(CANDATAFORM & frame)
{
    int err;
    CANPRODATA cmp;
    memcpy(&cmp.candata, &frame, sizeof(CANDATAFORM));

    err = prolist.condition_delete_list_data(frame_full_func,cmp);
//    printf("return err %d\n",err);
    return err;

}



void ncan_protocol::start(void)
{
    protm.start();
}




