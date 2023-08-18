#ifndef HYSAPPMSG_H
#define HYSAPPMSG_H

/******/
#include "appmsgtype.h"

/***************数据采集控制相关，1-100***************/
int HysAppMsgRxCtrlDevProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxManualHeatBeatProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxFollowOnInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxAutoBackFlushOnInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxKeyLedStatusCtrlProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxEndKeyStatusProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);

/**************参数传输相关，101-200***************/
int HysAppMsgRxDisplayRunParamInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxReportRunParamChangedProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxDisplaySystemParamInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxReportSystemParamChangedProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);

/*************网络维护及文件传输相关，201-300.以下为自定义*****************/
int HysAppMsgRxServiceOperateInfProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxDownloadDevPrgCxbProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxDownloadDevPrgRemoteProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgRxUpdateDevPrgProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);

int HysAppMsgCheckCommAreaInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgCheckDataTransInfInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
int HysAppMsgCheckEmvdPointStatusInquireProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);
/**************数据发送相关*******************/
/**服务器参数的新修改***/
int HysAppMsgSendSSDisplayResponseProc(u_int16_t devtype, u_int16_t numb, u_int8_t *data, u_int16_t len);

/*******/
int HysAppMsgSendServiceParamChanedProc(void);//上报服务类参数修改(含密码)
int HysAppMsgSendKeyConfigParamChanedProc(void);//上报按键配置参数修改
int HysAppMsgSendAssintConfigParamChanedProc(void);//上报辅助菜单参数修改
int HysAppMsgSendHomeDisplayParamChanedProc(void);//上报主界面显示参数修改
int HysAppMsgSendGrpKeyConfParamChanedProc(void);//上报成组配置菜单参数修改
int HysAppMsgSendSingleActParamChanedProc(void);//上报单动作中，动作参数的修改
int HysAppMsgSendCommbActParamChanedProc(void);//上报组合动作中，动作参数的修改
int HysAppMsgSendGrpActParamChanedProc(void);//上报成组动作中，动作参数的修改

#endif // HYSAPPMSG_H
