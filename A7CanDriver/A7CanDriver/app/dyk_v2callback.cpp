#ifndef __DYK_V2CALLBACK_C__
#define __DYK_V2CALLBACK_C__


#include "dyk_v2pro.h"
#include "timers.h"
#include "zprint.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_handler.h"
#include "fileload.h"
#include "ydshrdlib.h"

STR_CAN_RECV_PROGRAM can_recv_program;

//电磁阀驱动器功能接口最大数
#define EMVD_OUTNUMB_MAX			32
/**动作命令字转换表，将按键按下功能键转换为对应电磁阀驱动器输出口的命令字。2018.08.16，电磁阀驱动器变更为26输出口。**/
/**对应的电磁阀驱动器程序版本号为V2.6.X。**/
/**2019年9月份，电磁阀驱动器变更为30功能输出口。程序版本号也为V2.6.X。**/
/**ActCmdTransTbl_V26适用于26功能~32功能**/
static const int ActCmdTransTbl_V26[EMVD_OUTNUMB_MAX] =
{
    22,23,20,21,
    18,19,16,17,
    27,26,24,25,
    9, 8, 7, 6,
    5, 4, 3, 2,
    1, 0,15,14,
    13,12,10,11,
    28,29,30,31
};

//动作命令字转换表，将按键按下功能键转换为对应电磁阀驱动器输出口的命令字。
//对应的电磁阀驱动器程序版本号为V1.0.x
static const int ActCmdTransTbl_V22[EMVD_OUTNUMB_MAX] =
{
    16,17,14,15,
    12,13,10,11,
    21,20,18,19,
    9, 8, 7, 6,
    5, 4, 3, 2,
    1, 0,-1,-1,
    -1,-1,-1,-1,
    -1,-1,-1,-1
};


DeviceInfo& deviceInfo = DeviceInfo::getInstance();


static uint8_t	s_u8CRC_TABLE[]= {0x00,0x31,0x62,0x53,0xC4,0xF5,0xA6,0x97,0xB9,0x88,0xDB,0xEA,0x7D,0x4C,0x1F,0x2E};	 //CRC校验表


uint16_t const u16Crc16tab[256] =
{
0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

void Crc16Ccitt(const uint8_t *u8Buf, uint32_t u32Len, uint16_t *u16CheckOld)
{
    uint32_t u32Cnt = 0x00;
    uint16_t u16Crc = *u16CheckOld;


    for( u32Cnt = 0; u32Cnt < u32Len; u32Cnt++)
        u16Crc = (u16Crc<<8) ^ u16Crc16tab[((u16Crc>>8) ^ *(uint8_t *)u8Buf++)&0x00FF];
    *u16CheckOld = u16Crc;
}


/***********************************************************
** 函数名：CRC_8
** 输　入：unsigned char *PData,unsigned char Len
** 输　出：CRC校验结果
** 功能描述：8位CRC校验函数
************************************************************/
uint8_t	CRC_8(uint8_t *PData, uint8_t Len)
{
    uint8_t	CRC_Temp=0;
    uint8_t	Temp, i;
    uint8_t	PData_H=0;
    uint8_t	PData_L=0;

    for (i = 0; i < Len; i++)
    {
        PData_L=PData[i];
        if (i < (Len-1))
        {
            PData_H=PData[i+1];

            Temp=CRC_Temp>>4;
            CRC_Temp<<=4;
            CRC_Temp^=s_u8CRC_TABLE[Temp^(PData_H>>4)];
            Temp=CRC_Temp>>4;
            CRC_Temp<<=4;
            CRC_Temp^=s_u8CRC_TABLE[Temp^(PData_H&0x0F)];
            i++;
        }

        Temp=CRC_Temp>>4;
        CRC_Temp<<=4;
        CRC_Temp^=s_u8CRC_TABLE[Temp^(PData_L>>4)];
        Temp=CRC_Temp>>4;
        CRC_Temp<<=4;
        CRC_Temp^=s_u8CRC_TABLE[Temp^(PData_L&0x0F)];
    }

    return (CRC_Temp);
}


uint32_t byteChangeLow_U8ToU32(uint8_t *data,uint8_t size)
{
    uint32_t midvp=0;
    if((size>4)||(data==NULL))
        return midvp;

    for(int i=0;i<size;i++)
        midvp +=((uint32_t)data[i])<<((size-1-i)*8);
    return midvp;
}


int Emvd_Heart_Proc(void * pro, CANDATAFORM  rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    st_DeviceStatusMsg statusMsg;
    memset(&statusMsg,0,sizeof(st_DeviceStatusMsg));
    statusMsg.HeartTimer = EMVD_HEART_TIMER;
    statusMsg.isOnline = DEVICE_ONLINE;
    statusMsg.deviceStatus = DEVICE_STATE_NORMAL;

    statusMsg.deviceVersion[0] = rxmeg.Data[5];
    statusMsg.deviceVersion[1] = rxmeg.Data[6];
    statusMsg.deviceVersion[2] = rxmeg.Data[7];
    deviceInfo.setDeviceStateProc(canp->canIndex,ID_EMVD,1,&statusMsg);

//    zprintf3("emvd_heart_proc canp->canIndex:%d \n",canp->canIndex);

    return 1;
}


int Emvd_Send_Before_Proc(void *pro ,void * pro1, CANDATAFORM*  rxmeg)
{
    CANPROHEAD * canp1 = (CANPROHEAD *) pro1;
    ncan_protocol * canp = (ncan_protocol *) pro;
    canp->pro_del_buf_frame(*rxmeg);
    if(rxmeg->Data[0] == 0 && rxmeg->Data[1] == 0 && rxmeg->Data[2] == 0 && rxmeg->Data[3] == 0)
    {
        canp1->repeatnum = 0x03;
    }
    return 1;
}


int Emvd_Check_Proc(void * pro, CANDATAFORM  rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint8_t res = 0;
    if(rxmeg.Data[4] == 8)//短路
       res = 3;
    else if(rxmeg.Data[4] == 4)
        res = 2;
    else if(rxmeg.Data[4] == 1)
        res = 1;
    else
        return 0;
    deviceInfo.setDeviceAppProc_EMVD(canp->canIndex,ID_EMVD,1,APP_EMVD_STATE,res,0);
    return 1;
}


int Hub_Heart_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    st_DeviceStatusMsg statusMsg;
    uint8_t deviceType;
    uint8_t device;
    uint8_t subdevice;
    memset(&statusMsg,0,sizeof(st_DeviceStatusMsg));

    deviceType =rxmeg.Data[0];
    if(deviceType == 'H')
    {
        memset(&statusMsg,0,sizeof(st_DeviceStatusMsg));
        device = rxmeg.Data[3];
        statusMsg.HeartTimer = HUB_HEART_TIMER;
        statusMsg.isOnline = DEVICE_ONLINE;
        statusMsg.deviceStatus = DEVICE_STATE_NORMAL;

        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_MINIHUB1+device,1,&statusMsg);
    }
    else
    {
        device = rxmeg.Data[3] + ID_MINIHUB1;
        subdevice = rxmeg.Data[2] - V2_ALARM_HEART;

        statusMsg.HeartTimer = ANGLE_HEART_TIMER;
        statusMsg.isOnline = DEVICE_ONLINE;
        statusMsg.deviceStatus = DEVICE_STATE_NORMAL;
        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];

        deviceInfo.setDeviceStateProc_MiniHub(device,(canp->canIndex<<16)|(device<<8)|1,subdevice,&statusMsg);
    }
    return 1;
}

