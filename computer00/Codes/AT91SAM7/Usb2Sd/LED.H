/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org
        
LED.h  file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-03-04
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/


#ifndef __LED_H__
#define __LED_H__

#include <AT91SAM7S64.H>

#define OnLed2() *AT91C_PIOA_CODR=(1<<2)
#define OnLed3() *AT91C_PIOA_CODR=(1<<1)
#define OnLed4() *AT91C_PIOA_CODR=(1<<0)

#define OffLed2() *AT91C_PIOA_SODR=(1<<2)
#define OffLed3() *AT91C_PIOA_SODR=(1<<1)
#define OffLed4() *AT91C_PIOA_SODR=(1<<0)

void LedInit(void);
void DelayXms(unsigned long int x);

#endif
