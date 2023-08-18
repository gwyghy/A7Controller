/*
*********************************************************************************************************
*	                                            V4
*                           Commuication Controller For The Mining Industry
*
*                                  (c) Copyright 1994-2013  HNDZ
*                       All rights reserved.  Protected by international copyright laws.
*
*
*    File    : Bsp_Time.h
*    Module  : user
*    Version : V1.0
*    History :
*   -----------------
*              Version  Date           By            Note
*              v1.0
*
*********************************************************************************************************
*/
#ifndef CRC_CCITT
#define CRC_CCITT

unsigned char	CRC_8(unsigned char *PData, unsigned char Len);
void Crc16Ccitt( char *u8Buf, unsigned int u32Len, unsigned short *u16CheckOld);
#endif // CRC_CCITT