int Hub1_ReportAnalog_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB1<<8)|1);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_ANGLOG1,&rxmeg.Data[0],8);
    return 1;
}

int Hub1_ReportAngleXY_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    V2_CAN_ID v2id;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB1<<8)|1);
    memcpy(&v2id,&rxmeg.ExtId,sizeof(V2_CAN_ID));
    if(v2id.Sum == 1)
    {
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_X1,&rxmeg.Data[0],4);
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_X2,&rxmeg.Data[4],4);
    }
    else if(v2id.Sub == 1)
    {
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_X3,&rxmeg.Data[0],4);
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_X4,&rxmeg.Data[4],4);
    }
    return 1;
}

int Hub1_ReportAngleZ_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB1<<8)|1);

    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_Z1,&rxmeg.Data[0],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_Z2,&rxmeg.Data[2],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_Z3,&rxmeg.Data[4],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB1,deviceTmp,MINIHUB_Z4,&rxmeg.Data[6],2);
    return 1;
}


int Hub2_ReportAnalog_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB2<<8)|1);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_ANGLOG1,&rxmeg.Data[0],8);
    return 1;
}

int Hub2_ReportAngleXY_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    V2_CAN_ID v2id;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB2<<8)|1);
    memcpy(&v2id,&rxmeg.ExtId,sizeof(V2_CAN_ID));
    if(v2id.Sum == 1)
    {
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_X1,&rxmeg.Data[0],4);
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_X2,&rxmeg.Data[4],4);
    }
    else if(v2id.Sub == 1)
    {
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_X3,&rxmeg.Data[0],4);
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_X4,&rxmeg.Data[4],4);
    }
    return 1;
}

int Hub2_ReportAngleZ_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB2<<8)|1);

    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_Z1,&rxmeg.Data[0],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_Z2,&rxmeg.Data[2],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_Z3,&rxmeg.Data[4],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB2,deviceTmp,MINIHUB_Z4,&rxmeg.Data[6],2);
    return 1;
}

int Hub3_ReportAnalog_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB3<<8)|1);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_ANGLOG1,&rxmeg.Data[0],8);
    return 1;
}

int Hub3_ReportAngleXY_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    V2_CAN_ID v2id;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB3<<8)|1);
    memcpy(&v2id,&rxmeg.ExtId,sizeof(V2_CAN_ID));
    if(v2id.Sum == 1)
    {
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_X1,&rxmeg.Data[0],4);
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_X2,&rxmeg.Data[4],4);
    }
    else if(v2id.Sub == 1)
    {
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_X3,&rxmeg.Data[0],4);
        deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_X4,&rxmeg.Data[4],4);
    }
    return 1;
}

int Hub3_ReportAngleZ_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t deviceTmp;
    deviceTmp = ((canp->canIndex<<16)|(ID_MINIHUB3<<8)|1);

    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_Z1,&rxmeg.Data[0],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_Z2,&rxmeg.Data[2],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_Z3,&rxmeg.Data[4],2);
    deviceInfo.setDeviceAppProc_MINIHUB(ID_MINIHUB3,deviceTmp,MINIHUB_Z4,&rxmeg.Data[6],2);
    return 1;
}


/*******************************************************************************************
**函数名称：RemoteContralOverrideWarnin
**函数作用：控制器预警控制
**函数参数：ykqID：被控架号，status：0启动，1停止
**函数输出：无
**注意事项：无
*******************************************************************************************/
void RemoteContralOverrideWarnin(void * pro, uint8_t ScID, const bool &status)
{
    CanDriver * canp = (CanDriver *) pro;
    RemoteActionCtrlDataType ActionData;
    uint32_t ykqNum = 0;
    memset(&ActionData,0,sizeof(RemoteActionCtrlDataType));
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQNUM,&ykqNum);

    ActionData.ContralSCNum = ScID;
    ActionData.ActionID = 1;
    ActionData.ContralCommand = status;
    ActionData.ActionCtrlMode = ActionCtrl_ModeType_AUTO;
    ActionData.CtrlSourceAddr = ykqNum;
    ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
    ActionData.UserID = 0;

    Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,3,(unsigned char *)&ActionData,14);
}





int WL_Heart_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    st_DeviceStatusMsg statusMsg;
    CANDATAFORM  wirelessTxmeg;
    memset(&statusMsg,0,sizeof(st_DeviceStatusMsg));
    statusMsg.HeartTimer = WL_HEART_TIMER;
    statusMsg.isOnline = DEVICE_ONLINE;
    statusMsg.deviceStatus = DEVICE_STATE_NORMAL;

    if(rxmeg.Data[0] == 'W' && rxmeg.Data[1] == 'L' && rxmeg.Data[2] == 'M')
    {
        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];
        if(rxmeg.Data[3] == 0)
            deviceInfo.setDeviceStateProc(canp->canIndex,ID_IRRX,1,&statusMsg);
        else if(rxmeg.Data[3] == 0x31)
        {
            statusMsg.deviceStatus = rxmeg.Data[4];
            if(rxmeg.Data[4] == DEVICEINIT)
            {
                Wireless_InitPara_Proc(&wirelessTxmeg);
                canp->write_send_data(wirelessTxmeg);
            }
            deviceInfo.setDeviceStateProc(canp->canIndex,ID_WIRELESS,1,&statusMsg);
        }

    //    zprintf3("WL_Heart_Proc****rxmeg.ExtId:%x,rxmeg.DLC:%d data:%x %x %x %x %x %x %x %x \n",rxmeg.ExtId,rxmeg.DLC,rxmeg.Data[0],rxmeg.Data[1],rxmeg.Data[2],rxmeg.Data[3],rxmeg.Data[4],rxmeg.Data[5],rxmeg.Data[6],rxmeg.Data[7]);
    //    zprintf3("WL_Heart_Proc canp->canIndex:%d \n",canp->canIndex);
    }
    else
    {
        uint32_t ykqNum = 0;
        uint32_t controllNum = 0;
        uint32_t RuningActionInfo = 0;
        uint32_t temp ;

        deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQNUM,&ykqNum);
        deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CONTROLLNUM,&controllNum);
        deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_ACTIONID,&RuningActionInfo);

        if(rxmeg.Data[0] != controllNum)
            return 1;
        if(rxmeg.Data[1] != ykqNum)
            return 1;
        //重置定时器

        //动作id对应的bit位
        uint32_t manualActionID;
        manualActionID = rxmeg.Data[3]|(rxmeg.Data[4]<<8)|(rxmeg.Data[5]<<16)|(rxmeg.Data[6]<<24);
    //    qDebug("remote contral heart = 0x%08x",manualActionID);
        //给逻辑上报手动动作心跳，将动作🆔id对应的位转换位🆔id并填入u16的数组中上报给逻辑
        uint16_t TempActionId[10];
        temp = RuningActionInfo & manualActionID;
        TempActionId[0] = controllNum;
        if(temp != 0)
        {
            uint8_t j = 1;
            for(uint8_t i = 0;i < 32; i++)
            {
                if(((temp >> i)&0x01) != 0)
                {
                    TempActionId[j] = i + 1;
                    j++;
                    j = j % 10;
                }
            }
            Ydshrdlib::getInston()->sendShmDataFunc(1,9,((2<<5)|9),1,0,(unsigned char *)TempActionId,j*sizeof(uint16_t));
        }
        //正常情况，心跳与实时动作一致。
        if((manualActionID^RuningActionInfo) == 0)
        {
            return 0;
        }
        else
        {
            temp = (manualActionID^RuningActionInfo)&RuningActionInfo;      //当前运行的动作与心跳不一致，找到这个动作并停止
            if(temp)
            {
                for(uint8_t i = 0;i < 32; i++)
                {
                    if(((temp >> i)&0x01) != 0)
                    {
                        //停动作
                        RemoteActionCtrlDataType ActionData;
                        ActionData.ActionID = i+1;
                        ActionData.ContralSCNum = controllNum;
                        ActionData.ContralCommand = ActionCtrl_CmdType_STOP;
                        ActionData.ActionCtrlMode = ActionCtrl_ModeType_MANUAL; //手动
                        ActionData.CtrlSourceAddr = ykqNum;
                        ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
                        ActionData.UserID = 0;
                        Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,0,(unsigned char *)&ActionData,14);
                        qDebug("Action Heart Error,Mast Stop action:%d",ActionData.ActionID);
                    }
                    else
                        continue;
                }

            }
        }
        //无动作的情况
        if(manualActionID == 0)
        {
            TempActionId[1] = 0;
            Ydshrdlib::getInston()->sendShmDataFunc(1,9,((2<<5)|9),1,0,(unsigned char *)TempActionId,4);
        }
    }
    return 1;
}

