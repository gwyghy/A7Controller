#ifndef __DYK_V2CALLBACK_H__
#define __DYK_V2CALLBACK_H__

#include "can_bus.h"
#include "can_device_app.h"


int Emvd_Heart_Proc(void * pro, CANDATAFORM  rxmeg);
int Emvd_Check_Proc(void * pro, CANDATAFORM  rxmeg);
int Emvd_Send_Before_Proc(void *pro ,void * pro1, CANDATAFORM*  rxmeg);
int Hub_Heart_Proc(void * pro, CANDATAFORM rxmeg);
int Hub1_ReportAnalog_Proc(void * pro, CANDATAFORM rxmeg);
int Hub1_ReportAngleXY_Proc(void * pro, CANDATAFORM rxmeg);
int Hub1_ReportAngleZ_Proc(void * pro, CANDATAFORM rxmeg);
int Hub2_ReportAnalog_Proc(void * pro, CANDATAFORM rxmeg);
int Hub2_ReportAngleXY_Proc(void * pro, CANDATAFORM rxmeg);
int Hub2_ReportAngleZ_Proc(void * pro, CANDATAFORM rxmeg);
int Hub3_ReportAnalog_Proc(void * pro, CANDATAFORM rxmeg);
int Hub3_ReportAngleXY_Proc(void * pro, CANDATAFORM rxmeg);
int Hub3_ReportAngleZ_Proc(void * pro, CANDATAFORM rxmeg);
int WL_Heart_Proc(void * pro, CANDATAFORM  rxmeg);
int WL_CoalLocal_Proc(void * pro, CANDATAFORM  rxmeg);
int WL_WirelessCodeMatch_proc(void * pro,CANDATAFORM rxmeg);
int WL_WirelessDeCodeMatch_proc(void * pro,CANDATAFORM rxmeg);
int WL_WirelessControllKeyDown_proc(void * pro,CANDATAFORM rxmeg);
int WL_WirelessNeighboorCtrlReq_proc(void * pro,CANDATAFORM rxmeg);
int CXB_Heart_Proc(void * pro, CANDATAFORM rxmeg);
int CXB_RecvProgram_Proc(void * pro, CANDATAFORM rxmeg);
int CXB_RecvCrc_Proc(void * pro, CANDATAFORM rxmeg);
int Angle_Heart_Proc(void * pro, CANDATAFORM  rxmeg);
int Angle_ValueReport_Proc(void * pro, CANDATAFORM  rxmeg);
int Alarm_Heart_Proc(void * pro, CANDATAFORM  rxmeg);
int Alarm_Report_Proc(void * pro, CANDATAFORM  rxmeg);
int Alarm_SetPara_Proc(void * pro, CANDATAFORM  rxmeg);
int Alarm_Reset_Proc(uint8_t device, CANDATAFORM* rxmeg);
int Emvd_Controll_Proc(uint8_t ft,uint32_t ioValue, uint8_t type, CANDATAFORM*  rxmeg);
int Alarm_Controll_Proc(uint8_t type, CANDATAFORM*  rxmeg);
int Wireless_InitPara_Proc(CANDATAFORM * value);
int Person_InitPara_Proc(CANDATAFORM * value,uint16_t scNum,uint16_t dis,uint16_t timer);
int UpdateProgram_Proc(uint8_t device, uint8_t ft, uint32_t pakNum,uint8_t * data, uint8_t len,CANDATAFORM*  rxmeg);


#endif
