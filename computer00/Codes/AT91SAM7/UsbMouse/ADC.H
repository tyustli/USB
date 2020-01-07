/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
    欢迎访问我的USB专区：http://group.ednchina.com/93/
    欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                         http://computer00.21ic.org
          
ADC.h  file
         
作者：Computer-lov
建立日期: 2009-03-06
修改日期: 2009-03-06
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#ifndef __ADC_H__
#define __ADC_H__

#define AdcReset()            *AT91C_ADC_CR=1
#define AdcStartConversion()  *AT91C_ADC_CR=(1<<1)

void AdcInit(void);
int AdcRead(void);
void AdcSetupDma(short int *Addr, int Count);
void AdcWaitDmaFinish(void);

#endif