int WL_CoalLocal_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    deviceInfo.setDeviceAppProc_WL(canp->canIndex,ID_IRRX,1,1);
//    zprintf3("WL_CoalLocal_Proc canp->canIndex:%d \n",canp->canIndex);
    return 1;
}


/*******************************************************************************************
**函数名称：RemoteContralCodeMatchingAck_Proc
**函数作用：发送遥控器对码帧应答
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
void frameRemoteContralCodeMatchingAck_Proc(void * pro,uint8_t state,uint8_t scnum,uint8_t ykqnum)
{
    //获取主控时间
//    uint16_t scactiveTimer;
    CanDriver * canp = (CanDriver *) pro;
    V2_CAN_ID frame;
    CANDATAFORM txmeg;
    memset(&txmeg,0,sizeof(CANDATAFORM));
    memset(&frame,0,sizeof(V2_CAN_ID));

    frame.ACK = 0;
    frame.FrameType = V4YKQ_WL_CODEMATCH_ACK;
    frame.LiuShuiNumb = 0;
    frame.RxID = 3;
    txmeg.DLC = 8;
    txmeg.Data[0] = 0;
    txmeg.Data[1] = 1;
    txmeg.Data[2] = 0x40;
    txmeg.Data[2] |= 0x80;
    txmeg.Data[2] |= state;
    txmeg.Data[3] = 1;
    txmeg.Data[4] = scnum;
    txmeg.Data[5] = ykqnum;
    txmeg.Data[6] = 20;
    txmeg.Data[7] = 20;

    memcpy(&txmeg.ExtId,&frame,sizeof(V2_CAN_ID));
    txmeg.IDE = 1;
    txmeg.RTR = 0;

    canp->writeframe(txmeg);
}


int WL_WirelessCodeMatch_proc(void * pro,CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint8_t data[2];
    if(rxmeg.Data[5] != getSupprotNum())
        return 0;
    memset(&data,0,sizeof(data));
    data[0] = 2;
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,(canp->canIndex<<5)|ID_WIRELESS,1,11,data,2);

    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQCRC,(rxmeg.Data[7]<<8)|rxmeg.Data[6]);
    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQNUM,rxmeg.Data[5]);
    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CODESTATE,WIRELESS_STATE_WLMATCH);
    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CONTROLLNUM,getSupprotNum());
    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CODEINGTIMER,2000);

    //发送无线对码应答
    frameRemoteContralCodeMatchingAck_Proc(pro,WIRELESS_STATE_WLMATCH,getSupprotNum(),rxmeg.Data[5]);

    return 0;
}


/*******************************************************************************************
**函数名称：RemoteContralActionSendAck
**函数作用：遥控器动作控制应答帧
**函数参数：actionType:动作类型， actionid：动作id
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
void RemoteContralActionSendAck(void * pro, uint8_t &actionType, uint8_t &actionID)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t ykqNum = 0;
    uint32_t controllNum = 0;

    V2_CAN_ID frame;
    CANDATAFORM txmeg;
    memset(&txmeg,0,sizeof(CANDATAFORM));
    memset(&frame,0,sizeof(V2_CAN_ID));

    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQNUM,&ykqNum);
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CONTROLLNUM,&controllNum);

    frame.ACK = 0;
    frame.FrameType = V4YKQ_CONTROL_DATA_ACK;
    frame.LiuShuiNumb = 0;
    frame.RxID = 3;
    txmeg.DLC = 8;
    txmeg.Data[0] = ykqNum;
    txmeg.Data[1] = controllNum;
    txmeg.Data[2] = actionType;
    txmeg.Data[3] = actionID;
    txmeg.Data[4] = 1;
    txmeg.Data[5] = 0;
    txmeg.Data[6] = 0;
    txmeg.Data[7] = 0;

    memcpy(&txmeg.ExtId,&frame,sizeof(V2_CAN_ID));
    txmeg.IDE = 1;
    txmeg.RTR = 0;

    canp->writeframe(txmeg);
}


int WL_WirelessControllKeyDown_proc(void * pro,CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    //获取本架架号
    uint8_t pstate = 1;
    uint8_t supportDirect;
    uint8_t ManualOrAuto;
    uint8_t buf[7] = {0};
    uint32_t numtmp = 1;
    uint32_t ykqNum = 0;
    uint32_t codeState = 0;
    uint32_t controllNum = 0;


    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CODEINGTIMER,2000);
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQNUM,&ykqNum);
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CODESTATE,&codeState);
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CONTROLLNUM,&controllNum);

    if(rxmeg.Data[0] != ykqNum)
        return 0;
    if(codeState != WIRELESS_STATE_WLMATCH)
        return 0;
    RemoteActionCtrlDataType ActionData;
    //frame->u8DT[2]//bit2手动/自动
    //frame->u8DT[3]//被控设备ID
    //frame->u8DT[4]//控制命令字
    //frame->u8DT[5]//起始架号
    //frame->u8DT[6]//结束架号
    //frame->u8DT[7]//执行方向

    switch(rxmeg.Data[2]&0X07)
    {
        case SINGLE_CONTROL:
        {
            ActionData.ContralSCNum = controllNum;
            ActionData.ActionID = rxmeg.Data[3];
            ActionData.ContralCommand = rxmeg.Data[4];
            if((rxmeg.Data[2]&0x08) == 0x08)
                ActionData.ActionCtrlMode = ActionCtrl_ModeType_AUTO;   //自动
            else
                ActionData.ActionCtrlMode = ActionCtrl_ModeType_MANUAL; //手动
            ActionData.CtrlSourceAddr = ykqNum;
            ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ActionData.UserID = 0;

            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,0,(unsigned char *)&ActionData,14);
            //手动动作心跳处理
            if(ActionData.ActionCtrlMode == ActionCtrl_ModeType_MANUAL)
            {
                if(ActionData.ContralCommand == ActionCtrl_CmdType_STARTUP)
                {
                    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_ACTIONID,ActionData.ActionID - 1);
//                    Timer::getInstance().startTimerEvent(timeIdRemoteActionHeart);
                }
            }
            //test
            zprintf3("----------single control------------");
            zprintf3("action id = %d",ActionData.ActionID);
            zprintf3("ActionCtrlMode = %d",ActionData.ActionCtrlMode);
            zprintf3("ContralCommand = %d",ActionData.ContralCommand);
            zprintf3("-----------------------------------");
        }
            break;
        case COMB_CONTROL:
        {
            ActionData.ContralSCNum = controllNum;
            ActionData.ActionID = rxmeg.Data[3];
            ActionData.ContralCommand = rxmeg.Data[4];
            ActionData.ActionCtrlMode = ActionCtrl_ModeType_AUTO;
            ActionData.CtrlSourceAddr = ykqNum;
            ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ActionData.UserID = 0;

            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,1,(unsigned char *)&ActionData,14);
            //test
            zprintf3("----------comb control------------");
            zprintf3("action id = %d",ActionData.ActionID);
            zprintf3("ContralCommand = %d",ActionData.ContralCommand);
            zprintf3("-----------------------------------");
        }
            break;
        case GROUP_CONTROL:
        {
            if(rxmeg.Data[7] == 0) //从近到远
            {
                ActionData.ContralSCNum = rxmeg.Data[5];    //V4ykq.controllNum;
                ActionData.GroupBeginSC = rxmeg.Data[5];
                ActionData.GroupEndSc = rxmeg.Data[6];
            }
            else {
                ActionData.ContralSCNum = rxmeg.Data[6];    //V4ykq.controllNum;
                ActionData.GroupBeginSC = rxmeg.Data[6];
                ActionData.GroupEndSc = rxmeg.Data[5];
            }
            //小到大1，大到小2
            if(ActionData.GroupBeginSC < ActionData.GroupEndSc)
                ActionData.GroupExecutionDirection = 1;
            else
                ActionData.GroupExecutionDirection = 2;

            ActionData.ActionID = rxmeg.Data[3];
            ActionData.ContralCommand = rxmeg.Data[4];
            ActionData.ActionCtrlMode = ActionCtrl_ModeType_AUTO;
            ActionData.CtrlSourceAddr = ykqNum;
            ActionData.CtrlSourceType = ActionCtrl_SourceType_HANDLER;
            ActionData.UserID = 0;
            ActionData.DataLength = 6;

            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,2,(unsigned char *)&ActionData,sizeof(RemoteActionCtrlDataType));
            //test
            zprintf3("----------Group control------------");
            zprintf3("action id = %d",ActionData.ActionID);
            zprintf3("ContralCommand = %d",ActionData.ContralCommand);
            zprintf3("GroupBeginSC id = %d",ActionData.GroupBeginSC);
            zprintf3("GroupEndSc = %d",ActionData.GroupEndSc);
            zprintf3("GroupExecutionDirection = %d",ActionData.GroupExecutionDirection);
            zprintf3("-----------------------------------");
        }
            break;
        case FUNCTION_KEY:
        {
            //被控架号切换回控制器架号。
            deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CONTROLLNUM,&controllNum);
            //给逻辑报停止
            uint16_t temp[2];
            temp[0] = 1;
            temp[1] = 1;
            //停止键
            Ydshrdlib::getInston()->sendShmDataFunc(1,7,((2<<5)|9),1,99,(unsigned char *)temp,sizeof(temp));
            //test
            zprintf3("remote control STOP");
        }
            break;
    }
    RemoteContralActionSendAck(pro,rxmeg.Data[2],rxmeg.Data[3]);
    return 1;
}


/*******************************************************************************************
**函数名称：RemoteContralDecondeDeal
**函数作用：遥控器主动解除对码帧
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
int WL_WirelessDeCodeMatch_proc(void * pro,CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t ykqNum = 0;
    uint32_t controllNum = 0;
    uint32_t matchState = 0;

    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQNUM,&ykqNum);
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CONTROLLNUM,&controllNum);
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CODESTATE,&matchState);

    if(rxmeg.Data[3] != controllNum)
        return 1;
    if(rxmeg.Data[4] != ykqNum)
        return 1;
//    if(matchState != STATE_WL_SUCCESS)
//        return 1;

    //发送解除预警
    RemoteContralOverrideWarnin(pro,controllNum,ActionCtrl_CmdType_STOP);

    //发送解除对码状态，更新遥控器状态
    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQNUM,0);
    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CONTROLLNUM,0);
    deviceInfo.setDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CODESTATE,0);
    uint16_t RemoteContralConnectStatus = 3;
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((2<<5)|9),1,11,(unsigned char *)&RemoteContralConnectStatus,sizeof(RemoteContralConnectStatus));
    //停定时器

    return 0;
}


/*******************************************************************************************
**函数名称：RemoteContralNeighboorCtrlReqDeal
**函数作用：遥控器邻架控制请求
**函数参数：pframe:接收的数据帧
**函数输出：0：正确，非0：失败
**注意事项：无
*******************************************************************************************/
int WL_WirelessNeighboorCtrlReq_proc(void * pro,CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t ykqNum = 0;
    uint32_t controllNum = 0;
    uint32_t matchState = 0;

    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_YKQNUM,&ykqNum);
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CONTROLLNUM,&controllNum);
    deviceInfo.getDeviceAppProc_Wireless(canp->canIndex,ID_WIRELESS,1,WIRELESS_CODESTATE,&matchState);

    if(rxmeg.Data[1] != controllNum)
        return 1;
    if(rxmeg.Data[0] != ykqNum)
        return 1;

