/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

main.c  file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-03-10
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <AT91SAM7S64.H>
#include "SystemClock.h"
#include "uart.h"
#include "UsbCore.h"
#include "AT91SAMxUSB.h"
#include "config.h"
#include "adc.h"
#include "led.h"
#include "timer.h"
#include "key.h"
#include "lcd.h"

const uint8 HeadTable[][74]={
"********************************************************************\r\n",
"******            21IC DIY U盘学习板 之USB MIDI键盘设备       ******\r\n",
"******                     AT91SAM7S64 CPU                    ******\r\n",
"******                  建立日期：",__DATE__,"                 ******\r\n",
"******                   建立时间：",__TIME__,"                   ******\r\n",
"******                    作者：电脑圈圈                      ******\r\n",
"******                    欢迎访问作者的                      ******\r\n",
"******           USB专区：http://group.ednchina.com/93/       ******\r\n",
"******      BLOG1：http://www.ednchina.com/blog/computer00    ******\r\n",
"******                BLOG2：http://computer00.21ic.org       ******\r\n",
"******                   请按K1-K6分别进行测试                ******\r\n",
"******           K6:1 K5:2 K4:3 K1:4 K2: 5 K3:播放自动弹      ******\r\n",
"********************************************************************\r\n",
};

/********************************************************************
函数功能：根据按键情况返回Note On消息的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void SendNoteOnMsg(void)
{
 //4字节的缓冲区
 uint8 Buf[4];
 
 //Note On消息第一字节固定为0x09，第二字节为0x9n（n为通道号）
 //第三字节为0xKK（K为音高），第四字节为0xVV（V为力度）。
 
 Buf[0]=0x09; //Note On消息的包头
 Buf[1]=0x90; //在通道0上发送Note On消息
 Buf[3]=0x7F; //音量设置为最大
 
 if(KeyDown&KEY6)
 {
  Buf[2]=60;  //C调的1（绝对音高为C音，即中央C）
  //通过端点1返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyDown&=~KEY6; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 if(KeyDown&KEY5)
 {
  Buf[2]=62;  //C调的2（绝对音高为D音）
  //通过端点1返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyDown&=~KEY5; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 if(KeyDown&KEY4)
 {
  Buf[2]=64;  //C调的3（绝对音高为E音）
  //通过端点1返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyDown&=~KEY4; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 if(KeyDown&KEY1)
 {
  Buf[2]=65;  //C调的4（绝对音高为F音）
  //通过端点1返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyDown&=~KEY1; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 if(KeyDown&KEY2)
 {
  Buf[2]=67;  //C调的5（绝对音高为G音）
  //通过端点2返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyDown&=~KEY2; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 //如果有按键弹起，则关闭对应的音
 Buf[3]=0x00;  //音量设置为0
 
 if(KeyUp&KEY6)
 {
  Buf[2]=60;  //C调的1（绝对音高为C音，即中央C）
  //通过端点1返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyUp&=~KEY6; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 if(KeyUp&KEY5)
 {
  Buf[2]=62;  //C调的2（绝对音高为D音）
  //通过端点1返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyUp&=~KEY5; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 if(KeyUp&KEY4)
 {
  Buf[2]=64;  //C调的3（绝对音高为E音）
  //通过端点1返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyUp&=~KEY4; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 if(KeyUp&KEY1)
 {
  Buf[2]=65;  //C调的4（绝对音高为F音）
  //通过端点1返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyUp&=~KEY1; //清除对应的按键
  return;  //发送一个音符后就返回
 }
 
 if(KeyUp&KEY2)
 {
  Buf[2]=67;  //C调的5（绝对音高为G音）
  //通过端点2返回4字节MIDI事件包。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
  KeyUp&=~KEY2; //清除对应的按键
  return;  //发送一个音符后就返回
 } 
}
////////////////////////End of function//////////////////////////////

void PlaySong(void);

/********************************************************************
函数功能：在LCD上显示按键情况。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void DispKey(void)
{
 int i;

 for(i=0;i<6;i++)
 {
  LcdSetPoint(0,0);
  if(KeyDown&(1<<i))
  {
   LcdPrints("Key ");
   LcdPutChar('1'+i);
   LcdPrints(" down  ");
  }
  LcdSetPoint(0,0);
  if(KeyUp&(1<<i))
  {
   LcdPrints("Key ");
   LcdPutChar('1'+i);
   LcdPrints(" up    ");
  }
 }
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：主函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void main(void)
{
 int i;

 int InterruptSource;
 
 SystemClockInit(); //系统时钟初始化
 LedInit();         //LED对应的管脚初始化
 LcdInit();         //LCD初始化
 AdcInit();         //ADC初始化
 Timer1Init();      //定时器1初始化，用来产生10ms的定时扫描信号
 KeyInit();         //键盘初始化
 Uart0Init();       //串口0初始化

 for(i=0;i<17;i++)   //显示头信息
 {
  Prints(HeadTable[i]);
 }

 UsbChipInit();  //初始化USB部分
 
 while(1)
 {
  InterruptSource=(*AT91C_UDP_ISR)&(0x0F|(1<<8)|(1<<12)); //取出需要的中断
  if(InterruptSource) //如果监视的中断发生
  {
   if(InterruptSource&(1<<8))
   {
    *AT91C_UDP_ICR=1<<8; //清除中断
    UsbBusSuspend(); //总线挂起中断处理
   }
   if(InterruptSource&(1<<12))
   {
    *AT91C_UDP_ICR=1<<12; //清除中断
    UsbBusReset();   //总线复位中断处理
   }
   if(InterruptSource&(1<<0))
   {
    if(AT91C_UDP_CSR[0]&((1<<1)|(1<<2)|(1<<6)))  //如果是SETUP包、缓冲未空等
    {
     UsbEp0Out();     //端点0输出中断处理
    }
    if(AT91C_UDP_CSR[0]&(1<<0)) //如果是端点0输入完成
    {
     UsbEp0In();     //端点0输入中断处理
    }
   }
   if(InterruptSource&(1<<1))
   {
    UsbEp1In();      //端点1输入中断处理
   }
   if(InterruptSource&(1<<2))
   {
    UsbEp2Out();     //端点2输出中断处理
   }
   if(InterruptSource&(1<<3))
   {
    UsbEp3In();     //端点3输入中断处理
   }
  }
  if(KeyUp||KeyDown)  //如果用户操作了按键
  {
   DispKey(); //在LCD上显示按键情况
   if(ConfigValue!=0) //如果已经设置为非0的配置，则可以返回报告数据
   {
    if(!Ep1InIsBusy)  //如果端点1输入没有处于忙状态，则可以发送数据
    {
     KeyCanChange=0;  //禁止按键扫描
     if(KeyPress==(KEY3))  //如果按住KEY3，则自动播放曲子
     {
      KeyCanChange=1; //允许按键扫描
      PlaySong(); //播放歌曲
     }
     if(KeyUp||KeyDown) //如果有按键事件发生
     {
      SendNoteOnMsg();  //则返回MIDI Note On消息
     }
     KeyCanChange=1;  //允许按键扫描
    }
   }
   LcdRefresh();  //刷新LCD显示
  }
 }
}
/////////////////////////End of function/////////////////////////////

