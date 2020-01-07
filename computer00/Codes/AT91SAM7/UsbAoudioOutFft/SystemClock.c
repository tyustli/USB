/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

SystemClock.c  file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-03-04
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <AT91SAM7S64.H>

/********************************************************************
函数功能：系统时钟初始化。
入口参数：无。
返    回：无。
备    注：晶体频率18.432MHz。DIV=14，MUL=72，PLL输出频率为96.1MHz。
********************************************************************/
void SystemClockInit(void)
{
 *AT91C_PMC_IDR = 0xFFFFFFFF;  //disable all PMC interrupt
 
 *AT91C_PMC_SCER = (1<<0)   //Processor Clock Enable
                  |(1<<7);  //USB Device Port Clock Enable
                  
 *AT91C_CKGR_MOR = (1<<0)   //Main Oscillator Enable
                  |(0<<1)   //Oscillator is NOT Bypassed
                  |(0xFF<<8);  //Main Oscillator Start-up Time(255)
 
 while(!((*AT91C_PMC_SR)&0x01));  //wait for MOSCS to be set
 
 *AT91C_CKGR_PLLR = (0x0E<<0)  //DIV
                   |(0x3F<<8)  //PLL COUNT
                   |(0x00<<14)  //OUT
                   |(0x48<<16)  //MUL
                   |(0x01<<28); //USB DIV
 
 while(!((*AT91C_PMC_SR)&(0x01<<2)));  //wait for LOCK to be set
 while(!((*AT91C_PMC_SR)&(0x01<<3)));  //wait for MCKRDY to be set
 
 *AT91C_PMC_MCKR = (0<<0)  //Slow Clock is selected 
                  |(1<<2); //Selected clock divided by 2
                  
 while(!((*AT91C_PMC_SR)&(0x01<<3)));  //wait for MCKRDY to be set
 
 *AT91C_PMC_MCKR  |= (0x03<<0);  //PLL Clock is selected.
                  
 while(!((*AT91C_PMC_SR)&(0x01<<3)));  //wait for MCKRDY to be set
 
 *AT91C_PMC_PCER =0xFFFFFFFF;  //enable all Peripheral Clock
}
/////////////////////////End of function/////////////////////////////
