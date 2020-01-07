/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org
          
Timer.c  file
         
作者：Computer-lov
建立日期: 2009-03-06
修改日期: 2009-03-06
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <AT91SAM7S64.H>
#include "timer.h"
#include "key.h"
#include "config.h"

/********************************************************************
函数功能：定时器0初始化。
入口参数：无。
返    回：无。
备    注：产生一个44.1KHz的
********************************************************************
void Timer0Init(void)
{
 *AT91C_TC0_IDR=0xFF;  //Disable interrupts
 
 *AT91C_TC0_CCR=(1<<0)   //Counter Clock Enable
               |(0<<1)
               |(1<<2);  //the counter is reset and the clock is started.
               
 *AT91C_TC0_CMR=(0<<0)  //TIMER_CLOCK1 (MCK/2=48.05M/2=24.025M
               |(0<<3)  //Counter is incremented on rising edge of the clock.
               |(0<<4)  //The clock is not gated by an external signal.
               |(0<<6)  //Counter clock is not stopped when counter reaches RC.
               |(0<<7)  //Counter clock is not disabled when counter reaches RC.
               |(0<<8)  //External Event Edge Selection: none
               |(0<<10)  //External Event Selection: TIOB
               |(0<<12)  //The external event has no effect on the counter and its clock.
               |(2<<13)  //UP mode with automatic trigger on RC Compare
               |(1<<15)  //Waveform Mode is enabled.
               |(1<<16)  //RA Compare Effect on TIOA: set
               |(2<<18)  //RC Compare Effect on TIOA: clear
               |(0<<20)  //External Event Effect on TIOA: none
               |(0<<22)  //Software Trigger Effect on TIOA: none
               |(0<<24)  //RB Compare Effect on TIOB: none
               |(0<<26)  //RC Compare Effect on TIOB: none
               |(0<<28)  //External Event Effect on TIOB: none
               |(0<<30); //Software Trigger Effect on TIOB: none
               
 //频率为44.1KHz
 *AT91C_TC0_RC=(Fmck/2)/44100;
 *AT91C_TC0_RA=*AT91C_TC0_RC/2; //产生50%占空比的方波
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：定时器1中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void Timer1Isr(void) __irq
{
 if(*AT91C_AIC_ISR!=13)return; //不是该中断源，直接返回
 //*AT91C_AIC_ICCR=1<<13; //使用了向量中断，会自动清除中断，这里不用
 //清除定时器状态标志
 if(!((*AT91C_TC1_SR)&(1<<4)))return; //不是RC比较中断，返回
 KeyScan();  //按键扫描
 *AT91C_AIC_EOICR=0;    //中断处理完毕
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：定时器1初始化。
入口参数：无。
返    回：无。
备    注：产生一个周期为5ms的定时中断信号。
********************************************************************/
void Timer1Init(void)
{
 *AT91C_TC1_IDR=0xFF;   //Disable interrupts
 *AT91C_TC1_IER=(1<<4); //Enables the RC Compare Interrupt.
 
 *AT91C_TC1_CCR=(1<<0)   //Counter Clock Enable
               |(0<<1)
               |(1<<2);  //the counter is reset and the clock is started.
               
 *AT91C_TC1_CMR=(2<<0)  //TIMER_CLOCK3 (MCK/32=48.05M/32 is about 1.5M)
               |(0<<3)  //Counter is incremented on rising edge of the clock.
               |(0<<4)  //The clock is not gated by an external signal.
               |(0<<6)  //Counter clock is not stopped when counter reaches RC.
               |(0<<7)  //Counter clock is not disabled when counter reaches RC.
               |(0<<8)  //External Event Edge Selection: none
               |(0<<10)  //External Event Selection: TIOB
               |(0<<12)  //The external event has no effect on the counter and its clock.
               |(2<<13)  //UP mode with automatic trigger on RC Compare
               |(1<<15)  //Waveform Mode is enabled.
               |(1<<16)  //RA Compare Effect on TIOA: set
               |(2<<18)  //RC Compare Effect on TIOA: clear
               |(0<<20)  //External Event Effect on TIOA: none
               |(0<<22)  //Software Trigger Effect on TIOA: none
               |(0<<24)  //RB Compare Effect on TIOB: none
               |(0<<26)  //RC Compare Effect on TIOB: none
               |(0<<28)  //External Event Effect on TIOB: none
               |(0<<30); //Software Trigger Effect on TIOB: none
               
 //频率为200Hz
 *AT91C_TC1_RC=(Fmck/32)/200;
 *AT91C_TC1_RA=*AT91C_TC1_RC/2; //产生50%占空比的方波
 
 //初始化中断源13（定时器1中断）
 AT91C_AIC_SMR[13] = (0<<0)  //The priority level = 0(lowest)
                    |(3<<5); //Positive edge triggered
 
 AT91C_AIC_SVR[13]=(int)Timer1Isr;  //set ISR entrance address
 
 *AT91C_AIC_IECR=(1<<13);  //Enable source ID 13 interrupt
}
/////////////////////////End of function/////////////////////////////