//    if(V4ykq.YkqState != STATE_WL_SUCCESS)
//        return 1;
    //记录上一次的预警架号
    zprintf3("Recv control sc num = %d",rxmeg.Data[2]);
    RemoteContralOverrideWarnin(pro,controllNum,ActionCtrl_CmdType_STOP);
    zprintf3("stop alarm sc num = %d",controllNum);
    //判断隔架架数
//    if(controllNum > frame->u8DT[2])
//    {
//        V4ykq.controllNum = (scNum-frame->u8DT[2] > gapNum)?(V4ykq.SCNum - gapNum):frame->u8DT[2];
//    }
//    else
//    {
//        V4ykq.controllNum = (frame->u8DT[2] - scNum > gapNum)?(V4ykq.SCNum + gapNum):frame->u8DT[2];
//    }
//    //发送邻架控制应答
//    RemoteContralNeighborAckProc();
//    //发送邻架预警
//    RemoteContralOverrideWarnin(V4ykq.controllNum,ActionCtrl_CmdType_STARTUP);
//    qDebug("start alarm sc num = %d",V4ykq.controllNum);
//    //邻架控制状态上报
//    uint16_t RemoteContralConnectStatus = 4;
//    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((2<<5)|9),1,11,(unsigned char *)&RemoteContralConnectStatus,sizeof(uint16_t));

    return 0;
}




uint32_t u32CxbSaveQueuepWrite;
uint8_t  u8ScSaveProgramQueue[1024*1024];

uint32_t u32CxbRecvQueuepWrite;
uint8_t  u8ScRecvProgramQueue[256];

void * recvCxbProgram_thread(void *arg)
{
    while(1)
    {
        if(u32CxbSaveQueuepWrite!= 0)
        {

            if(u32CxbSaveQueuepWrite == (can_recv_program.fileSize+256))
            {

                zprintf3("recvCxbProgram_thread recv:%d    total: %d\n",u32CxbSaveQueuepWrite,can_recv_program.fileSize);
                zprintf3("save file:%d \n");
                if (BinaryFileHandler::CreateBinaryFile("data.bin"))
                {
                    zprintf3("二进制文件创建成功。: \n");
                } else
                {
                    zprintf3("二进制文件创建失败。: \n");
                }
                if (BinaryFileHandler::WriteBinaryFile("data.bin", u8ScSaveProgramQueue, u32CxbSaveQueuepWrite))
                {
                    zprintf3("二进制文件写入成功。: \n");
                } else
                {
                    zprintf3("二进制文件写入失败。: \n");
                }

                u32CxbSaveQueuepWrite = 0;

                std::cout << "线程执行结束。" << std::endl;
                pthread_exit(NULL);
            }
        }
        usleep(100000);
    }
}

