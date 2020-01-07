/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

AT90USB.C  file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2009-05-10
版本：V1.1
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/

#include <90USB162.H>
#include "MyType.h"
#include "AT90USB.h"
#include "config.h"
#include "UART.h"
#include "Led.h"
#include "UsbCore.h"

/********************************************************************
函数功能：USB断开连接函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbDisconnect(void)
{
#ifdef DEBUG0
 Prints("断开USB连接。\r\n");
#endif
 UDCON=0x01; //Disconnect pull-up resistor
 DelayXms(1000);  //延迟1秒
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：USB连接函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbConnect(void)
{
#ifdef DEBUG0
 Prints("连接USB。\r\n");
#endif
 USBCON=0x80;  //使能时钟
 UDCON=0x00;   //Connect pull-up resistor
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：USB芯片初始化。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbChipInit(void)
{
 UsbDisconnect();  //先断开USB连接
 
 PLLCSR=0x04;   //PLL clock Prescaler为2分频
 PLLCSR|=0x02;  //启动PLL
 while(!(PLLCSR&0x01));  //等待PLL启动完成
 
 USBCON=0x00; //复位USB模块
 USBCON=0x80; //使能USB模块
 UDPADDH=0x00; //FIFO
 UPOE=0x00;

 //disable all USB interrupts
 UDIEN=0x00;
 UEIENX=0x00; 
 
 ConfigValue=0; //配置值初始化为0 

 UsbConnect();     //将USB连接上 
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：USB端点复位。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbChipResetEndpoint(void)
{ 
 UERST=0x1F;  //复位端点
 UERST=0x00;  //复位端点完成
 
 UENUM=0;
 UEINTX=0x00;  //清除中断标志   
 UECONX=0x01;  //使能端点0
 UECFG0X=0x00; //设置为控制输出端点
 UECFG1X=0x02; //设置为8字节、单缓冲，分配内存
 USBCON=0x80;  //使能USB模块
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：设置芯片配置状态
入口参数：Value:配置值。
返    回：无。
备    注：无。
********************************************************************/
void UsbChipSetConfig(uint8 Value)
{
 //无操作
 Value=0;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：设置芯片进入设置地址状态
入口参数：Value：地址状态，非0为设置地址，0为未设置地址。
返    回：无。
备    注：无。
********************************************************************/
void UsbChipSetAddressStatus(uint8 Value)
{
 if(Value==0)
 {
  UDADDR&=~(1<<7);  //默认状态
 }
 else
 {
  UDADDR|=(1<<7);  //进入到设置地址阶段
 }
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：设置USB芯片功能地址函数。
入口参数：Addr：要设置的地址值。
返    回：无。
备    注：无。
********************************************************************/
void UsbChipWriteAddress(uint8 Addr)
{
 UDADDR=Addr;  //设置地址
 //等待前一个数据包（实际上是状态阶段）发送完毕
 //等待中断产生
 while(1)
 {
  if(UEINTX&(1<<0))break; //发送完毕
  if(UDINT&((1<<0)||(1<<3)))return; //如果产生复位、挂起则直接返回
 }
 if(Addr!=0)  //如果地址非0
 {
  UsbChipSetAddressStatus(1); //设置进入到设置地址阶段
 }
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：使能端点函数。
入口参数：Enable: 是否使能。0值为不使能，非0值为使能。
返    回：无。
备    注：无。
********************************************************************/
void UsbChipSetEndpointEnable(uint8 Enable)
{
 if(Enable!=0)
 {
  UENUM=1;   
  UECONX=1;  //使能端点1
  UECFG0X=0xC1; //设置为中断输入端点
  UECFG1X=0x12; //设置为16字节、单缓冲，分配内存        
  UENUM=2;   
  UECONX=1;  //使能端点2
  UECFG0X=0xC0; //设置为中断输出端点
  UECFG1X=0x12; //设置为16字节、单缓冲，分配内存
  UENUM=3;   
  UECONX=1;  //使能端点3
  UECFG0X=0x81; //设置为批量输入端点
  UECFG1X=0x32; //设置为64字节、单缓冲，分配内存
  UENUM=4;   
  UECONX=1;  //使能端点4
  UECFG0X=0x80;  //设置为批量输出端点
  UECFG1X=0x32; //设置为64字节、单缓冲，分配内存
 }
 else
 {
 }
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：判断是否是SETUP包。
入口参数：无。
返    回：0：不是SETUP；非0：是SETUP。
备    注：无。
********************************************************************/
int UsbChipIsSetup(uint8 Endp)
{
 UENUM=Endp; //选择端点
 if(UEINTX&(1<<3))
 {
  return 0xFF;  //是setup包，返回非0
 }
 else
 {
  return 0; //不是setup包，返回0
 }
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：清除接收端点缓冲区的函数。
入口参数：无。
返    回：无。
备    注：只有使用该函数清除端点缓冲后，该接收端点才能接收新的数据包。
********************************************************************/
void UsbChipClearBuffer(uint8 Endp)
{
 UENUM=Endp; //选择端点
 UEINTX=~(1<<7); //清除FIFOCON - FIFO Control Bit
 UsbLedBlink=2; //闪烁LED，表示有数据通信
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：应答建立包的函数。
入口参数：Endp:端点号。
返    回：无。
备    注：无。
********************************************************************/
void UsbChipAcknowledgeSetup(uint8 Endp)
{
 UENUM=Endp; //选择端点
 UEINTX=~(1<<3);  //清除RXSTPI
 UsbLedBlink=2; //闪烁LED，表示有数据通信
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：读取端点缓冲区函数。
入口参数：Endp：端点号；Len：需要读取的长度；Buf：保存数据的缓冲区。
返    回：实际读到的数据长度。
备    注：无。
********************************************************************/
uint8 UsbChipReadEndpointBuffer(uint8 Endp, uint8 Len, uint8 *Buf)
{
 uint8 i,j;
 UENUM=Endp; //选择端点
 j=UEBCLX; //获取数据长度
 if(j>Len) //如果要读的字节数比实际接收到的数据长
 {
  j=Len;  //则只读指定的长度数据
 }
#ifdef DEBUG1 //如果定义了DEBUG1，则需要显示调试信息
 Prints("读端点");
 PrintLongInt(Endp);
 Prints("缓冲区");
 PrintLongInt(j);      //实际读取的字节数
 Prints("字节。\r\n");
#endif
 for(i=0;i<j;i++)
 {
  *(Buf+i)=UEDATX;  //从FIFO中读一字节数据
#ifdef DEBUG1
  PrintHex(*(Buf+i)); //如果需要显示调试信息，则显示读到的数据
  if(((i+1)%16)==0)Prints("\r\n"); //每16字节换行一次
#endif
 }
#ifdef DEBUG1
 if((j%16)!=0)Prints("\r\n"); //换行。
#endif
 return j; //返回实际读取的字节数。
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：使能发送端点缓冲区数据有效的函数。
入口参数：Endp: 端点号。
返    回：无。
备    注：只有使用该函数使能发送端点数据有效之后，数据才能发送出去。
********************************************************************/
void UsbChipValidateBuffer(unsigned char Endp)
{
 UENUM=Endp; //选择端点
 if(Endp==0)
 {
  //对于端点0，清除端点0的输入完成中断标志位，将使能数据发送
  UEINTX=~(1<<0);
 }
 else //对于非0端点
 {
  UEINTX=~(1<<7); //清除FIFOCON - FIFO Control Bit使能数据发送
 }
 UsbLedBlink=2; //闪烁LED，表示有数据通信
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：将处于RAM中的数据写入端点缓冲区函数。
入口参数：Endp：端点号；Len：需要发送的长度；Buf：保存数据的缓冲区。
返    回：Len的值。
备    注：此函数从RAM中读取数据并写入到端点缓冲区。
********************************************************************/
uint8 DataInRam_WriteEndpointBuffer(uint8 Endp,uint8 Len,uint8 * Buf)
{
 uint8 i;
 
#ifdef DEBUG1 //如果定义了DEBUG1，则需要显示调试信息
 Prints("写端点");
 PrintLongInt(Endp);
 Prints("缓冲区");
 PrintLongInt(Len);    //写入的字节数
 Prints("字节。\r\n");
#endif
 UENUM=Endp; //选择端点
 for(i=0;i<Len;i++)
 {
  UEDATX=*(Buf+i); //将数据写到FIFO中
#ifdef DEBUG1
  PrintHex(*(Buf+i));  //如果需要显示调试信息，则显示发送的数据
  if(((i+1)%16)==0)Prints("\r\n"); //每16字节换行一次
#endif
  }
#ifdef DEBUG1
 if((Len%16)!=0)Prints("\r\n"); //换行
#endif
 UsbChipValidateBuffer(Endp); //使端点数据有效
 return Len; //返回Len
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：将处于FALSH中的数据写入端点缓冲区函数。
入口参数：Endp：端点号；Len：需要发送的长度；Buf：保存数据的缓冲区。
返    回：Len的值。
备    注：此函数从Flash中读取数据并写入到端点缓冲区。
********************************************************************/
uint8 DataInFlash_WriteEndpointBuffer(uint8 Endp,uint8 Len, flash uint8 * Buf)
{
 uint8 i;
 
#ifdef DEBUG1 //如果定义了DEBUG1，则需要显示调试信息
 Prints("写端点");
 PrintLongInt(Endp);
 Prints("缓冲区");
 PrintLongInt(Len);    //写入的字节数
 Prints("字节。\r\n");
#endif
 UENUM=Endp; //选择端点
 for(i=0;i<Len;i++)
 {
  UEDATX=*(Buf+i); //将数据写到FIFO中
#ifdef DEBUG1
  PrintHex(*(Buf+i));  //如果需要显示调试信息，则显示发送的数据
  if(((i+1)%16)==0)Prints("\r\n"); //每16字节换行一次
#endif
  }
#ifdef DEBUG1
 if((Len%16)!=0)Prints("\r\n"); //换行
#endif
 UsbChipValidateBuffer(Endp); //使端点数据有效
 return Len; //返回Len
}
////////////////////////End of function//////////////////////////////

