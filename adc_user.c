
#include "adc.h"  
#include "sau.h"

//AD采样(10位/1024):单次选择等待转化完成,非中断方式
u16 AdcSampSingleChannelOnce(u8 eChannel)
{
   ADS = eChannel;
   ADIF = 0;       
   ADCS = 1;       
   while(!ADIF);
   ADIF = 0;
   ADCS = 0;      

   return(ADCR>>6); 
}








