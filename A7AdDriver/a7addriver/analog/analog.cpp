#include "analog.h"
#include "include.h"

#define DEV_AD_SPI_CHANNEL_MAX  4
#define DEV_AD_CHANNEL_MAX		6

int fileDescriptor;  //mcp3204 spi句柄
MCP3204 ad_MCP3204;

short DevmngAD_value[DEV_AD_CHANNEL_MAX];

void * Analog_proc(void *parg)
{
      u8 i,res,j,data[2];
      short sensorvalue,adcvalue;
      char error[55];

      Adcfd_init();
      memset(DevmngAD_value,0,sizeof(DevmngAD_value));

      Ydshrdlib::getInston()->InitLibParam(41,87654333,87654332);

      if (MCP3204_init(&fileDescriptor,"/dev/spidev0.0",&ad_MCP3204,mode_SPI_00,2.5,error))
      {
          printf("Cannot initialize the MCP3204 ADC.\n");
          printf("%s\n",error);
          exit(1);
      }
      while(1)
      {
          for(i=0;i<DEV_AD_SPI_CHANNEL_MAX;i++)
          {
              res = MCP3204_Value(i,&sensorvalue);
              if(res)
              {
                  DevmngAD_value[i] = sensorvalue;
                  data[0] =  DevmngAD_value[i] & 0xFF;        // 低位字节
                  data[1] = (DevmngAD_value[i] >> 8) & 0xFF;  // 高位字节
//                printf("sensorvalue = %d\n",sensorvalue);
                  Ydshrdlib::getInston()->sendShmDataFunc(1,55,1<<5|ID_SC_ANALOG,1,i+1,data,2);
              }
          }
          for(i=0;i<2;i++)
          {
              res  = AdcPro(i,&adcvalue);
              if(res)
              {
                  DevmngAD_value[DEV_AD_SPI_CHANNEL_MAX+i] = adcvalue;
                  data[0] =  DevmngAD_value[DEV_AD_SPI_CHANNEL_MAX+i] & 0xFF;        // 低位字节
                  data[1] = (DevmngAD_value[DEV_AD_SPI_CHANNEL_MAX+i] >> 8) & 0xFF;  // 高位字节
//                printf("adcvalue = %d\n",adcvalue);
                  Ydshrdlib::getInston()->sendShmDataFunc(1,55,1<<5|ID_SC_ANALOG,1,i+5,data,2);
              }
          }
          usleep(10000);
      }
}
//输入 chanl：通道号0-3
short Calc_MCP3204_Value(char chanl)
{
    short data;
    char error[50];
    if (MCP3204_convert(fileDescriptor,singleEnded,(inputChannel)chanl,&ad_MCP3204,error))
    {
        printf("Error during conversion.\n");
        printf("%s\n",error);
        exit(1);
    }
    data = MCP3204_getValue(ad_MCP3204);

    return data;
}
/***********************************************************
* 函数名称：MCP3204_Value
* 功能描述：
* 入口参数：
* 出口参数：无
* 使用说明：无
************************************************************/
short sensorValue[4][10] = {0};
char sensorCalcCount[4] = {0};
short sensortmpvalue = 0;
short MCP3204_Value(char channel, short *value)
{
    short res=0,iTemp = 0,iPos;
    char i,j;
    switch(channel)
    {
        case Analog1:
        case Analog2:
        case Analog3:
        case Analog4:
              sensorValue[channel][sensorCalcCount[channel]] = Calc_MCP3204_Value(channel);
              sensorCalcCount[channel]++;
        break;
        default:
        break;
    }
    if(sensorCalcCount[channel] == 9)
    {
        // 从小到大排序
        for(i=0; i<9; i++)                              // 设置外层循环为下标 0~8 的元素
        {
            iTemp = sensorValue[channel][i];            // 设置当前元素的最小值
            iPos = i;                                   // 记录当前元素的位置
            for(j=i+1; j<10; j++)                       // 设置内层循环为 i+1 ~ 9
            {
                if(sensorValue[channel][j] < iTemp)                        // 如果当前元素比最小值还小
                {
                    iTemp = sensorValue[channel][j];                       // 重新设置最小值
                    iPos = j;                           // 记录新的元素位置
                }
            }
            // 交换两个元素值
            sensorValue[channel][iPos] = sensorValue[channel][i];
            sensorValue[channel][i] = iTemp;

        }
        sensortmpvalue = 0;
        for(i=1; i<9; i++)
            sensortmpvalue += sensorValue[channel][i];
        sensortmpvalue = sensortmpvalue / 8 * PRESS_COEF;
        sensorCalcCount[channel] = 0;
        *value = sensortmpvalue;
//      printf("value = %d\n",*value);
        res = 1;
    }
    return res;
}

