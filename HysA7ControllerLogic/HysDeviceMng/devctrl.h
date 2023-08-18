#ifndef V4HYSAPPPROTOCAL_H
#define V4HYSAPPPROTOCAL_H

/*******/
#include "devtype.h"

int DevCtrlInit(void);

/***控制设备请求(非电话,非网络),触发方式****//***控制设备的动作，对应包类型为7***/
int DevCtrlInquire(u_int16_t driverid,u_int16_t devid,\
                   u_int16_t childdevid,u_int16_t point,u_int8_t *pbuf,\
                   int length);

/***获取驱动所对应程序编号以及硬件设备的编号***/
int DevCtrlGetProgramAndHardWareInf(u_int16_t driverid,uchar &programid,uchar &hardwareid);

/***控制设备命令(非电话,非网络),持续发送方式****//***控制设备的动作，对应包类型为7***/
int DevCtrlContinue(u_int16_t scid, u_int16_t driverid,u_int16_t devid,\
                    u_int16_t childdevid,u_int16_t point,void *pbuf,\
                    int length);

/***控制电话的请求，预留功能***/
int DevCtrlPhoneInquire(u_int16_t phoneAddr,void *pdata,u_int16_t length);




/*!
 * \brief UdpAppProcCtrl,发送应用层数据
 * \param childdevid，子设备编号，此处为设备类型
 * \param numb，设备编号
 * \param frametype，包标识符
 * \param pdata，数据指针
 * \param length，数据长度
 * \return 0正常负数异常
 */
int UdpAppProcCtrl(uchar childdevid,uchar numb,\
                  u16 frametype,u_int8_t *pdata,u16 length);

#endif // V4HYSAPPPROTOCAL_H
