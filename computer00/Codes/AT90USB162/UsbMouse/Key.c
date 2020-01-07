/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

KEY.C  file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2009-05-10
版本：V1.2
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/

#include <90USB162.H>
#include "Key.h"
#include "MyType.h"

volatile uint8  KeyCurrent,KeyOld,KeyNoChangedTime;
volatile uint8  KeyPress;
volatile uint8  KeyDown,KeyUp,KeyLast;

volatile uint8 KeyCanChange;

/********************************************************************
函数功能：键盘初始化
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void KeyInit(void)
{
 KeyPress=0;       //无按键按下
 KeyNoChangedTime=0;
 KeyOld=0;
 KeyCurrent=0;
 KeyLast=0;
 KeyDown=0;
 KeyUp=0;
 KeyCanChange=1;   //允许键值改变
}
/*******************************************************************/

/********************************************************************
函数功能：获取按键值。
入口参数：无。
返    回：无。
备    注：按键值的位图。
********************************************************************/
uint8 KeyGetValue(void)
{
 //读者可以自行在此增加读取按键状况的函数，实现按键的功能。
 return 0;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：键盘扫描程序
入口参数：无。
返    回：无。
备    注：在中断函数中每5ms调用一次。
********************************************************************/
void KeyScan(void)
{
 if(!KeyCanChange)return;     //如果正在处理按键，则不再扫描键盘 

 //开始键盘扫描
 //保存按键状态到当前按键情况
 //KeyCurrent总共有8个bit
 //当某个开关按下时，对应的bit为1
 KeyCurrent=KeyGetValue(); //读取键值

 if(KeyCurrent!=KeyOld)  //如果两次值不等，说明按键情况发生了改变
 {
  KeyNoChangedTime=0;       //键盘按下时间为0
  KeyOld=KeyCurrent;        //保存当前按键情况
  return;  //返回
 }
 else
 {
  KeyNoChangedTime++;      //按下时间累计
  if(KeyNoChangedTime>=1)  //如果按下时间足够
  {
   KeyNoChangedTime=1;
   KeyPress=KeyOld;      //保存按键
   KeyDown|=(~KeyLast)&(KeyPress); //求出新按下的键
   KeyUp|=KeyLast&(~KeyPress);     //求出新释放的键
   KeyLast=KeyPress;               //保存当前按键情况
  }
 }
}
/*******************************************************************/


