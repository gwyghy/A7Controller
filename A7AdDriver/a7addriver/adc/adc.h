#ifndef ADC_H
#define ADC_H
#include "include.h"


#define adc_vol  0
#define adc_cur  1


short  read_adc(short adcNum);
short  AdcPro(char adcNum,short * value);
void   Adcfd_init(void);

#endif // ADC_H
