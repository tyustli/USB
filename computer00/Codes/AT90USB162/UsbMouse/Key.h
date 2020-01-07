/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

KEY.H  file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2009-05-10
版本：V1.1
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/


#ifndef __KEY_H__
#define __KEY_H__

#include "MyType.h"

extern volatile uint8 KeyCurrent,KeyOld,KeyNoChangedTime;
extern volatile uint8 KeyPress;
extern volatile uint8 KeyDown,KeyUp,KeyLast;

extern volatile uint8 KeyCanChange;

void KeyInit(void);
uint8 KeyGetValue(void);
void KeyScan(void);

#define KEY1 0x01
#define KEY2 0x02
#define KEY3 0x04
#define KEY4 0x08
#define KEY5 0x10
#define KEY6 0x20
#define KEY7 0x40
#define KEY8 0x80

#endif
