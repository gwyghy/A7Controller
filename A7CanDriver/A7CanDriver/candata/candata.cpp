#include "stdio.h"
#include "candata.h"
#include "can_device_app.h"
#include "zprint.h"
#include "ydshrdlib.h"


void* deviceInfoThreadFun(void* arg) {

    DeviceInfo& deviceInfo = DeviceInfo::getInstance();
    int timer = 100;
    for(;;)
    {
        deviceInfo.queryDeviceStateProc_emvd(timer);
        deviceInfo.queryDeviceStateProc_wireless(timer);
        deviceInfo.queryDeviceStateProc_angle(timer);
        deviceInfo.queryDeviceStateProc_alarm(timer);
        deviceInfo.queryDeviceStateProc_person(timer);
        deviceInfo.queryDeviceStateProc_high(timer);
        deviceInfo.queryDeviceStateProc_alarmAngle(timer);
        deviceInfo.queryDeviceStateProc_highAngle(timer);
        deviceInfo.queryDeviceStateProc_ir(timer);
        deviceInfo.queryDeviceStateProc_MiniHub1(timer);
        deviceInfo.queryDeviceStateProc_MiniHub2(timer);
        deviceInfo.queryDeviceStateProc_MiniHub3(timer);
        usleep(timer*1000);
    }
}


void Can_Data::creat_can_bus_pro(void)
{
    canbus = new CanDriver(LOGIC_CAN1);
    canbus1 = new CanDriver(LOGIC_CAN2);
    canpro = new ncan_protocol();
    dykv2 = new dyk_can(canpro);
}

int Can_Data::can_app_init(void)
{
    canbus->can_bus_init(LOGIC_CAN1, CAN_BITRATE_200K);
    canbus1->can_bus_init(LOGIC_CAN2, CAN_BITRATE_200K);
    canpro->ncan_pro_init(canbus,canbus1);

    canpro->start();
    canbus->start();
    canbus1->start();

//    if(dykv2 != NULL)emvdMsgInfo.state
    {
        dykv2->dyk_driver_config_init();
    }

    int res_canapp =  Ydshrdlib::getInston()->InitLibParam(DRIVER_ID_CAN,87654333,87654332);

    zprintf3("A7Can_init res:%d \n ",res_canapp);

    Ydshrdlib::getInston()->setShmCallBack(1,57,RecvShmCallback_Controll);
    Ydshrdlib::getInston()->setShmCallBack(1,51,RecvShmCallback_ValueCheck);
    Ydshrdlib::getInston()->setShmCallBack(1,59,RecvShmCallback_ControllHeart);
//    Ydshrdlib::getInston()->setShmCallBack(1,61,RecvShmCallback_ReadDeviceState);
//    Ydshrdlib::getInston()->setShmCallBack(1,63,RecvShmCallback_ReadAllDeviceState);

    Ydshrdlib::getInston()->setMsgCallBack(1,8,RecvMsgCallback_InitDevice);
//    Ydshrdlib::getInston()->setMsgCallBack(1,6,RecvMsgCallback_CXBLoadProgram);
    Ydshrdlib::getInston()->setMsgCallBack(1,2,RecvMsgCallback_UpdateProgram);

    pthread_t deviceInfoThread;
    pthread_create(&deviceInfoThread, NULL, deviceInfoThreadFun, NULL);


#if 0  //update program test
    char pwddd[30] = "/opt/bin/sgV103.bin";
    uint8_t testttt[60] = {0};

    testttt[0] = 3;
    testttt[2] = ID_ALARM;
    memcpy(&testttt[6],pwddd,30);
    RecvMsgCallback_UpdateProgram(ID_ALARM,1,1,testttt,36);
#endif
#if 0
    sleep(5);
    RecvShmCallback_ValueCheck(0x41,1,5,0,0);
#endif
    return 0;
}


