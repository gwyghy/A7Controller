#ifndef CAN_BUS_H
#define CAN_BUS_H

#include "can_relate.h"

#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "e_poll.h"
#include <sys/eventfd.h>
#include <bitset>
#include "timers.h"
#include "pro_data.h"
#include "zprint.h"


using namespace std;

#define CAN_DATA_LEN  8
#define CANBUS_TX_QSIZE 64
#define CANBUS_RX_QSIZE 64

#define CAN_MAX_NUM 1

#ifndef CAN_MAX_DLEN
#define CAN_MAX_DLEN 8
#endif


#define MESSAGE_LEN 8

#define  CANMNG(x) Can_Manage * (x) = Can_Manage::get_can_manage();


typedef struct
{
    unsigned int StdId;		            //标准帧ID
    unsigned int ExtId;		            //扩展帧ID
    unsigned char IDE;		            //帧类型 0：标准帧 1：扩展帧
    unsigned char RTR;		            //数据类型 0：数据帧  1：远程帧
    unsigned char DLC;		            //数据长度
    unsigned char Data[CAN_DATA_LEN];	//数据组

}CANDATAFORM;


enum
{
    CAN_Id_Standard = 0x00,
    CAN_Id_Extended
};

typedef struct
{
    unsigned int Baudrate;
}CANDEVPARA;


#define READBUF  Pth_Buf_T<CanFrame, CANBUS_TX_QSIZE, CanDriver>
#define WRITEBUF  Pth_Buf_T<CANDATAFORM, CANBUS_TX_QSIZE, CanDriver>

class CanDriver:public NCbk_Poll
{
public:
    int      canIndex;
    int      interval;
    int      CanFileP;
    void *   father_p;
    READBUF  canread;
    WRITEBUF canwrite;
public:
    CanDriver(int fnum = 1):NCbk_Poll(fnum){
        father_p = NULL;
        interval = 5;
        canIndex = 0;
    }

    int can_bus_init(int registerdev ,int brate = 100000);
    void run();
    int write_send_data(CANDATAFORM  & Msg);
    int writeframe(const CANDATAFORM& f);
    int writeframe(const CanFrame& f);

};

CANDATAFORM lawdata_to_prodata(const CanFrame & f);
#endif // CAN_BUS_H
