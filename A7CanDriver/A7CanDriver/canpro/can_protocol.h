#ifndef CAN_PROTOCOL_H
#define CAN_PROTOCOL_H
#include <map>
#include <vector>
#include <algorithm>
#include "can_bus.h"
#include "timers.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "clist.h"

#define V4CANPROFRAME          0x10000000
#define V2CANPROFRAME_UPDATE   0x08000000
#define V2CANPROACK            0x04000000


#define LOGIC_CAN1  1
#define LOGIC_CAN2  2


using namespace std;
typedef unsigned int FrameId;

typedef void* (*Pro_Init_Process)(void * para);
typedef FrameId (*Get_Frame_Mapid)(FrameId paraid);


typedef struct
{
    unsigned int    frametype;                         //帧类型
    unsigned int    frameid;                           //映射号
    int             overtime;
    int             repeatnum;
    unsigned int    ackframe;
    void         *  father;
    int (*cancallbacfun)(void * para, CANDATAFORM data);      //接收回调函数
    int (*overcallbacfun)(void *para,  CANDATAFORM data);     //超时回调函数
    int (*sendiffunc)(void *);                                //发送条件回调函数
    int (*sendprocessfunc)(void *pro ,void *para,  CANDATAFORM* data);    //发送前处理回调函数

}CANPROHEAD;


/**V2外围设备can帧格式***/
typedef struct
{
    unsigned int	RxID:3;				//接收方ID
    unsigned int	TxID:3;				//发送方ID
    unsigned int	FrameType:10;		//帧类别
    unsigned int	LiuShuiNumb:4;		//流水序列号，0x0-0xf循环，ACK=1应答帧的流水，采用被应答帧的流水
    unsigned int	Sum:5;		//子帧序号或
    unsigned int	Sub:1;		//组合帧/子帧，0：组合帧总数帧：1：子帧
    unsigned int	ACK:1;				//应答标志，1:应答，0:无需应答
    unsigned int	Reservd:2;			//预留区域。用于传程序:01,其他:00
    unsigned int	NoUsed:3;			//不存在区域
} V2_CAN_ID;


typedef struct
{
    unsigned int	RxID:3;				//接收方ID
    unsigned int	FrameType:3;		//帧类别
    unsigned int	PacketNum:20;		//包id
    unsigned int	ACK:1;				//应答标志，1:应答，0:无需应答
    unsigned int	Reservd:2;			//预留区域。用于传程序:01,其他:00
    unsigned int	NoUsed:3;			//不存在区域
} V2_CXBCAN_ID;//帧ID



typedef struct
{
    CANPROHEAD          canhead;
    CANDATAFORM         candata;
}CANPROFRAME;

typedef struct can_pro_data
{
    CANPROHEAD          canhead;
    CANDATAFORM         candata;
    int                 sendnum;
    int                 runtime;
    int                 driver;
}CANPRODATA;


class ncan_protocol;

#define CANp_TIMER          B_Timer(ncan_protocol)
#define CANp_TIME_ET        TimerEvent<ncan_protocol,void>
#define CAN_DATA_LIST       List_N<CANPRODATA>
#define CANp_List           C_LIST_T<CANPRODATA,CANBUS_TX_QSIZE>


typedef map<FrameId, CANPROHEAD> FrameMap;

class ncan_protocol
{
public:
    FrameMap                        canidmap;
    CanDriver     *                 candrip;     //canDeiver指针
    CanDriver     *                 candrip1;    //canDeiver指针
    CANp_TIMER                      protm;       //can协议定时器
    CANp_List                       prolist;

public:
    ncan_protocol(){
    }

    ~ncan_protocol(){
        zprintf3("destory ncan_protocol!\n");
    }
    int ncan_pro_init(CanDriver * dri,CanDriver * dri1);
    int add_protocol_frame(unsigned int id, CANPROHEAD info);
    int init_pro_frame(CANPROHEAD * info, int size);
    int get_protocol_frameinfo(unsigned int id, unsigned int type, CANPROHEAD & info);
    CANPROHEAD * get_protocol_frameinfo(unsigned int id, unsigned int type);

    int can_protocol_send(uint8_t driver,CANDATAFORM  canprop);
    int can_protocol_send_cxb(uint8_t driver,CANDATAFORM  canprop);
    int pro_del_buf_frame(CANDATAFORM & frame);

    void start(void);

};


uint8_t deviceToV2type(uint8_t device);

#endif // CAN_PROTOCOL_H
