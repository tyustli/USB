/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

AT91SAMxUSB.C  file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2009.03.08
版本：V1.1
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/

#include <AT91SAM7S64.H>
#include "MyType.h"
#include "AT91SAMxUSB.H"
#include "config.h"
#include "UART.h"
#include "UsbCore.h"

unsigned char CurrentBank[4];  //保存当前读的是哪个bank的数据


#define REG_NO_EFFECT_1_ALL 0x4F
/********************************************************************
函数功能：设置CSR寄存器对应的位。
入口参数：endpoint：端点号；flags：要设置的位。
返    回：无。
备    注：无。
********************************************************************/
void SetCsr(int endpoint, unsigned int flags)
{
 volatile unsigned int reg;
 reg = AT91C_UDP_CSR[endpoint];
 reg|= REG_NO_EFFECT_1_ALL;
 reg|= (flags);
 AT91C_UDP_CSR[endpoint] = reg;
 while((AT91C_UDP_CSR[endpoint]&(flags))!=(flags));
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：清除CSR寄存器对应的位。
入口参数：endpoint：端点号；flags：要设置的位。
返    回：无。
备    注：无。
********************************************************************/
void ClrCsr(int endpoint, unsigned int flags)
{
 volatile unsigned int reg;
 reg = AT91C_UDP_CSR[endpoint];
 reg|= REG_NO_EFFECT_1_ALL;
 reg&= ~(flags);
 AT91C_UDP_CSR[endpoint] = reg;
 while((AT91C_UDP_CSR[endpoint]&(flags))==(flags));
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：延时x毫秒函数。
入口参数：毫秒数。
返    回：无。
备    注：无。
********************************************************************/
void DelayXms(unsigned long int x)
{
 unsigned long int i;
 while(x--)
 {
  i=6868;
  while(--i);
 }
}
/////////////////////////End of function/////////////////////////////

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
 *AT91C_PIOA_SODR=(1<<16); //Disconnect pull-up resistor
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
 *AT91C_PIOA_CODR=(1<<16); //Connect pull-up resistor
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
 *AT91C_UDP_TXVC=0; //enable the transceiver
 *AT91C_UDP_IDR=0xFFFFFFFF; //disable all USB interrupts
 
 *AT91C_PIOA_PER=(1<<16);  //Enable PA16
 *AT91C_PIOA_OER=(1<<16);  //Output Enable PA16
 
 ConfigValue=0; //配置值初始化为0 
 
 UsbDisconnect();  //先断开USB连接
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
 *AT91C_UDP_RSTEP=0x0E;  //复位端点
 *AT91C_UDP_RSTEP=0x00;  //复位端点完成
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
 if(Value==0)
 {
  *AT91C_UDP_GLBSTATE&=~(1<<1);  //配置值为0，清除配置
 }
 else
 {
  *AT91C_UDP_GLBSTATE|=(1<<1);   //配置值非0，设置配置
 }
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
  *AT91C_UDP_GLBSTATE&=~(1<<0);  //默认状态
 }
 else
 {
  *AT91C_UDP_GLBSTATE|=(1<<0);  //进入到设置地址阶段
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
 //等待前一个数据包（实际上是状态阶段）发送完毕
 //等待中断产生
 while(1)
 {
  if((*AT91C_UDP_ISR)&(1<<0))break; //发送完毕
  if((*AT91C_UDP_ISR)&((1<<8)|(1<<12)))return; //如果产生复位、挂起则直接返回
 }

 *AT91C_UDP_FADDR=(1<<8)|Addr;  //使能功能端点并设置地址
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
  SetCsr(0,1<<15); //使能端点0
  SetCsr(1,(6<<8)|(1<<15)); //使能端点1，并设置为批量IN端点
  SetCsr(2,(2<<8)|(1<<15)); //使能端点2，并设置为批量OUT端点
  SetCsr(3,(7<<8)|(1<<15)); //使能端点3，并设置为中断IN端点
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
 if((AT91C_UDP_CSR[Endp])&(1<<2))
 {
  return 0xFF;  //是setup包，返回非0
 }
 else
 {
  return 0;
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
 if(CurrentBank[Endp]==0) //清除BANK0
 {
  ClrCsr(Endp,1<<1);//将RX_DATA_BK0置0
 }
 else
 {
  ClrCsr(Endp,1<<6); //将RX_DATA_BK1置0
 }
}
////////////////////////End of function//////////////////////////////

extern uint8 Buffer[16];  //读端点0用的缓冲区
/********************************************************************
函数功能：应答建立包的函数。
入口参数：Endp:端点号。
返    回：无。
备    注：无。
********************************************************************/
void UsbChipAcknowledgeSetup(uint8 Endp)
{
 SetCsr(Endp,1<<7); //设置DIR位
 ClrCsr(Endp,1<<2); //清除RX SETUP位
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

#ifdef DEBUG2
 Prints("UDP_CSR");
 Printc('0'+Endp);
 Prints(" is ");
 PrintLongIntHex(AT91C_UDP_CSR[Endp]);
 Prints("\r\n");
#endif
 
 if(!UsbChipIsSetup(Endp))  //如果不是SETUP包，则要检查是哪个端点缓冲区
 {
  switch(AT91C_UDP_CSR[Endp]&((1<<1)|(1<<6)))
  {
   case ((1<<1)|(1<<6)): //两个缓冲区都满了
    if(CurrentBank[Endp]==0)  //如果前面读的是BANK0，那么本次读BANK1
    {
     CurrentBank[Endp]=1;
    }
    else   //如果前面读的是BANK1，那么本次读BANK0
    {
     CurrentBank[Endp]=0;
    }
   break;
   case (1<<1): //如果只是BANK0满，那么就读它
    CurrentBank[Endp]=0;
   break;
   case (1<<6): //如果只是BANK1满，那么就读它
    CurrentBank[Endp]=1;
   break;
   default:  //没有缓冲区有数据，则返回0，没有数据
   return 0;
  }
 }
 
 j=(AT91C_UDP_CSR[Endp])>>16; //获取数据长度
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
  *(Buf+i)=AT91C_UDP_FDR[Endp];  //从FIFO中读一字节数据
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
 SetCsr(Endp,1<<4);//TXPKTRDY置1，使能发送数据
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：将数据写入端点缓冲区函数。
入口参数：Endp：端点号；Len：需要发送的长度；Buf：保存数据的缓冲区。
返    回：Len的值。
备    注：无。
********************************************************************/
uint8 UsbChipWriteEndpointBuffer(uint8 Endp,uint8 Len,uint8 * Buf)
{
 uint8 i;
 
#ifdef DEBUG1 //如果定义了DEBUG1，则需要显示调试信息
 Prints("写端点");
 PrintLongInt(Endp);
 Prints("缓冲区");
 PrintLongInt(Len);    //写入的字节数
 Prints("字节。\r\n");
#endif
 for(i=0;i<Len;i++)
 {
  AT91C_UDP_FDR[Endp]=*(Buf+i); //将数据写到FIFO中
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

