/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org
          
Timer.c  file
         
作者：Computer-lov
建立日期: 2009-03-06
修改日期: 2009-05-10
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <90USB162.H>
#include "timer.h"
#include "key.h"
#include "config.h"
#include "led.h"
#include "UsbCore.h"

/********************************************************************
函数功能：定时器1中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
interrupt [TIM0_OVF] void Timer0Isr(void)
{
 static uint8 i,Count;
 static uint16 KeyCount;
 TCNT0=0xFF-78;  //定时器重装
 KeyScan();  //按键扫描
 Count++;
 //由于圈圈在这里并没有实现键盘的功能，所以用软件模拟一个右键
 //如果读者自行实现了按键功能，请将这段代码注释掉，避免混乱
 KeyCount++;
 if(KeyCount%790==0) //每4秒发一次
 {
  KeyPress=KEY4;  //KEY4是右键
  KeyDown=KEY4;
 }
 if(KeyCount>=800)  //50ms后，释放右键，这时将弹出右键菜单
 {
  KeyCount=0;
  KeyPress=0;
  KeyUp=KEY4;
 }
 //////////////////模拟右键功能完毕//////////////////////
 if(Count>=25) //每秒闪烁8次
 {
  Count=0;
  if(UsbLedBlink)   //如果需要闪烁
  {
   if(i) OnLed1();
   else OffLed1();
   i=!i;
   UsbLedBlink--;
  }
  else
  {
   if(ConfigValue) //如果配置值为非0，Led1闪烁后亮
   {
    OnLed1();
   }
   else         //否则，Led1闪烁后灭
   {
    OffLed1();
   }
  }
 }
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：定时器0初始化。
入口参数：无。
返    回：无。
备    注：产生一个周期为5ms的定时中断信号。
********************************************************************/
void Timer0Init(void)
{
 // Timer/Counter 0 initialization
 // Clock source: System Clock
 // Clock value: 15.625 kHz
 // Mode: Normal top=FFh
 // OC0A output: Disconnected
 // OC0B output: Disconnected
 TCCR0A=0x00;
 TCCR0B=0x05;
 TCNT0=0xFF-78;
 OCR0A=0x00;
 OCR0B=0x00; 
 //Timer 1 overflow interrupt is on
 TIMSK0|=0x01;
 #asm("sei");  //开中断
}
/////////////////////////End of function/////////////////////////////