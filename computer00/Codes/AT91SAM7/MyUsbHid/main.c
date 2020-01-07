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

#ifdef DEBUG0
const uint8 HeadTable[][74]={
"********************************************************************\r\n",
"******            21IC DIY U盘学习板 之用户自定义HID设备      ******\r\n",
"******                     AT91SAM7S64 CPU                    ******\r\n",
"******                  建立日期：",__DATE__,"                 ******\r\n",
"******                   建立时间：",__TIME__,"                   ******\r\n",
"******                    作者：电脑圈圈                      ******\r\n",
"******                    欢迎访问作者的                      ******\r\n",
"******           USB专区：http://group.ednchina.com/93/       ******\r\n",
"******      BLOG1：http://www.ednchina.com/blog/computer00    ******\r\n",
"******                BLOG2：http://computer00.21ic.org       ******\r\n",
"******          打开上位机应用软件，然后按K1-K6分别进行测试   ******\r\n",
"********************************************************************\r\n",
};
#endif

/********************************************************************
函数功能：根据按键情况返回报告的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void SendReport(void)
{
 //需要返回的8字节报告的缓冲。但在本测试程序中，只使用前5字节。
 uint8 Buf[8]={0,0,0,0,0,0,0,0};
 
 //每发送一次数据，则将Count增加一。
 Count++;
 
 //根据不同的按键设置输入报告。这里将8个按键情况放在第一字节。
 Buf[0]=KeyPress;
 
 //根据Count的值设置报告的第二到第五字节。
 Buf[1]=(Count&0xFF);       //最低字节
 Buf[2]=((Count>>8)&0xFF);  //次低字节
 Buf[3]=((Count>>16)&0xFF); //次高字节
 Buf[4]=((Count>>24)&0xFF); //最高字节
 
 //报告准备好了，通过端点1返回，长度为8字节。
 UsbChipWriteEndpointBuffer(1,8,Buf);
 Ep1InIsBusy=1;  //设置端点忙标志。
}
////////////////////////End of function//////////////////////////////


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
#ifdef DEBUG0
 int i;
#endif

 int InterruptSource;
 
 SystemClockInit(); //系统时钟初始化
 LedInit();         //LED对应的管脚初始化
 LcdInit();         //LCD初始化
 AdcInit();         //ADC初始化
 Timer1Init();      //定时器1初始化，用来产生10ms的定时扫描信号
 KeyInit();         //键盘初始化
 Uart0Init();       //串口0初始化

#ifdef DEBUG0
 for(i=0;i<16;i++)   //显示头信息
 {
  Prints(HeadTable[i]);
 }
#endif

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
     if(KeyUp||KeyDown) //如果有按键事件发生
     {
      SendReport();  //则返回报告
     }
     KeyCanChange=1;  //允许按键扫描
    }
   }
   //清除KeyUp和KeyDown
   KeyUp=0;
   KeyDown=0;
   LcdRefresh();  //刷新LCD显示
  }
 }
}
/////////////////////////End of function/////////////////////////////

