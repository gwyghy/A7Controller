#ifndef ANALOG_H
#define ANALOG_H
#include "include.h"

#define Analog1          0x00   //第一路模拟量输入
#define Analog2          0x01   //第二路模拟量输入
#define Analog3			 0x02   //第三路模拟量输入
#define Analog4		     0x03	//第四路模拟量输入

#define PRESS_COEF  1000/819

//设备RID,TID
enum
{
    ID_SC,
    ID_EMVD = 1,						//电磁阀驱动器1
    ID_CXB = 2,							//程序棒2
    ID_ANGLE = 3,						//角度传感器5
    ID_HIGH = 4,						//高度4
    ID_ALARM = 5,						//声光报警器6
    ID_PERSON = 6,                      //人员7
    ID_IRRX = 7,						//红外接收
    ID_HIGHANGLE = 13,                  //角高一体
    ID_ALARMANGLE= 14,                  //多功能声光
    ID_SC_ANALOG =15,                   //本架模拟量
    ID_SC_IO  =16,                      // 本架IO

    ID_MINIHUB1,
    ID_MINIHUB2,
    ID_MINIHUB3,
    ID_DEVICEMAX
};

void * Analog_proc(void *parg);
short Calc_MCP3204_Value(char chanl);
short MCP3204_Value(char channel, short *value);
#endif // ANALOG_H