pthread_t recvCxbThread;

#if 0
static uint8_t iiii = 0;
#endif

int CXB_Heart_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    st_DeviceStatusMsg statusMsg;
#if 0
    CANDATAFORM txmeg;
    if(iiii == 0)
    {
        txmeg.Data[0] = 1;
        txmeg.DLC = 1;
        txmeg.ExtId = 0x08000004;
        txmeg.RTR = 0;
        txmeg.IDE = 1;
        canp->write_send_data(txmeg);
        iiii++;
    }
#endif
    if(rxmeg.DLC == 8)
    {
        memset(&statusMsg,0,sizeof(st_DeviceStatusMsg));
        statusMsg.HeartTimer = CXB_HEART_TIMER;
        statusMsg.isOnline = DEVICE_ONLINE;
        statusMsg.deviceStatus = DEVICE_STATE_NORMAL;

        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_CXB,1,&statusMsg);
        zprintf3("CXB_Heart_Proc canp->canIndex:%d \n",canp->canIndex);
    }
    else if(rxmeg.DLC == 1)
    {

        if(rxmeg.Data[0] == 0xFF)
        {
//            Ydshrdlib::getInston()->sendMsgDataFunc(7,1,device,subDevice,point,txData,8);
            return -1;
        }

        memset(&u8ScSaveProgramQueue,0,sizeof(u8ScSaveProgramQueue));
        u32CxbRecvQueuepWrite = 0;
        memset(&can_recv_program,0,sizeof(STR_CAN_RECV_PROGRAM));

        int result = pthread_create(&recvCxbThread, NULL, recvCxbProgram_thread, NULL);
        if (result != 0) {
            zprintf3("无法创建线程。错误代码: %d \n",result);
            return result;
        }
        zprintf3("CXB_RequestDownload_Proc device:%d \n",result);
    }
    return 1;
}


int CXB_RecvProgram_Proc(void * pro, CANDATAFORM rxmeg)
{
    V2_CXBCAN_ID cxbframeId;
    memcpy(&cxbframeId,&rxmeg.ExtId,sizeof(V2_CXBCAN_ID));
    if(cxbframeId.PacketNum == 1)
    {
        u32CxbRecvQueuepWrite = 0;
        memset(&u8ScRecvProgramQueue,0,sizeof(u8ScRecvProgramQueue));
    }
    if(u32CxbRecvQueuepWrite+rxmeg.DLC > 256)
        return 0;
    memcpy(&u8ScRecvProgramQueue[u32CxbRecvQueuepWrite],&rxmeg.Data,rxmeg.DLC);
    u32CxbRecvQueuepWrite+=rxmeg.DLC;
    return 1;
}


int CXB_RecvCrc_Proc(void * pro, CANDATAFORM rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint32_t u32StartPacketNumb = 0;
    uint16_t u16Crc = 0, u16CrcTemp = 0, u16crcpacket;
    V2_CXBCAN_ID cxbframeId;
    CANDATAFORM txmeg;
    uint8_t frametype;

    memcpy(&cxbframeId,&rxmeg.ExtId,sizeof(V2_CXBCAN_ID));

    frametype = cxbframeId.FrameType;
    u32StartPacketNumb = (uint32_t)(rxmeg.Data[0x00]|(rxmeg.Data[0x01]<<8));
    u32StartPacketNumb |= (uint32_t)((rxmeg.Data[0x02]<<16)|(rxmeg.Data[0x03]<<24));
    u16crcpacket = (uint32_t)(rxmeg.Data[0x04]|(rxmeg.Data[0x05]<<8));
    u16Crc = (uint32_t)(rxmeg.Data[0x06]|(rxmeg.Data[0x07]<<8));

    Crc16Ccitt(u8ScRecvProgramQueue, u32CxbRecvQueuepWrite, &u16CrcTemp);//计算此时的校验值

    if(u16CrcTemp == u16Crc)
    {
        if(frametype == 6)
        {
            can_recv_program.verson =u8ScRecvProgramQueue[11]<<24|u8ScRecvProgramQueue[10]<<16|u8ScRecvProgramQueue[9]<<8|u8ScRecvProgramQueue[8];
            can_recv_program.fileCrc = u8ScRecvProgramQueue[29]<<8|u8ScRecvProgramQueue[28];
            can_recv_program.fileSize =u8ScRecvProgramQueue[15]<<24|u8ScRecvProgramQueue[14]<<16|u8ScRecvProgramQueue[13]<<8|u8ScRecvProgramQueue[12];
        }
        memcpy(&u8ScSaveProgramQueue[u32CxbSaveQueuepWrite],u8ScRecvProgramQueue,u32CxbRecvQueuepWrite);
        u32CxbSaveQueuepWrite += u32CxbRecvQueuepWrite;
    }
    else
    {
        zprintf3("crc fail \n");
    }

    memset(u8ScRecvProgramQueue,0,sizeof(u8ScRecvProgramQueue));
    u32CxbRecvQueuepWrite = 0;
    zprintf3("send meg \n");
    cxbframeId.RxID = V2ID_CXB;
    memcpy(&txmeg,&rxmeg,sizeof(CANDATAFORM));
    memcpy(&txmeg.ExtId,&cxbframeId,sizeof(V2_CXBCAN_ID));
    txmeg.Data[6] = (u16CrcTemp & 0xFF);
    txmeg.Data[7] = (u16CrcTemp & 0xFF00)>>8;

    canp->writeframe(txmeg);
    return 1;
}

int Wireless_InitPara_Proc(CANDATAFORM * value)
{
    CANDATAFORM  rxmeg;
    V2_CAN_ID v2id;

    v2id.ACK = 0;
    v2id.FrameType = 22;
    v2id.LiuShuiNumb = 0;
    v2id.NoUsed = 0;
    v2id.Reservd = 0;
    v2id.RxID = V2ID_WL;
    v2id.Sub = 0;
    v2id.Sum = 0;
    v2id.TxID = V2ID_SC;

    rxmeg.Data[0] = getSupprotNum();
    rxmeg.Data[1] = 0x33;
    rxmeg.Data[2] = 0x33;

    memcpy(&rxmeg.ExtId, &v2id, sizeof(V2_CAN_ID));

    rxmeg.DLC = 3;
    rxmeg.IDE = 1;
    rxmeg.RTR = 0;

    *value = rxmeg;
    return 0;
}


int Angle_High_InitPara_Proc(uint8_t device, CANDATAFORM * value)
{
    CANDATAFORM  rxmeg;
    V2_CAN_ID v2id;
    stAngleParam angleParam;

    v2id.ACK = 0;
    v2id.FrameType = 0x105;
    v2id.LiuShuiNumb = 0;
    v2id.NoUsed = 0;
    v2id.Reservd = 0;
    v2id.RxID = V2ID_ANGLE;
    v2id.Sub = 0;
    v2id.Sum = 0;
    v2id.TxID = V2ID_SC;

    angleParam.forwardTimes = 0x00;
    angleParam.workMode = 0x02;
    angleParam.interval = 3000;
    if(device == ID_ANGLE)
    {
        angleParam.RxID = 0;
        angleParam.angleVariation = 10;
    }
    else if(device == ID_HIGH)
    {
        angleParam.RxID = 1;
        angleParam.angleVariation = 2;
    }
    angleParam.RxSubID  = 0;
    angleParam.reserve = 0;
    memcpy(&rxmeg.ExtId, &v2id, sizeof(V2_CAN_ID));
    memcpy(rxmeg.Data, &angleParam, sizeof(angleParam));

    rxmeg.DLC = 8;
    rxmeg.IDE = 1;
    rxmeg.RTR = 0;

    *value = rxmeg;
    return 0;
}

