/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

main.c  file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-03-09
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <AT91SAM7S64.H>
#include "SystemClock.h"
#include "LED.h"
#include "LCD.h"
#include "ADC.H"
#include "Timer.h"
#include "key.h"
#include "uart.h"
#include "UsbCore.h"
#include "AT91SAMxUSB.h"

const uint8 HeadTable[][74]={
"********************************************************************\r\n",
"******               21IC DIY U盘学习板 之USB鼠标             ******\r\n",
"******                     AT91SAM7S64 CPU                    ******\r\n",
"******                  建立日期：",__DATE__,"                 ******\r\n",
"******                   建立时间：",__TIME__,"                   ******\r\n",
"******                    作者：电脑圈圈                      ******\r\n",
"******                    欢迎访问作者的                      ******\r\n",
"******           USB专区：http://group.ednchina.com/93/       ******\r\n",
"******      BLOG1：http://www.ednchina.com/blog/computer00    ******\r\n",
"******                BLOG2：http://computer00.21ic.org       ******\r\n",
"******                  请按K1-K6分别进行测试                 ******\r\n",
"******              K1:鼠标上移  K2:鼠标下移  K3:鼠标左键     ******\r\n",
"******              K6:鼠标左移  K5:鼠标右移  K4:鼠标右键     ******\r\n",
"******                                                        ******\r\n",
"********************************************************************\r\n",
};

/********************************************************************
函数功能：根据按键情况返回报告的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void SendReport(void)
{
 //需要返回的4字节报告的缓冲
 //Buf[0]的D0就是左键，D1就是右键，D2就是中键（这里没有）
 //Buf[1]为X轴，Buf[2]为Y轴，Buf[3]为滚轮
 uint8 Buf[4]={0,0,0,0};
 
 //如果有按键按住，并且不是KEY3、KEY4（左、右键）
 //或者KEY3、KEY4任何一个键有变动的话，则需要返回报告
 if((KeyPress&(~(KEY3|KEY4)))||(KeyUp&(KEY3|KEY4))||(KeyDown&(KEY3|KEY4)))
 {
  if(KeyPress & KEY1) //如果KEY1按住，则光标需要左移，即X轴为负值。
  {
   Buf[1]=-1;  //这里一次往左移动一个单位。
  }
  if(KeyPress & KEY2) //如果KEY2按住，则光标需要右移，即X轴为正值。
  {
   Buf[1]=1;   //这里一次往右移动一个单位。
  }
  if(KeyPress & KEY6) //如果KEY6按住，则光标需要上移，即Y轴为负值。
  {
   Buf[2]=-1;   //这里一次往上移动一个单位。
  }
  if(KeyPress & KEY5)  //如果KEY5按住，则光标需要下移，即Y轴为正值。
  {
   Buf[2]=1;  //这里一次往下移动一个单位。
  }
  if(KeyPress & KEY3)  //鼠标左键
  {
   Buf[0]|=0x01;  //D0为鼠标左键
  }
  if(KeyPress & KEY4)  //鼠标右键
  {
   Buf[0]|=0x02;  //D1为鼠标右键
  }
  //报告准备好了，通过端点1返回，长度为4字节。
  UsbChipWriteEndpointBuffer(1,4,Buf);
  Ep1InIsBusy=1;  //设置端点忙标志。
 }
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
 LcdCls();
 for(i=0;i<6;i++)
 {
  LcdSetPoint(0,8);
  if(KeyDown&(1<<i))
  {
   LcdPrints("Key ");
   LcdPutChar('1'+i);
   LcdPrints(" down");
  }
  LcdSetPoint(0,0);
  if(KeyUp&(1<<i))
  {
   LcdPrints("Key ");
   LcdPutChar('1'+i);
   LcdPrints(" up");
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
 
 for(i=0;i<19;i++)   //显示头信息
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
  }

  if(KeyUp||KeyDown||KeyPress)  //如果用户操作了按键或者压住按键
  {
   if(KeyUp||KeyDown)
   {
    DispKey(); //在LCD上显示按键情况
    LcdRefresh();  //刷新LCD显示
   }
   if(ConfigValue!=0) //如果已经设置为非0的配置，则可以返回报告数据
   {
    if(!Ep1InIsBusy)  //如果端点1输入没有处于忙状态，则可以发送数据
    {
     KeyCanChange=0;  //禁止按键扫描
     if(KeyUp||KeyDown||KeyPress) //如果有按键事件发生
     {
      SendReport();  //则返回报告
     }
     KeyCanChange=1;  //允许按键扫描
    }
   }   
   //清除KeyUp和KeyDown
   KeyUp=0;
   KeyDown=0;
  }
 }
}
/////////////////////////End of function/////////////////////////////

