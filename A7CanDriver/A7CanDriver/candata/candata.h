#ifndef __CANDATA_H__
#define __CANDATA_H__

#include "can_protocol.h"
#include "pro_data.h"
#include "bitset"
#include "dyk_v2pro.h"
#include "zprint.h"


using namespace std;

#define CON_OUT_BUF_SIZE 256
#define CON_IN_BUF_SIZE 1024



#define CAN_BITRATE_200K 200000


#define DRIVER_ID_CAN  31


class Can_Data
{

public:
    CanDriver          * canbus;
    CanDriver          * canbus1;
    ncan_protocol      * canpro;
    dyk_can            * dykv2;

public:
    Can_Data(){
        canbus = NULL;
        canbus1 = NULL;
        canpro = NULL;
        dykv2 = NULL;
    }

    ~Can_Data(){

        if(dykv2 != NULL){
             zprintf1("desotry dykv2!\n");
             delete dykv2;
            zprintf1("destory dykv2 over!\n");
            dykv2 = NULL;
        }
        if(canpro != NULL){
            delete canpro;
            canpro = NULL;
            zprintf1("delete canpro!\n");
        }
        if(canbus != NULL){
            delete canbus;
            canbus = NULL;
            zprintf1("delete canbus!\n");
        }
        if(canbus1 != NULL)
        {
            delete canbus1;
            canbus1 = NULL;
            zprintf1("delete canbus1!\n");
        }
    }

    void creat_can_bus_pro(void);
    int can_app_init(void);
};



#endif /*__CANDATA_H__*/
