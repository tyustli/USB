/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

main.c  file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-05-10
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <90USB162.H>
#include "SystemClock.h"
#include "LED.h"
#include "Timer.h"
#include "key.h"
#include "uart.h"
#include "UsbCore.h"
#include "AT90USB.h"

#ifdef DEBUG0
const uint8 HeadTable[][74]={
"********************************************************************\r\n",
"******                《圈圈教你玩USB》之USB鼠标              ******\r\n",
"******                     AT91SAM7S64 CPU                    ******\r\n",
"******                  建立日期：",__DATE__,"                 ******\r\n",
"******                   建立时间：",__TIME__,"                   ******\r\n",
"******                    作者：电脑圈圈                      ******\r\n",
"******                    欢迎访问作者的                      ******\r\n",
"******           USB专区：http://group.ednchina.com/93/       ******\r\n",
"******      BLOG1：http://www.ednchina.com/blog/computer00    ******\r\n",
"******                BLOG2：http://computer00.21ic.org       ******\r\n",
"******                  每4秒钟将自动弹出一次右键演示         ******\r\n",
"******                                                        ******\r\n",
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

 #asm("cli");
 
 MCUSR=0;
 MCUSR &= ~(1 << 3);
 WDTCSR |= (1<<4) | (1<<3);
 /* Turn off WDT */
 WDTCSR = 0x00;

 SystemClockInit(); //系统时钟初始化
 LedInit();         //LED对应的管脚初始化

 Timer0Init();      //定时器0初始化，用来产生5ms的定时扫描信号
 KeyInit();         //键盘初始化
 
 OnLed2(); //电源指示灯
  
#if (defined DEBUG0)||(defined DEBUG1)
 Uart1Init();       //串口0初始化
#endif

#ifdef DEBUG0 
 for(i=0;i<17;i++)   //显示头信息
 {
  Prints(HeadTable[i]);
 }
#endif
 
 UsbChipInit();  //初始化USB部分
 
 while(1)
 {
  if(UDINT&(1<<0)) //SUSPI - Suspend Interrupt Flag
  {
   UDINT=~(1<<0); //清除中断
   UsbBusSuspend(); //总线挂起中断处理
  }
  if(UDINT&(1<<3)) //EORSTI - End Of Reset Interrupt Flag
  {
   UDINT=~(1<<3); //清除中断
   UsbBusReset();   //总线复位中断处理
  }
  UENUM=0; //选择端点0
  if(UEINTX&(3<<2))  //FIFOCON - FIFO Control Bit //如果是SETUP包、输出数据等
  {
   UsbEp0Out();     //端点0输出中断处理
  }
  if((SendLength!=0)||(NeedZeroPacket)) //如果端点0有数据要发送
  {
   if(UEINTX&(1<<0)) //并且端点0缓冲区空闲
   { 
    UsbEp0In();      //端点0输入中断处理
   }
  }
  UENUM=1; //选择端点1
  if(UEINTX&(1<<0))
  {
   UsbEp1In();      //端点1输入中断处理
  }
  UENUM=2; //选择端点2
  if(UEINTX&(1<<2))
  {
   UsbEp2Out();     //端点2输出中断处理
  }
  UENUM=3; //选择端点3
  if(UEINTX&(1<<0))
  {
   UsbEp3In();      //端点3输入中断处理
  }
  UENUM=4; //选择端点4
  if(UEINTX&(1<<2))
  {
   UsbEp4Out();     //端点4输出中断处理
  }

  //实际上，这个板上并没有按键，为了演示程序，在定时器0中每隔4秒
  //模拟一次开关按下，发送一次鼠标右键。你可以根据实际情况返回鼠标的报告。
  if(KeyUp||KeyDown||KeyPress)  //如果用户操作了按键或者压住按键
  {
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

