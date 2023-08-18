#ifndef __DYK_V2PRO_C__
#define __DYK_V2PRO_C__

#include "dyk_v2pro.h"
#include "timers.h"
#include "zprint.h"
#include "can_device_app.h"
#include "ydshrdlib.h"
#include "fileload.h"
#include "file_handler.h"

extern STR_CAN_RECV_PROGRAM can_recv_program;

ncan_protocol  *    apppro_p;
/***********************************************************************************
 * 函数名：dyk_driver_config_init
 * 功能：cs配置初始化函数
 ***********************************************************************************/
int dyk_can::dyk_driver_config_init(void)
{
    CANPROHEAD  csmidinfo[] ={
/*****************1)本地参数修改\状态上报******************************************/
        //---------------------dcf心跳--imd=0x00------------------------
        {
            1,
            0x00,                               //帧id
            0,                                  //超时时间
            0,                                  //重发次数
            0x00,                               //应答帧
            this,
            Emvd_Heart_Proc,                   //接受回调函数
            NULL,                              //超时回调函数
            NULL,                              //发送条件回调函数
            NULL                               //发送处理回调函数
        },

        //---------------------dcf动作--imd=0x01-----------------------
        {
            1,
            0x01,                                //帧id
            50,                                   //超时时间
            0xFF,                                //重发次数
            0x00,                                //应答帧
            this,
            NULL,                                 //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            Emvd_Send_Before_Proc                 //发送处理回调函数
        },
        //---------------------dcf阀检测--imd=0x02-----------------------
        {
            1,
            0x02,                                //帧id
            30,                                   //超时时间
            0x02,                                //重发次数
            0x00,                                //应答帧
            this,
            NULL,                                 //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------dcf阀检测结果--imd=0x03-----------------------
        {
            1,
            0x03,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Emvd_Check_Proc,                      //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            0x00,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub_Heart_Proc,                      //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            0x02,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub1_ReportAnalog_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            0x04,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub1_ReportAngleXY_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            24,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub1_ReportAngleZ_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            32,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub2_ReportAnalog_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            34,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub2_ReportAngleXY_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            54,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub2_ReportAngleZ_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },

        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            62,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub3_ReportAnalog_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            64,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub3_ReportAngleXY_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------hub-heart--imd=0x00-----------------------
        {
            2,
            84,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Hub3_ReportAngleZ_Proc,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },

        //---------------------wl-heart--imd=0x00-----------------------
        {
            3,
            0x00,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            WL_Heart_Proc,                        //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------wl-coalLocal--imd=0x01-----------------------
        {
            3,
            0x01,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            WL_CoalLocal_Proc,                    //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------WL_WirelessCodeMatch_proc--imd=0x01-----------------------
        {
            3,
            0x03,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            WL_WirelessCodeMatch_proc,            //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },   
        //---------------------WL_WirelessDeCodeMatch_proc--imd=0x07-----------------------
        {
            3,
            0x07,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            WL_WirelessDeCodeMatch_proc,            //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------WL_WirelessCodeMatch_proc--imd=0x01-----------------------
        {
            3,
            0x0a,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            WL_WirelessControllKeyDown_proc,     //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------WL_WirelessCodeMatch_proc--imd=0x01-----------------------
        {
            3,
            0x13,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            WL_WirelessNeighboorCtrlReq_proc,     //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------cxb-heart--imd=0x01-----------------------
        {
            4,
            0x00,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            CXB_Heart_Proc,                        //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------cxb-recvVerson--imd=0x01-----------------------
        {
            4,
            0x01,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            CXB_RecvProgram_Proc,                 //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------cxb-recvProgram--imd=0x02-----------------------
        {
            4,
            0x02,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            CXB_RecvProgram_Proc,                 //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------cxb-recvProgram--imd=0x02-----------------------
        {
            4,
            0x06,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            CXB_RecvCrc_Proc,                     //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------cxb-recvProgram--imd=0x02-----------------------
        {
            4,
            0x07,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            CXB_RecvCrc_Proc,                 //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------angle-heart--imd=0x00-----------------------
        {
            5,
            0x00,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Angle_Heart_Proc,                     //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------angle-valueReport--imd=0x0100-----------------------
        {
            5,
            0x100,                               //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Angle_ValueReport_Proc,               //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------alarm-heart--imd=0x0000-----------------------
        {
            7,
            0x00,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Alarm_Heart_Proc,                     //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------alarm-reportPersonData--imd=0x0001-----------------------
        {
            7,
            0x01,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Alarm_Report_Proc,                    //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------alarm-setPersonPara--imd=0x0003-----------------------
        {
            7,
            0x03,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            Alarm_SetPara_Proc,                   //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------alarm-alarmCMD--imd=0x0004-----------------------
        {
            7,
            0x04,                                //帧id
            40,                                   //超时时间
            0xFF,                                //重发次数
            0x00,                                //应答帧
            this,
            NULL,                //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------alarm-actionCMD--imd=0x0005-----------------------
        {
            7,
            0x05,                                //帧id
            40,                                   //超时时间
            0xFF,                                //重发次数
            0x00,                                //应答帧
            this,
            NULL,                                 //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },
        //---------------------alarm-reset--imd=0x0006-----------------------
        {
            7,
            0x04,                                //帧id
            0,                                   //超时时间
            0x00,                                //重发次数
            0x00,                                //应答帧
            this,
            NULL,                                 //接受回调函数
            NULL,                                 //超时回调函数
            NULL,                                 //发送条件回调函数
            NULL                                  //发送处理回调函数
        },

     };

    pro_p->init_pro_frame(csmidinfo, sizeof(csmidinfo)/sizeof(CANPROHEAD));

    return 0;
}


void StopEmvdAction(void)
{
    uint32_t emvdKey = 0,iovalue = 0;
    CANDATAFORM  rxmeg;
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();
    deviceInfo.queryKeyDataProc_emvd(&emvdKey);
    deviceInfo.getDeviceAppProc_EMVD(((emvdKey>>16)&0xFF), (emvdKey>>8)&0xFF,emvdKey&0xFF,APP_EMVD_IO,&iovalue);
    if(iovalue == 0)
        return;
    deviceInfo.setDeviceAppProc_EMVD(((emvdKey>>16)&0xFF), (emvdKey>>8)&0xFF,emvdKey&0xFF,APP_EMVD_IO,0,2);
    Emvd_Controll_Proc(1,0,2,&rxmeg);
    apppro_p->can_protocol_send(((emvdKey>>16)&0xFF),rxmeg);
}



void StopAlarmAction(void)
{
    uint32_t alarmKey = 0;
    uint8_t tmp = 0;
    CANDATAFORM  rxmeg;
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();
    deviceInfo.queryKeyDataProc_alarm(&alarmKey);
    deviceInfo.getDeviceAppProc_ALARM(((alarmKey>>16)&0xFF), (alarmKey>>8)&0xFF,alarmKey&0xFF,ALARMMODE,&tmp);
    if(tmp == 0)
        return;
    deviceInfo.setDeviceAppProc_ALARM(((alarmKey>>16)&0xFF), (alarmKey>>8)&0xFF,alarmKey&0xFF,ALARMMODE,0);
    Alarm_Controll_Proc(tmp,&rxmeg);
    apppro_p->pro_del_buf_frame(rxmeg);
}


void StopEmvdAction_Offline(uint8_t driverId)
{
    CANDATAFORM  rxmeg;
    Emvd_Controll_Proc(1,0,2,&rxmeg);
    apppro_p->can_protocol_send(driverId,rxmeg);
}



//57(控制驱动)
void RecvShmCallback_Controll(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len)
{
    uint32_t iovalue = 0;
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();
    CANDATAFORM  rxmeg;
    st_DeviceStatusMsg deviceStatusMsg;
    uint8_t deviceType = device&0x1F;
    uint8_t tmp;

    memset(&deviceStatusMsg,0,sizeof(deviceStatusMsg));
    switch(deviceType)
    {
        case ID_EMVD:
            if(deviceInfo.getDeviceStateProc((device&0xE0)>>5,device&0x1F,subDevice,&deviceStatusMsg) == -1)
                return;
            if(deviceStatusMsg.isOnline == DEVICE_OFFLINE)
            {
                if(deviceInfo.getDeviceAppProc_EMVD((device&0xE0)>>5,device&0x1F,subDevice,APP_EMVD_IO,&iovalue) == -1)
                    return;
                if(iovalue == 0)
                    return;
               deviceInfo.setDeviceAppProc_EMVD((device&0xE0)>>5, device&0x1F,subDevice,APP_EMVD_IO,0,2);
               Emvd_Controll_Proc(1,0,deviceStatusMsg.deviceVersion[0],&rxmeg);
               apppro_p->can_protocol_send((device&0xE0)>>5,rxmeg);
            }
            else
            {
                if(deviceInfo.setDeviceAppProc_EMVD((device&0xE0)>>5, device&0x1F,subDevice,APP_EMVD_IO,point,*data) == -1)
                    return;
//                zprintf3("*************************point :%d******************state: %d\n",point,*data);
                if(deviceInfo.getDeviceAppProc_EMVD((device&0xE0)>>5,device&0x1F,subDevice,APP_EMVD_IO,&iovalue) == -1)
                    return;
//                zprintf3("RecvShmCallback_Controll  driver:%d,device：%d sub:%d,,,iovalue%d\n",(device&0xE0)>>5,device&0x1F,subDevice,iovalue);
                Emvd_Controll_Proc(1,iovalue,deviceStatusMsg.deviceVersion[0],&rxmeg);
                apppro_p->can_protocol_send((device&0xE0)>>5,rxmeg);
            }
            break;
        case ID_ALARM:

            if(point == ALARMMODE)
            {
//                printf("controll alarm data: %d\n",*data);
                if(*data != 0)
                {
                    tmp = 1;//设置起始状态
                    deviceInfo.getDeviceAppProc_ALARM((device&0xE0)>>5, device&0x1F,subDevice,ALARMMODE,&tmp);

                    if(tmp == 0)//当前有动作在执行,不执行其他效果
                    {
                        deviceInfo.setDeviceAppProc_ALARM((device&0xE0)>>5, device&0x1F,subDevice,point,*data);
                        if(Alarm_Controll_Proc(*data,&rxmeg) == 0)
                            apppro_p->can_protocol_send((device&0xE0)>>5,rxmeg);
                    }
                }
                else if(*data == 0)
                {
                    deviceInfo.getDeviceAppProc_ALARM((device&0xE0)>>5, device&0x1F,subDevice,point,&tmp);
                    Alarm_Controll_Proc(tmp,&rxmeg);
                    apppro_p->pro_del_buf_frame(rxmeg);
                    deviceInfo.setDeviceAppProc_ALARM((device&0xE0)>>5, device&0x1F,subDevice,point,*data);
                }
            }
            break;
        case ID_WIRELESS:
        {
            if(point == 11 && *data == 3)
            {

            }
        }
            break;
    }
}



//阀检测
void RecvShmCallback_ValueCheck(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len)
{

    uint32_t iovalue = 0;
    CANDATAFORM  rxmeg;
    st_DeviceStatusMsg deviceStatusMsg;
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();

//    zprintf3("RecvShmCallback_ValueCheck %d\n",point);
    if(deviceInfo.getDeviceStateProc((device&0xE0)>>5,device&0x1F,subDevice,&deviceStatusMsg) == -1)
        return;

    if(deviceStatusMsg.isOnline == DEVICE_OFFLINE)
    {
        return;
    }

    deviceInfo.getDeviceAppProc_EMVD((device&0xE0)>>5,device&0x1F,subDevice,APP_EMVD_CHECKIO,&iovalue);
    if(iovalue != 0)
        return;
    deviceInfo.setDeviceAppProc_EMVD((device&0xE0)>>5,device&0x1F,subDevice,APP_EMVD_CHECKIO,point,0);
    deviceInfo.getDeviceAppProc_EMVD((device&0xE0)>>5,device&0x1F,subDevice,APP_EMVD_CHECKIO,&iovalue);
    Emvd_Controll_Proc(2,iovalue,deviceStatusMsg.deviceVersion[0],&rxmeg);
    apppro_p->can_protocol_send((device&0xE0)>>5,rxmeg);
}

int countOnes(uint32_t n) {
    int count = 0;
    while (n > 0) {
        n &= (n - 1);
        count++;
    }
    return count;
}

// 比较两个uint32_t变量的二进制表示，并返回拥有更多1的变量
uint32_t compareAndReturn(uint32_t a, uint32_t b) {
    int onesA = countOnes(a);
    int onesB = countOnes(b);

    if (onesA > onesB) {
        return a;
    } else {
        return b;
    }
}



void RecvShmCallback_ControllHeart(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len)
{
    uint32_t iovalue = 0;
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();
    CANDATAFORM  rxmeg;
    st_DeviceStatusMsg deviceStatusMsg;
    uint8_t deviceType = device&0x1F;
    uint32_t heartACtion = 0;
    uint32_t valuetmp;

//    zprintf3("RecvShmCallback_ControllHeart  driver:%d,device：%d sub:%d,,,len%d\n",(device&0xE0)>>5,device&0x1F,subDevice,len);
    memset(&deviceStatusMsg,0,sizeof(st_DeviceStatusMsg));
    switch(deviceType)
    {
        case ID_EMVD:
            if(deviceInfo.getDeviceStateProc((device&0xE0)>>5,ID_EMVD,1,&deviceStatusMsg) == -1)
                return;
            for(uint8_t i = 0; i<6;i++)
            {
                if(len<=(i*2))
                    break;
                valuetmp = 0;
//                zprintf3("RecvShmCallback_ControllHeart data %d\n",data[i*2]);
                if(data[i*2]>32)
                    break;
                valuetmp = (1<<(data[i*2]));
                heartACtion |= valuetmp;
            }
            deviceInfo.getDeviceAppProc_EMVD((device&0xE0)>>5,(device&0x1F),1,APP_EMVD_IO,&iovalue);
//            zprintf3("RecvShmCallback_ControllHeart heartACtion%d，，iovalue %d\n",heartACtion,iovalue);
            if(heartACtion != iovalue)
            {
//                heartErrorTimer = 0;
//                iovalue = compareAndReturn(iovalue,heartACtion);
//                deviceInfo.setDeviceAppProc_EMVD((device&0xE0)>>5, device&0x1F,1,APP_EMVD_IO,point,iovalue);
//                Emvd_Controll_Proc(1,iovalue,deviceStatusMsg.deviceVersion[0],&rxmeg);
//                apppro_p->can_protocol_send((device&0xE0)>>5,rxmeg);
            }
            deviceInfo.setDeviceAppProc_EMVD((device&0xE0)>>5, ID_EMVD,1,APP_EMVD_ACTIONTIMER,300,0);
            break;
//        case ID_ALARM:
//            deviceInfo.getDeviceAppProc_EMVD(device&0xE0>>5,device&0x1F,subDevice,APP_EMVD_IO,&iovalue);
//            deviceInfo.setDeviceAppProc_EMVD(device&0xE0>>5, device&0x1F,subDevice,APP_EMVD_ACTIONTIMER,1000,0);
//            break;

    }
}


//61(读取设备状态)
void RecvShmCallback_ReadDeviceState(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len)
{
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();
    st_DeviceStatusMsg deviceStatusMsg;

    memset(&deviceStatusMsg,0,sizeof(deviceStatusMsg));

    if(deviceInfo.getDeviceStateProc((device&0xE0)>>5,device&0x1F,subDevice,&deviceStatusMsg) == -1)
        return;
    if(subDevice != 0)
    {
        deviceStatusMsg.ReportState = STATE_MODE_REPORT;
        if(deviceInfo.setDeviceStateProc((device&0xE0)>>5, device&0x1F,subDevice,&deviceStatusMsg) == -1)
            return;
    }
}

//63(读取设备状态)
void RecvShmCallback_ReadAllDeviceState(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len)
{
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();
    uint8_t deviceType = device&0x1F;
    if(subDevice != 0)
        return;
    switch (deviceType) {
    case 0:
        deviceInfo.SetAllDeviceStateProc_emvd();
        deviceInfo.SetAllDeviceStateProc_ir();
        deviceInfo.SetAllDeviceStateProc_angle();
        deviceInfo.SetAllDeviceStateProc_high();
        deviceInfo.SetAllDeviceStateProc_highAngle();
        deviceInfo.SetAllDeviceStateProc_alarmAngle();
        deviceInfo.SetAllDeviceStateProc_person();
        deviceInfo.SetAllDeviceStateProc_alarm();
        break;
    case ID_EMVD:
        deviceInfo.SetAllDeviceStateProc_emvd();
        break;
    case ID_IRRX:
        deviceInfo.SetAllDeviceStateProc_ir();
        break;
    case ID_ANGLE:
        deviceInfo.SetAllDeviceStateProc_angle();
        break;
    case ID_HIGH:
        deviceInfo.SetAllDeviceStateProc_high();
        break;
    case ID_ALARM:
        deviceInfo.SetAllDeviceStateProc_alarm();
        break;
    case ID_ALARMANGLE:
        deviceInfo.SetAllDeviceStateProc_alarmAngle();
        break;
    case ID_HIGHANGLE:
        deviceInfo.SetAllDeviceStateProc_highAngle();
        break;

    default:
        break;
    }
}



//uint16_t V2DeviceType = 0;
uint16_t SupportNum = 0;
uint16_t CheckDis_Person = 0;
uint16_t ReportInter_Person = 0;

uint16_t WirelessEnable = 0;
uint16_t SupportNumIncDir = 0;
uint16_t MasterControllTimer = 0;
uint16_t NumberShelves = 0;


uint16_t getSupprotNum(void)
{
    return SupportNum;
}
uint16_t getCheckDis_Person(void)
{
    return CheckDis_Person;
}
uint16_t getReportInter_Person(void)
{
    return ReportInter_Person;
}
uint16_t getWirelessEnable(void)
{
    return WirelessEnable;
}
uint16_t getSupportNumIncDir(void)
{
    return SupportNumIncDir;
}
uint16_t getMasterControllTimer(void)
{
    return MasterControllTimer;
}
uint16_t getNumberShelves(void)
{
    return NumberShelves;
}

void RecvMsgCallback_InitDevice(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len)
{
    if(len<10)
        return;
    CANDATAFORM  personTxmeg;
    uint32_t deviceIndex = 0;
//    memcpy(&V2DeviceType,data,2);
    if(ID_PERSON == (device&0x1F))
    {
//    设备类型(2)+设备编号(2)+检测距离(2)+上报间隔(2)+发送功率(2)。
        memcpy(&SupportNum,data+2,2);
        memcpy(&CheckDis_Person,data+4,2);
        memcpy(&ReportInter_Person,data+6,2);
    }
//    2)无线模块：设备类型(2)+设备编号(2)+使能(2，0否1是,下同)+架号增向(2字节,0左1右)+主控时间(2字节)+隔架架数(2)。
    if(ID_WIRELESS == (device&0x1F))
    {
        memcpy(&WirelessEnable,data+4,2);
        memcpy(&SupportNumIncDir,data+6,2);
        memcpy(&MasterControllTimer,data+8,2);
        memcpy(&MasterControllTimer,data+10,2);
    }
    StopEmvdAction();
    StopAlarmAction();

    DeviceInfo& deviceInfo = DeviceInfo::getInstance();

    zprintf3("init    can_____________ \n");

    deviceInfo.SetAllDeviceStateProc_emvd();
    deviceInfo.SetAllDeviceStateProc_wireless();
    deviceInfo.SetAllDeviceStateProc_ir();
    deviceInfo.SetAllDeviceStateProc_angle();
    deviceInfo.SetAllDeviceStateProc_high();
    deviceInfo.SetAllDeviceStateProc_highAngle();
    deviceInfo.SetAllDeviceStateProc_alarmAngle();
    deviceInfo.SetAllDeviceStateProc_person();
    deviceInfo.SetAllDeviceStateProc_alarm();

    if(deviceInfo.queryKeyDataProc_person(&deviceIndex) == -1)
        return;
    deviceIndex = ((deviceIndex>>16)&0xFF);
    Person_InitPara_Proc(&personTxmeg,getSupprotNum(),getCheckDis_Person(),getReportInter_Person());//声光初始化
    if(deviceIndex == LOGIC_CAN1)
    {
        apppro_p->candrip->write_send_data(personTxmeg);
    }
    else if(deviceIndex == LOGIC_CAN2)
        apppro_p->candrip1->write_send_data(personTxmeg);
}

uint8_t DeviceIdToV2DeviceId(uint8_t device, uint8_t fileType)
{
    uint8_t v2Device = device;
    switch (device) {
    case ID_EMVD:
        v2Device = 0x01;
        break;
    case ID_IRRX:
        v2Device = 0x03;
        break;
    case ID_ANGLE:
        v2Device = 0x05;
        break;
    case ID_PERSON:
        v2Device = 0x07;
        break;
    case ID_HIGH:
        v2Device = 0x0B;
        break;
    case ID_ALARM:
        v2Device = 0x0C;
        break;
    default:
        break;
    }
    return v2Device;
}


void RecvMsgCallback_CXBLoadProgram(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data,int len)
{
    //文件类型(2)+目的设备类型(2)+目的设备编号(2)
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();

    can_recv_program.device = device;
    can_recv_program.subDevice = subDevice;
    can_recv_program.point = point;
    can_recv_program.fileType = (data[0]|(data[1]<<8));
    can_recv_program.deviceType = (data[2]|(data[3]<<8));
    can_recv_program.deviceNum = (data[4]|(data[5]<<8));

    uint16_t deviceOnline = DEVICE_OFFLINE;
    uint8_t txData[8];
    uint8_t driver;
    CANDATAFORM  txmeg;
    V2_CXBCAN_ID cxbframeId;

    deviceInfo.getDeviceStateProc(0,ID_CXB,1,STATE_ISONLINE,&deviceOnline);
    if(deviceOnline == DEVICE_OFFLINE)
    {
        deviceInfo.getDeviceStateProc(1,ID_CXB,1,STATE_ISONLINE,&deviceOnline);
    }
    else
       driver = 0;

    if(deviceOnline == DEVICE_OFFLINE)
    {
        //文件类型(2)+目的设备类型(2)+目的设备编号(2)+结果(2个字节。251设备不存在；252文件不存在)
        txData[6] = 251;
    }
    else
    {
        memset(&cxbframeId,0,sizeof(V2_CXBCAN_ID));
        cxbframeId.RxID = V2ID_CXB;
        memcpy(&txmeg.ExtId,&cxbframeId,sizeof(V2_CXBCAN_ID));
        txmeg.DLC = 1;
        txmeg.IDE = 1;
        txmeg.RTR = 0;
        txmeg.Data[0] = (DeviceIdToV2DeviceId(can_recv_program.deviceType&0xFF,can_recv_program.fileType));
        if(driver == LOGIC_CAN2)
            apppro_p->candrip1->write_send_data(txmeg);
        else if(driver == LOGIC_CAN1)
            apppro_p->candrip->write_send_data(txmeg);
    }
    memcpy(txData,data,6);
//    Ydshrdlib::getInston()->sendMsgDataFunc(7,1,device,subDevice,point,txData,8);
}


uint8_t updateFileType = 0;
uint8_t updateDeviceType = 0;
uint8_t updateFileName[128];


void* updatePrgram_thread(void* arg)
{
    CanSendDataProc(reinterpret_cast<char*>(updateFileName),updateDeviceType,updateFileType);
    pthread_exit(NULL);
}


void RecvMsgCallback_UpdateProgram(uint8_t device, uint8_t subDevice, uint8_t point,uint8_t* data, int len)
{
    //文件类型(2)+目的设备类型(2)+目的设备编号(2)+文件绝对路径及名称
    if(len<6)
        return;
    updateFileType = data[0];
    updateDeviceType = data[2];
    memcpy(&updateFileName,(data+6),len-6);

    pthread_t updateProgram;
    pthread_create(&updateProgram, NULL, updatePrgram_thread, NULL);
}


CAN_PROGRAM_UPDATE_DATA canUpdate_Queue;
uint8_t updateFileBuf[1024*1024];
uint32_t updateFileBufpt = 0;


/*******************************************************************************************
**函数名称：CanFildReadInfoProc
**函数作用：can发送程序读取程序头
**函数参数：无
**函数输出：无
**注意事项：无
*******************************************************************************************/
int CanFileReadInfoProc(char * name)
{
    uint32_t size = 0;
    memset(&canUpdate_Queue,0,sizeof(CAN_PROGRAM_UPDATE_DATA));
    memset(&updateFileBuf,0,sizeof(updateFileBuf));
    updateFileBufpt = 0;
    if(BinaryFileHandler::GetFileSize(name,size) == -1)
        return -1;

    if(BinaryFileHandler::ReadBinaryFile(name,updateFileBuf,size))
        return -1;

    memcpy(&canUpdate_Queue.programinfo,updateFileBuf,sizeof(canUpdate_Queue.programinfo));
    return 0;
}


void reportUpdateStateProc(uint8_t appid, uint8_t device,uint8_t fileType,uint8_t data)
{
    uint8_t reportData[8];
    //文件类型(2)+目的设备类型(2)+目的设备编号(2)+结果2个字节。
    memset(reportData,0,sizeof(reportData));
    reportData[0] = fileType;
    reportData[2] = device;
    reportData[4] = getSupprotNum();
    reportData[6] = data;
    Ydshrdlib::getInston()->sendMsgDataFunc(1,3,device,1,0,reportData,8);
}


void CanSendDataProc(char* name, uint8_t device,uint8_t fileType)
{
    CANDATAFORM  canFrame;
    uint8_t data[8];
    uint32_t driver;
    uint8_t actDevice = 0;
    uint16_t deviceState = 0;
    DeviceInfo& deviceInfo = DeviceInfo::getInstance();

    if(fileType == FILE_PROGRAM_APP || fileType == FILE_PROGRAM_BOOT)
        ;
    else
        return;

    if(CanFileReadInfoProc(name) == -1)	//打开文件，读取bin文件头信息
    {
        reportUpdateStateProc(7,device,fileType,FILE_UPDATE_FILE_ABSENT);
        zprintf3("*****fileType:%d***device:%d****FILE_UPDATE_FILE_ABSENT**\n",fileType,device);
        return;
    }
    canUpdate_Queue.packetNum = 1;

    if(fileType == FILE_PROGRAM_APP)
        actDevice = device;
    else
    {
        if(device == ID_HIGH)
            actDevice = ID_HIGH_BL;
        else if(device == ID_ALARM)
            actDevice = ID_ALARM_BL;
        else if(device == ID_PERSON)
            actDevice = ID_PERSON_BL;
    }

    if(deviceInfo.queryKeyDataProc(device,&driver) == -1)
    {
        reportUpdateStateProc(3,fileType,device,FILE_UPDATE_DEVICE_OFFLINE);
        zprintf3("*****fileType:%d***device:%d****FILE_UPDATE_DEVICE_OFFLINE**\n",fileType,device);
        return;
    }
    else
    {
        driver = ((driver>>16)&0xFF);
        if(driver == LOGIC_CAN1 || driver == LOGIC_CAN2)
            ;
        else
        {
            reportUpdateStateProc(7,fileType,device,FILE_UPDATE_DEVICE_OFFLINE);
            zprintf3("*****fileType:%d***device:%d****FILE_UPDATE_DEVICE_OFFLINE**\n",fileType,device);
            return;
        }
    }
    memcpy(&data,&updateFileBuf,8);
    UpdateProgram_Proc(device,1,canUpdate_Queue.packetNum,data,8,&canFrame);
    apppro_p->can_protocol_send_cxb(driver, canFrame);


    reportUpdateStateProc(3,fileType,device,1);
    zprintf3("*****fileType:%d***device:%d****1**\n",fileType,device);

    if(device == ID_ALARM ||device == ID_PERSON || device == ID_ALARMANGLE)//声光和人员特殊处理，需等待跳转BootLoader
        sleep(1);		//等待跳转BootLoader
    if(device == ID_HIGH)
        sleep(2);		//等待跳转BootLoader
    if(device == ID_IRRX)
        sleep(1);
    if(device == ID_EMVD)
        sleep(1);
    if(device == ID_ANGLE)
        sleep(1);
    if(device == ID_MINIHUB1)
        sleep(1);
    switch(device)
    {
        case ID_HIGH:
            deviceInfo.getDeviceStateProc(driver,device,1,STATE_DEVICESTATUS,&deviceState);
            break;
        case ID_ANGLE:
            deviceInfo.getDeviceStateProc(driver,device,1,STATE_DEVICESTATUS,&deviceState);
            break;
        case ID_IRRX:
            deviceInfo.getDeviceStateProc(driver,device,1,STATE_DEVICESTATUS,&deviceState);
            break;

        case ID_ALARMANGLE:
        case ID_EMVD:
        case ID_MINIHUB1:
        case ID_ALARM:
        case ID_PERSON:
            deviceState = DEVICEPROGRAM;
            deviceInfo.setDeviceStateProc(driver,device,1,STATE_DEVICESTATUS,deviceState);//查询状态
            break;
        default:
            break;
    }//查询状态
    if(device == ID_ALARM || device == ID_HIGH)//声光和人员特殊处理，需等待跳转BootLoader
    {
        if(deviceState == DEVICEPROGRAM)
        {
            apppro_p->can_protocol_send_cxb(driver, canFrame);
        }
        else
        {
            if(device == ID_HIGH)
            {
                apppro_p->can_protocol_send_cxb(driver, canFrame);
            }
        }
    }

    canUpdate_Queue.packetNum++;
    updateFileBufpt = updateFileBufpt+8;
    if(canUpdate_Queue.programinfo.u32PrgSize<THE_DEV_PRG_BULK_SIZE)
        sleep(FLASH_BULK_ERASER_TIME);
    else
        sleep((canUpdate_Queue.programinfo.u32PrgSize/THE_DEV_PRG_BULK_SIZE+1)*FLASH_BULK_ERASER_TIME);

    while(updateFileBufpt < PROGRAM_INFO_SIZE)//发送头信息其他帧
    {
        memcpy(data,(updateFileBuf + updateFileBufpt),8);

        UpdateProgram_Proc(device,1,canUpdate_Queue.packetNum,data,8,&canFrame);
        apppro_p->can_protocol_send_cxb(driver, canFrame);

        canUpdate_Queue.packetNum++;
        updateFileBufpt += 8;
        usleep(20*1000);
    }

    canUpdate_Queue.packetNum = 1;
    uint8_t timers = 0;
    while(updateFileBufpt<(canUpdate_Queue.programinfo.u32PrgSize+256))//发程序
    {

        if((canUpdate_Queue.programinfo.u32PrgSize+256 - updateFileBufpt) >=8)
        {
            memcpy(data,(updateFileBuf + updateFileBufpt),8);
            UpdateProgram_Proc(device,2,canUpdate_Queue.packetNum,data,8,&canFrame);
            apppro_p->can_protocol_send_cxb(driver, canFrame);
            updateFileBufpt += 8;
        }
        else
        {
            memcpy(data,(updateFileBuf + updateFileBufpt),(canUpdate_Queue.programinfo.u32PrgSize+256 - updateFileBufpt));
            UpdateProgram_Proc(device,2,canUpdate_Queue.packetNum,data,(canUpdate_Queue.programinfo.u32PrgSize+256 - updateFileBufpt),&canFrame);
            apppro_p->can_protocol_send_cxb(driver, canFrame);
            updateFileBufpt = (canUpdate_Queue.programinfo.u32PrgSize+256);
        }
        canUpdate_Queue.packetNum++;

        timers++;
        if(timers == 50)
        {
            timers = 0;
            uint32_t updateTimer = updateFileBufpt*100/(canUpdate_Queue.programinfo.u32PrgSize+256);
            if(updateTimer>100)
                updateTimer = 100;

            reportUpdateStateProc(3,fileType,device,updateTimer);
            zprintf3("*****fileType:%d***device:%d****updateTimer:%d**\n",fileType,device,updateTimer);
        }
        usleep(20*1000);
    }

    reportUpdateStateProc(3,fileType,device,100);
    zprintf3("*****fileType:%d***device:%d****100**\n",fileType,device);


    deviceState = DEVICENORMAL;
    deviceInfo.setDeviceStateProc(driver,device,1,STATE_DEVICESTATUS,deviceState);

    if(actDevice == ID_HIGH_BL || actDevice == ID_ALARM_BL || actDevice == ID_PERSON_BL)
    {
        Alarm_Reset_Proc(device,&canFrame);
        apppro_p->can_protocol_send(driver, canFrame);
    }
}






dyk_can::dyk_can(ncan_protocol * pro)
{
    pro_p = pro;
    apppro_p = pro;
}


#endif
