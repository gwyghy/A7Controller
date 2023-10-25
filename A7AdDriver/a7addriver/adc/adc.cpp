#include "adc.h"
#include "include.h"

#define AI8_DATA_PATH "/sys/devices/soc0/soc/2100000.aips-bus/2198000.adc/iio:device0/in_voltage8_raw"
#define AI9_DATA_PATH "/sys/devices/soc0/soc/2100000.aips-bus/2198000.adc/iio:device0/in_voltage9_raw"

#define SAMP_COUNT 10

int vol_fd = -1;
int cur_fd = -1;

void Adcfd_init(void)
{
    vol_fd = open(AI9_DATA_PATH, O_RDONLY);
    cur_fd = open(AI8_DATA_PATH, O_RDONLY);
}

/*
*********************************************************************************************************
*	函 数 名: AdcPro
*	功能说明: ADC采样处理，插入1ms systick 中断进行调用
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

short AdcValue[2][10] = {0};
char  AdcCalcCount[2] = {0};
unsigned long  adctmpvalue = 0;
short AdcPro(char adcNum,short * value)
{
    short res=0,iTemp = 0,iPos;
    char i,j;
    switch(adcNum)
    {
        case adc_vol:
        case adc_cur:
            AdcValue[adcNum][AdcCalcCount[adcNum]] = read_adc(adcNum);
            AdcCalcCount[adcNum]++;
        break;
         default:
            break;
    }
    if(AdcCalcCount[adcNum] == 9)
    {
        // 从小到大排序
        for(i=0; i<9; i++)                              // 设置外层循环为下标 0~8 的元素
        {
            iTemp = AdcValue[adcNum][i];            // 设置当前元素的最小值
            iPos = i;                                   // 记录当前元素的位置
            for(j=i+1; j<10; j++)                       // 设置内层循环为 i+1 ~ 9
            {
                if(AdcValue[adcNum][j] < iTemp)                        // 如果当前元素比最小值还小
                {
                    iTemp = AdcValue[adcNum][j];                       // 重新设置最小值
                    iPos = j;                           // 记录新的元素位置
                }
            }
            // 交换两个元素值
            AdcValue[adcNum][iPos] = AdcValue[adcNum][i];
            AdcValue[adcNum][i] = iTemp;

        }
        adctmpvalue = 0;
        for(i=1; i<9; i++)
            adctmpvalue += AdcValue[adcNum][i];
        adctmpvalue = adctmpvalue / 8 ;
        AdcCalcCount[adcNum] = 0;
        *value = adctmpvalue;
//      printf("voltage_cur=%d\n", adctmpvalue);
        res = 1;
    }
    return res;
}

// 读取ADC值，返回电压值（单位：伏特）
short read_adc(short adcNum)
{
    char fd;
    short ret;
    char value[20];
    short voltage;
    if(adcNum == adc_vol)
    {
       off_t offset =lseek(vol_fd,0,SEEK_SET);
       ret = read(vol_fd, value, sizeof(value));
       // 读取ADC的值
       if (ret < 0)
       {
           perror("Failed to read from ADC");
           exit(EXIT_FAILURE);
       }
       voltage = atof(value);
       voltage = (voltage*33000/4096+650)/10;
       // 将读取到的数字转换为电压值
       // printf("voltage_vol=%d\n", voltage);
    }
    else
    {
       off_t offset =lseek(cur_fd,0,SEEK_SET);
       ret = read(cur_fd, value, sizeof(value));
       // 读取ADC的值
       if (ret < 0)
       {
           perror("Failed to read from ADC");
           exit(EXIT_FAILURE);
       }
       voltage = atof(value);
       voltage = voltage*3300/16384;
       //printf("voltage_cur=%d\n", voltage);
    }
    return voltage;
}
