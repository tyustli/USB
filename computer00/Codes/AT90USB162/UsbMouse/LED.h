/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org
        
LED.h  file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-05-10
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/


#ifndef __LED_H__
#define __LED_H__

#include <90USB162.h>

#define OnLed1() PORTB|=(1<<7)
#define OnLed2() PORTB|=(1<<6)

#define OffLed1() PORTB&=~(1<<7)
#define OffLed2() PORTB&=~(1<<6)

extern volatile unsigned char UsbLedBlink;

void LedInit(void);
void DelayXms(unsigned long int x);

#endif
