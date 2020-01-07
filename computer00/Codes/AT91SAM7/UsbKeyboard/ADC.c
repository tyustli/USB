/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org
          
ADC.c  file
        
作者：Computer-lov
建立日期: 2009-03-06
修改日期: 2009-03-06
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <AT91SAM7S64.H>
#include "ADC.h"

/********************************************************************
函数功能：ADC初始化函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void AdcInit(void)
{
 AdcReset();
 *AT91C_ADC_MR=(0<<0)  //Hardware trigger selected by TRGSEL field is disabled.
              |(0<<1)  //TIOA Ouput of the Timer Counter Channel 0
              |(0<<4)  //10-bit resolution
              |(0<<5)  //Normal Mode
              |(4<<8)  //Prescaler Rate Selection, ADCClock = MCK/10 = 4.8MHz
              |(24<<16) //Startup Time=(24+1)*8/ADCClock is about 20us.
              |(5<<24);  //Sample & Hold Time = 5/ADCClock is about 1us.
              
 *AT91C_ADC_CHDR=0xFF; //Disable all channel.             
 *AT91C_ADC_CHER=(1<<4);  //Enable CH4, as key input.
 *AT91C_ADC_IDR=0xFFFFF;  //Disable all ADC interrupts.
 AdcStartConversion(); //软件启动一次转换
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：软件转换并读取一次ADC。
入口参数：ch：通道号。
返    回：ADC值。
备    注：无。
********************************************************************/
int AdcRead(void)
{
 AdcStartConversion(); //软件启动转换
 while(!((*AT91C_ADC_SR)&(1<<16)));  //等待转换完毕
 return *AT91C_ADC_LCDR;  //读取ADC值
}
/////////////////////////End of function/////////////////////////////


/********************************************************************
函数功能：设置DMA。
入口参数：Addr: 接收数据的缓冲区地址；Count：要接收的数量。
返    回：无。
备    注：无。
********************************************************************/
void AdcSetupDma(short int *Addr, int Count)
{
 *AT91C_ADC_RPR=(unsigned int)Addr;   //设置地址
 *AT91C_ADC_RCR=Count;  //设置数量
 *AT91C_ADC_PTCR=(1<<0);  //启动接收
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：等待ADC的DMA传输完成。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void AdcWaitDmaFinish(void)
{
 while((*AT91C_ADC_RCR)!=0); //当未接收完时，等待
}
/////////////////////////End of function/////////////////////////////