int Person_InitPara_Proc(CANDATAFORM * value,uint16_t scNum,uint16_t dis,uint16_t timer)
{
    CANDATAFORM  rxmeg;
    V2_CAN_ID v2id;
    stPersonParam personParam;

    v2id.ACK = 0;
    v2id.FrameType = 0x03;
    v2id.LiuShuiNumb = 0;
    v2id.NoUsed = 0;
    v2id.Reservd = 0;
    v2id.RxID = V2ID_ALARM;
    v2id.Sub = 0;
    v2id.Sum = 0;
    v2id.TxID = V2ID_SC;

    personParam.u8WORKPARAM.WORKPARAM.workMode = 1;
    personParam.u8WORKPARAM.WORKPARAM.calcMode = 1;
    personParam.u8WORKPARAM.WORKPARAM.disVariation = 0;
    personParam.u8WORKPARAM.WORKPARAM.trsPower = 11;

    personParam.interval = 20;
    personParam.range = 50;
    personParam.scHigh = 0;

    personParam.scNum = scNum;

    memcpy(&rxmeg.ExtId,&v2id,sizeof(v2id));
    memcpy(&rxmeg.Data,&personParam,sizeof(stPersonParam));
    rxmeg.DLC = 8;
    rxmeg.IDE = 1;
    rxmeg.RTR = 0;

    *value = rxmeg;
    return 0;
}


int Angle_Heart_Proc(void * pro, CANDATAFORM  rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    ncan_protocol *pros = (ncan_protocol*)canp->father_p;
    st_DeviceStatusMsg statusMsg;
    CANDATAFORM  angleTxmeg;
    memset(&statusMsg,0,sizeof(st_DeviceStatusMsg));
    statusMsg.HeartTimer = ANGLE_HEART_TIMER;
    statusMsg.isOnline = DEVICE_ONLINE;
    statusMsg.deviceStatus = DEVICE_STATE_NORMAL;
    if(rxmeg.Data[3] == 0X3A)
    {
        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_ALARMANGLE,1,&statusMsg);
        if(rxmeg.Data[4] == DEVICE_STATE_INIT)//设备初始化
        {
            Angle_High_InitPara_Proc(ID_ANGLE, &angleTxmeg);
            canp->write_send_data(angleTxmeg);
        }
//        zprintf3("angle_alarm_heart_proc canp->canIndex:%d \n",canp->canIndex);
    }
    else if(rxmeg.Data[3] < V2_HIGH_HEART)//
    {
        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_ANGLE,rxmeg.Data[3]-V2_ANGLE_HEART+1,&statusMsg);
        if(rxmeg.Data[4] == DEVICE_STATE_INIT)//设备初始化
        {
            Angle_High_InitPara_Proc(ID_ANGLE, &angleTxmeg);
            canp->write_send_data(angleTxmeg);
        }
//        zprintf3("angle_heart_proc canp->canIndex:%d \n",canp->canIndex);
    }
    else if(rxmeg.Data[3] == 0X4A || rxmeg.Data[3] == 0X4B)//角高一体中的底座角度/顶梁角度
    {
        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_HIGHANGLE,rxmeg.Data[3]-0X4A+1,&statusMsg);
        if(rxmeg.Data[4] == DEVICE_STATE_INIT)//设备初始化
        {
            Angle_High_InitPara_Proc(ID_ANGLE, &angleTxmeg);
            canp->write_send_data(angleTxmeg);
        }
//        zprintf3("angle_high_heart_proc canp->canIndex:%d \n",canp->canIndex);
    }
    else if(rxmeg.Data[3] == V2_HIGH_HEART)
    {
        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_HIGH,1,&statusMsg);
        if(rxmeg.Data[4] == DEVICE_STATE_INIT)//设备初始化
        {
            Angle_High_InitPara_Proc(ID_HIGH, &angleTxmeg);
            canp->write_send_data(angleTxmeg);
        }
//        zprintf3("high_heart_proc canp->canIndex:%d \n",canp->canIndex);
    }
    else if(rxmeg.Data[3] == V2_HIGHBL_HEART)
    {
        statusMsg.deviceVersion[0] = rxmeg.Data[5];
        statusMsg.deviceVersion[1] = rxmeg.Data[6];
        statusMsg.deviceVersion[2] = rxmeg.Data[7];
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_HIGH,1,&statusMsg);
        if(rxmeg.Data[4] == DEVICE_STATE_INIT)//设备初始化
        {
            ;
        }
    }
    return 1;
}

/*******************************************************************************************
*功能描述：角度±180切换
*入口参数：AngleValue取值：
*             角度值
*返回值：  角度值
*******************************************************************************************/
short GetAdjustAngleValue(uint16_t AngleValue)
{
    if ((AngleValue > 9000) && (AngleValue <= 18000))
    {
        AngleValue = 18000 - AngleValue;
    }
    else if ((AngleValue >= 0xB9B0) && (AngleValue < 0xDCD8))
    {
        AngleValue = ~(18000 + AngleValue) - 1;
    }

    return (short)AngleValue;
}

int Angle_ValueReport_Proc(void * pro, CANDATAFORM  rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    uint16_t data;
    if(rxmeg.Data[1] == 0 || rxmeg.Data[1] == 2 || rxmeg.Data[1] == 3 || rxmeg.Data[1] == 4 ) //0:普通倾角 2:动态倾角 3:声光倾角 4:角高一体
        {
            if(rxmeg.Data[1] ==3)  //声光倾角
            {

                //存角度xy值
                data = ((uint16_t)rxmeg.Data[2])|((uint16_t)rxmeg.Data[3]<<8);
                data = GetAdjustAngleValue(data);
//                zprintf3("angle_alaem_report_proc xzhou:%d \n",data/100);

                deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_ALARMANGLE,rxmeg.Data[0]+1,ANGLE_X,data);
                data = ((uint16_t)rxmeg.Data[4])|((uint16_t)rxmeg.Data[5]<<8);
                deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_ALARMANGLE,rxmeg.Data[0]+1,ANGLE_Y,data);
                if(rxmeg.DLC == 0x08)
                {
                    data = ((uint16_t)rxmeg.Data[6])|((uint16_t)rxmeg.Data[7]<<8);
                    deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_ALARMANGLE,rxmeg.Data[0]+1,ANGLE_Z,data);
                }

            }
            if(rxmeg.Data[1] ==4) //角高一体
            {
                //存角度xy值
                data = ((uint16_t)rxmeg.Data[2])|((uint16_t)rxmeg.Data[3]<<8);
                data = GetAdjustAngleValue(data);

//                zprintf3("angle_high_report_proc xzhou:%d \n",data/100);
                deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_HIGHANGLE,rxmeg.Data[0]+1,ANGLE_X,data);

                data = ((uint16_t)rxmeg.Data[4])|((uint16_t)rxmeg.Data[5]<<8);
                deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_HIGHANGLE,rxmeg.Data[0]+1,ANGLE_Y,data);
                if(rxmeg.DLC == 0x08)
                {
                    data = ((uint16_t)rxmeg.Data[6])|((uint16_t)rxmeg.Data[7]<<8);
                    deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_HIGHANGLE,rxmeg.Data[0]+1,ANGLE_Z,data);
                }
            }
            else  //普通倾角
            {
                //存角度xy值
                data = ((uint16_t)rxmeg.Data[2])|((uint16_t)rxmeg.Data[3]<<8);
                data = GetAdjustAngleValue(data);
//                zprintf3("angle_report_proc xzhou:%d \n",data/100);
                deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_ANGLE,rxmeg.Data[0]+1,ANGLE_X,data);
                data = ((uint16_t)rxmeg.Data[4])|((uint16_t)rxmeg.Data[5]<<8);
                deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_ANGLE,rxmeg.Data[0]+1,ANGLE_Y,data);
                if(rxmeg.DLC == 0x08)
                {
                    data = ((uint16_t)rxmeg.Data[6])|((uint16_t)rxmeg.Data[7]<<8);
                    deviceInfo.setDeviceAppProc_ANGLE(canp->canIndex,ID_ANGLE,rxmeg.Data[0]+1,ANGLE_Z,data);
                }
            }
        }
        else
        {
            data = rxmeg.Data[3];
            data = (data<<8)|(uint16_t)rxmeg.Data[2];
//            zprintf3("high_report_proc high:%d \n",data/10);
            deviceInfo.setDeviceAppProc_HIGH(canp->canIndex,ID_HIGH,rxmeg.Data[0]+1,data);
        }

    return 1;
}


