/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

main.c  file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-05-11
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <90USB162.H>
#include "SystemClock.h"
#include "LED.h"
#include "Timer.h"
#include "uart.h"
#include "UsbCore.h"
#include "AT90USB.h"

#ifdef DEBUG0
const uint8 HeadTable[][74]={
"********************************************************************\r\n",
"******              《圈圈教你玩USB》之 USB转串口             ******\r\n",
"******                      AT90USB162 CPU                    ******\r\n",
"******                  建立日期：",__DATE__,"                 ******\r\n",
"******                   建立时间：",__TIME__,"                   ******\r\n",
"******                    作者：电脑圈圈                      ******\r\n",
"******                    欢迎访问作者的                      ******\r\n",
"******           USB专区：http://group.ednchina.com/93/       ******\r\n",
"******      BLOG1：http://www.ednchina.com/blog/computer00    ******\r\n",
"******                BLOG2：http://computer00.21ic.org       ******\r\n",
"******                     请打开串口进行测试                 ******\r\n",
"******                                                        ******\r\n",
"********************************************************************\r\n",
};
#endif

/********************************************************************
函数功能：将串口缓冲区中的数据发送到端点3的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void SendUartDataToEp3(void)
{
 uint8 Len;
 //暂时禁止串行中断，防止UartByteCount在中断中修改而导致不同步
 #asm("cli");
 Len=UartByteCount;   //获取长度
 //检查长度是否为0，如果没有收到数据，则不需要处理，直接返回
 if(Len==0)
 {
  #asm("sei"); //打开中断
  return;
 }
 //检查Len字节个数据是否跨越了缓冲区边界，如果跨越了，那么本次只发送
 //跨越边界之前的数据，剩余的数据留待下次发送。否则，可以一次发送全部。
 if((Len+UartBufferOutputPoint)>BUF_LEN)
 {
  Len=BUF_LEN-UartBufferOutputPoint;
 }
 //修改缓冲区数据字节数
 UartByteCount-=Len;
 
 //到这里可以打开串口中断了
 #asm("sei");
 
 //将数据写入到端点3输入缓冲区
 UsbChipWriteEndpointBuffer(3,Len,UartBuffer+UartBufferOutputPoint);
 //修改输出数据的位置
 UartBufferOutputPoint+=Len;
 //如果已经到达缓冲区末尾，则设置回开头
 if(UartBufferOutputPoint>=BUF_LEN)
 {
  UartBufferOutputPoint=0;
 }
 Ep3InIsBusy=1;
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
 
 OnLed2(); //电源指示灯
  
 Uart1Init();       //串口0初始化

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

  if(ConfigValue!=0) //如果已经设置为非0的配置，则可以处理数据
  {   
   if(Ep3InIsBusy==0)  //如果端点3空闲，则发送串口数据到端点3
   {
    SendUartDataToEp3();  //调用函数将缓冲区数据发送到端点3
   }
   if(UsbEp4ByteCount!=0) //端点4接收缓冲区中还有数据未发送，则发送到串口
   {
    //发送一字节到串口
    UartPutChar(UsbEp4Buffer[UsbEp4BufferOutputPoint]);
    UsbEp4BufferOutputPoint++; //发送位置后移1
    UsbEp4ByteCount--;   //计数值减1
   }
  }
 }
}
/////////////////////////End of function/////////////////////////////

