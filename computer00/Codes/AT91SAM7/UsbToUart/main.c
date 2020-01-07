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

#ifdef DEBUG0
const uint8 HeadTable[][74]={
"********************************************************************\r\n",
"******               21IC DIY U盘学习板 之USB转串口           ******\r\n",
"******                     AT91SAM7S64 CPU                    ******\r\n",
"******                  建立日期：",__DATE__,"                 ******\r\n",
"******                   建立时间：",__TIME__,"                   ******\r\n",
"******                    作者：电脑圈圈                      ******\r\n",
"******                    欢迎访问作者的                      ******\r\n",
"******           USB专区：http://group.ednchina.com/93/       ******\r\n",
"******      BLOG1：http://www.ednchina.com/blog/computer00    ******\r\n",
"******                BLOG2：http://computer00.21ic.org       ******\r\n",
"********************************************************************\r\n",
};
#endif

/********************************************************************
函数功能：将串口缓冲区中的数据发送到端点1的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void SendUartDataToEp1(void)
{
 uint8 Len;
 
 //暂时禁止串行中断，防止UartByteCount在中断中修改而导致不同步
 *AT91C_US0_IDR = (1<<0); //RXRDY Interrupt Disable
 //将串口缓冲区接收到的字节数复制出来
 Len=UartByteCount;
 //检查长度是否为0，如果没有收到数据，则不需要处理，直接返回
 if(Len==0)
 {
  *AT91C_US0_IER = (1<<0);    //记得打开串口中断
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
 *AT91C_US0_IER = (1<<0);    //RXRDY Interrupt Enable
 
 //将数据写入到端点1输入缓冲区
 UsbChipWriteEndpointBuffer(1,Len,UartBuffer+UartBufferOutputPoint);
 //修改输出数据的位置
 UartBufferOutputPoint+=Len;
 //如果已经到达缓冲区末尾，则设置回开头
 if(UartBufferOutputPoint>=BUF_LEN)
 {
  UartBufferOutputPoint=0;
 }
  Ep1InIsBusy=1;
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

 int InterruptSource;
 
 SystemClockInit(); //系统时钟初始化

 Uart0Init();       //串口0初始化

#ifdef DEBUG0
 for(i=0;i<15;i++)   //显示头信息
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
  if(ConfigValue!=0) //如果已经设置为非0的配置，则可以返回和发送串口数据
  {   
   if(Ep1InIsBusy==0)  //如果端点1空闲，则发送串口数据到端点1
   {
    SendUartDataToEp1();  //调用函数将缓冲区数据发送到端点1
   }
   if(UsbEp2ByteCount!=0) //端点2接收缓冲区中还有数据未发送，则发送到串口
   {
    //发送一字节到串口
    UartPutChar(UsbEp2Buffer[UsbEp2BufferOutputPoint]);
    UsbEp2BufferOutputPoint++; //发送位置后移1
    UsbEp2ByteCount--;   //计数值减1
   }
  }
 }
}
/////////////////////////End of function/////////////////////////////