int Alarm_Heart_Proc(void * pro, CANDATAFORM  rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    ncan_protocol *pros = (ncan_protocol*)canp->father_p;
    st_DeviceStatusMsg statusMsg;
    CANDATAFORM  angleTxmeg;
    memset(&statusMsg,0,sizeof(st_DeviceStatusMsg));
    if(rxmeg.Data[4] == DEVICE_STATE_INIT)
    {
        if(getSupprotNum() != 0)
        {
            Person_InitPara_Proc(&angleTxmeg,getSupprotNum(),getCheckDis_Person(),getReportInter_Person());//声光初始化
            canp->write_send_data(angleTxmeg);
        }
    }

    if(rxmeg.Data[3] == V2_ALARM_HEART || rxmeg.Data[3] == V2_ALARMBL_HEART)
    {
        if(rxmeg.Data[3] == V2_ALARM_HEART)
        {
            deviceInfo.getDeviceStateProc(canp->canIndex,ID_ALARM,1,&statusMsg);
            statusMsg.deviceVersion[0] = rxmeg.Data[5];
            statusMsg.deviceVersion[1] = rxmeg.Data[6];
            statusMsg.deviceVersion[2] = rxmeg.Data[7];
            statusMsg.isOnline = DEVICE_ONLINE;
        }
        else
        {
            statusMsg.deviceBLVersion[0] = rxmeg.Data[5];
            statusMsg.deviceBLVersion[1] = rxmeg.Data[6];
            statusMsg.deviceBLVersion[2] = rxmeg.Data[7];
            statusMsg.isOnline = DEVICE_ONLINE;
        }
        statusMsg.deviceStatus = rxmeg.Data[4];
        statusMsg.HeartTimer = ALARM_HEART_TMIER;
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_ALARM,1,&statusMsg);
//        zprintf3("alarm_heart_proc canp->canIndex:%d \n",canp->canIndex);
    }
    else if(rxmeg.Data[3]  == V2_PERSON_HEART)
    {
         deviceInfo.getDeviceStateProc(canp->canIndex,ID_PERSON,1,&statusMsg);

         statusMsg.deviceStatus = rxmeg.Data[4];
         statusMsg.isOnline = DEVICE_ONLINE;
         statusMsg.HeartTimer = ALARM_HEART_TMIER;
         statusMsg.deviceVersion[0] = rxmeg.Data[5];
         statusMsg.deviceVersion[1] = rxmeg.Data[6];
         statusMsg.deviceVersion[2] = rxmeg.Data[7];
         deviceInfo.setDeviceStateProc(canp->canIndex,ID_PERSON,1,&statusMsg);
//        zprintf3("person_heart_proc canp->canIndex:%d \n",canp->canIndex);
    }
    else if(rxmeg.Data[3] == V2_PERSONBL_HEART)
    {
        statusMsg.deviceStatus = rxmeg.Data[4];
        statusMsg.isOnline = DEVICE_ONLINE;
        statusMsg.HeartTimer = ALARM_HEART_TMIER;

        statusMsg.deviceBLVersion[0] = rxmeg.Data[5];
        statusMsg.deviceBLVersion[1] = rxmeg.Data[6];
        statusMsg.deviceBLVersion[2] = rxmeg.Data[7];
        deviceInfo.setDeviceStateProc(canp->canIndex,ID_PERSON,1,&statusMsg);
    }
    return 1;
}

int Alarm_Report_Proc(void * pro, CANDATAFORM  rxmeg)
{
    CanDriver * canp = (CanDriver *) pro;
    st_DeviceStatusMsg statusMsg;
    memset(&statusMsg,0,sizeof(st_DeviceStatusMsg));

    uint32_t data;
    uint8_t tmp,status,pos;
    uint8_t Tmp[12];


    Tmp[0] = rxmeg.ExtId & 0xff;
    Tmp[1] = (rxmeg.ExtId >> 8) & 0xff;
    Tmp[2] = (rxmeg.ExtId >> 16) & 0xff;
    Tmp[3] = (rxmeg.ExtId >> 24) & 0xff;
    Tmp[4] = rxmeg.DLC & 0xff;
    Tmp[5] = rxmeg.Data[0];
    Tmp[6] = rxmeg.Data[1];
    Tmp[7] = rxmeg.Data[2];
    Tmp[8] = rxmeg.Data[3];
    Tmp[9] = rxmeg.Data[4];
    Tmp[10] = rxmeg.Data[5];
    Tmp[11] = rxmeg.Data[6];
    tmp = CRC_8(Tmp, 12);

    if(tmp != rxmeg.Data[7])
        return 0;

    status = rxmeg.Data[1]&0x0F;

    data = ((uint16_t)rxmeg.Data[2])|((uint16_t)rxmeg.Data[3]<<8);  // 编号

    if(data<UWBDYKMIN)
        return 0;

    deviceInfo.setDeviceAppProc_PERSON(canp->canIndex,ID_PERSON,1,status,data,NULL);

    memset(Tmp,0,sizeof(Tmp));
    Tmp[0] = rxmeg.Data[2];
    Tmp[1] = rxmeg.Data[3];
    Tmp[2] = rxmeg.Data[4];
    Tmp[3] = rxmeg.Data[5];
    Tmp[4] = ((rxmeg.Data[1]>>4)&0x0F);
    Tmp[5] = 0;
    Tmp[6] = (rxmeg.Data[1]&0x0F);
    Tmp[7] = 0;
    Ydshrdlib::getInston()->sendShmDataFunc(1,55,((canp->canIndex<<5)|ID_PERSON),1,0,Tmp,8);

//    zprintf3("person_report_proc personID:%d \n",data);
    return 1;
}

//int Alarm_Send_Before_Proc(void *pro ,void * pro1, CANDATAFORM*  rxmeg)
//{
//    CANPROHEAD * canp1 = (CANPROHEAD *) pro1;
//    ncan_protocol * canp = (ncan_protocol *) pro;
//    canp->pro_del_buf_frame(*rxmeg);
//    if(rxmeg->Data[0] == 0 && rxmeg->Data[1] == 0 && rxmeg->Data[2] == 0 && rxmeg->Data[3] == 0)
//    {#include "fileload.h"
//        canp1->repeatnum = 0x03;
//    }
//    return 1;
//}



