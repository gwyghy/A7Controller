#ifndef V4HYSAPP_H
#define V4HYSAPP_H

/******/
#include "v4hysapptype.h"

/********/
#define SC_PARAM_MENU_FIRST_DEFAULT_SET      0x0100000000000000//对控制器参数中，特殊处理的地方

/******/
#define SC_PARAM_MENU_GLOBAL_SUPPORT_SET     0x0101010000000000//对控制器参数中，特殊处理的地方
#define SC_PARAM_CURRENT_SC_ID_LINE_SET      0x01//本架编号的行号

/*****/
#define SYSTEM_PARAM_MENU_IP_ADDR_SET       0x0300000000000000//对控制器参数中，特殊处理的地方
#define SYSTEM_PARAM_IP_ADDR1_LINE_SET       0x02//IP地址中字段1的行号
#define SYSTEM_PARAM_IP_ADDR2_LINE_SET       0x03//IP地址中字段2的行号
#define SYSTEM_PARAM_IP_ADDR3_LINE_SET       0x04//IP地址中字段3的行号
#define SYSTEM_PARAM_IP_ADDR4_LINE_SET       0x05//IP地址中字段4的行号

#define SYSTEM_PARAM_IP_VLAN_LEFT_LINE_SET       14//VLAN_LEFT的行号
#define SYSTEM_PARAM_IP_VLAN_RIGHT_LINE_SET       15//VLAN_RIGHT的行号

/*****/
#define SYSTEM_PARAM_MENU_DEV_LEFT_SET       0x0600000000000000//对控制器参数中，特殊处理的地方
#define SYSTEM_PARAM_MENU_DEV_RIGHT_SET       0x0500000000000000//对控制器参数中，特殊处理的地方

/******************设备控制*************************/
/******/
int FrameGetSingleInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameGetMultiInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

/******/
int FrameCtrlDevProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameManulHeartBeatProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameStatusReportProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
/******/
int FrameDownloadSysStatusProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameFollowOnResponseProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameRecvYKQDecodeProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

/******/
//int FrameDownloadAutoBackFlushOnOffProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameDownloadAutoBackFlushStatusProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameTranDevStatusProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameTerminalInfReportProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

/***************参数修改****************************/
int FrameParamGetSingleResponseProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameParamGetMultiResponseProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameParamSetSingleInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameParamSetMultiInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameParamCheckInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameParamNoticeProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);//参数修改上报
int FrameParamServerOperateRequestProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameParamSSDisplayResponseProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameParamReportParamChangedProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameParamReportParamChangedAckProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameParamSetLascDataProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameParamSystemParamInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

int FrameParamReportSystemParamChangedProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

/***************网络维护****************************/
int FrameForceEnterSetIpStatusProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameSetDevIpProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameNetHeartBeatProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameNeighbourCommCheckProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);
int FrameNetRsyncFileInquireProc(uchar childdevtype, uchar devtype, ushort numb, ushort frametype, u_int8_t *data, int len);

/**********************参数修改时，运行参数的同步***********/
int DevNetMsgRunParamInit(void);
/**与udp同步参数，scid为此时的控制器架号***/
int DevNetMsgRunParamInit(u_int16_t scid);
#endif // V4HYSAPP_H
