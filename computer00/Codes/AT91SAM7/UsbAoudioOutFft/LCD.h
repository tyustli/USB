/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org
        
LCD.h  file

作者：Computer-lov
建立日期: 2009-
修改日期: 2009-
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/


#ifndef __LCD_H__
#define __LCD_H__

#include <AT91SAM7S64.H>

void LcdInit(void);
void LcdCls(void);
void LcdWriteCmd(unsigned char cmd);
void LcdWriteData(unsigned char dat);
void LcdSetXy(int x, int y);
void LcdSetPoint(int x, int y);
void LcdDrawPoint(int x, int y);
void LcdErasePoint(int x, int y);
void LcdDrawCircle(int x, int y, int r);
void LcdDrawLine(int x0, int y0, int x1, int y1);
void LcdFill(int x0, int y0, int x1, int y1);
void LcdPutChar(unsigned char ch);
void LcdRefresh(void);
void LcdPrints(unsigned char * p);

#define PI 3.1415926535897932384626433832795

#endif