int Alarm_SetPara_Proc(void * pro, CANDATAFORM  rxmeg)
{
    return 1;
}


int Alarm_Reset_Proc(uint8_t device, CANDATAFORM* rxmeg)
{
    V2_CAN_ID v2id;
    CANDATAFORM  canFrame;

    v2id.Reservd = 0;
    v2id.ACK = 0;
    v2id.FrameType = 6;
    v2id.Sub = 0;
    v2id.Sum = 0;
    v2id.RxID = V2ID_ALARM;
    v2id.TxID = 0;
    v2id.NoUsed = 0;
    v2id.LiuShuiNumb = 0;

    memcpy(&canFrame.ExtId,&v2id,sizeof(V2_CAN_ID));
    if(device == ID_ALARM)
        canFrame.Data[0] = 0x30;
    else if(device == ID_PERSON)
        canFrame.Data[0] = 0x31;
    canFrame.DLC = 0x01;

    canFrame.IDE = 1;
    canFrame.RTR = 0;

    *rxmeg = canFrame;
    return 1;
}


uint32_t ActCmdTransPro(uint8_t type,uint32_t CommandWord)
{
    uint32_t res = 0XFF;
    if(type == 2)
    {
        if (ActCmdTransTbl_V26[CommandWord - 1] != -1)
                res = (1 << ActCmdTransTbl_V26[CommandWord - 1]);
    }
    else if(type == 1 /*&& temp.deviceVersion[1] != 0*/)
    {
        if (ActCmdTransTbl_V22[CommandWord - 1] != -1)
                res = (1 << ActCmdTransTbl_V22[CommandWord - 1]);
    }
    return res;
}


int UpdateProgram_Proc(uint8_t device, uint8_t ft, uint32_t pakNum,uint8_t * data, uint8_t len,CANDATAFORM*  rxmeg)
{
    V2_CXBCAN_ID v2id;
    CANDATAFORM  canFrame;


    uint8_t tmpdevice = deviceToV2type(device);

    v2id.FrameType = ft;
    v2id.ACK = 0;
    v2id.NoUsed = 0;
    v2id.PacketNum = pakNum;
    v2id.Reservd = 0x01;
    v2id.RxID = tmpdevice;

    canFrame.DLC = len;
    memcpy(&canFrame.Data,data,len);
    memcpy(&canFrame.ExtId,&v2id,sizeof(V2_CXBCAN_ID));

    canFrame.IDE = 1;
    canFrame.RTR = 0;

    *rxmeg = canFrame;

    return 0;
}


int Emvd_Controll_Proc(uint8_t ft, uint32_t ioValue, uint8_t type, CANDATAFORM*  rxmeg)
{
    V2_CAN_ID v2id;
    uint32_t emvdAction = 0;
    uint8_t tmp[9];
    CANDATAFORM  canFrame;

    if(ft == 1)
    {
        for(int i = 0;i<32;i++)
        {
            if((ioValue&(1<<i)) != 0)
            {
                emvdAction |= ActCmdTransPro(type,i);
            }
        }
    }
    else if(ft == 2)
        emvdAction |= ActCmdTransPro(type,ioValue);
    else
        return 0;
    v2id.ACK = 0;
    v2id.FrameType = ft;
    v2id.LiuShuiNumb = 0;
    v2id.NoUsed = 0;
    v2id.Reservd = 0;
    v2id.RxID = V2ID_EMVD;
    v2id.Sub = 0;
    v2id.Sum = 0;
    v2id.TxID = ID_SC;

    canFrame.Data[0] = (emvdAction &0xFF);
    canFrame.Data[1] = ((emvdAction &0xFF00)>>8);
    canFrame.Data[2] = ((emvdAction &0xFF0000)>>16);
    canFrame.Data[3] = ((emvdAction &0xFF000000)>>24);
    canFrame.DLC = 5;

    memcpy(&canFrame.ExtId,&v2id,sizeof(V2_CAN_ID));

    tmp[0] = canFrame.ExtId&0xFF;
    tmp[1] = (canFrame.ExtId>>8)&0xFF;
    tmp[2] = (canFrame.ExtId>>16)&0xFF;
    tmp[3] = (canFrame.ExtId>>24)&0xFF;
    tmp[4] =  canFrame.DLC&0xFF;
    memcpy(&tmp[5],&canFrame.Data[0],4);

    canFrame.Data[4] = CRC_8(tmp, 9);
    canFrame.IDE = 1;
    canFrame.RTR = 0;
    *rxmeg = canFrame;
//    memcpy(rxmeg,canFrame,sizeof(CANDATAFORM));

    return 0;
}



int Alarm_Controll_Proc(uint8_t type, CANDATAFORM*  rxmeg)
{
    V2_CAN_ID v2id;
    uint8_t tmp[9];
    CANDATAFORM  canFrame;

    v2id.ACK = 0;
    if(type == RED_ALWAYS)
        v2id.FrameType = 4;
    else if(type == RED20_BLUE20)
        v2id.FrameType = 5;
    else
        return -1;

    v2id.LiuShuiNumb = 0;
    v2id.NoUsed = 0;
    v2id.Reservd = 0;
    v2id.RxID = V2ID_ALARM;
    v2id.Sub = 0;
    v2id.Sum = 0;
    v2id.TxID = ID_SC;


    memcpy(&canFrame.ExtId,&v2id,sizeof(V2_CAN_ID));

    if(v2id.FrameType == 4)
    {
        canFrame.Data[1] = 0x01;
        canFrame.DLC = 5;
        canFrame.Data[0] = 0x01;		//0x01;

        canFrame.Data[2] = (uint8_t)((0x02030 >> 8) & 0xFF);
        canFrame.Data[3] = (uint8_t)(0x02030 & 0xFF);

        tmp[0] = canFrame.ExtId & 0xff;
        tmp[1] = (canFrame.ExtId>>8) & 0xff;
        tmp[2] = (canFrame.ExtId>>16) & 0xff;
        tmp[3] = (canFrame.ExtId>>24) & 0xff;
        tmp[4] = canFrame.DLC & 0xff;
        tmp[5] = canFrame.Data[0];
        tmp[6] = canFrame.Data[1];
        tmp[7] = canFrame.Data[2];
        tmp[8] = canFrame.Data[3];

        canFrame.Data[4] = CRC_8(tmp, 9);
    }
    else if(v2id.FrameType == 5)
    {
        canFrame.Data[1] = 0x02;
        canFrame.DLC = 5;
        canFrame.Data[0] = 0x01;		//0x01;

        canFrame.Data[2] = (uint8_t)((0x02030 >> 8) & 0xFF);
        canFrame.Data[3] = (uint8_t)(0x02030 & 0xFF);

        tmp[0] = canFrame.ExtId & 0xff;
        tmp[1] = (canFrame.ExtId>>8) & 0xff;
        tmp[2] = (canFrame.ExtId>>16) & 0xff;
        tmp[3] = (canFrame.ExtId>>24) & 0xff;
        tmp[4] = canFrame.DLC & 0xff;
        tmp[5] = canFrame.Data[0];
        tmp[6] = canFrame.Data[1];
        tmp[7] = canFrame.Data[2];
        tmp[8] = canFrame.Data[3];

        canFrame.Data[4] = CRC_8(tmp, 9);
    }
    canFrame.RTR = 0;
    canFrame.IDE = 1;
    *rxmeg = canFrame;
    return 0;
}








#endif
