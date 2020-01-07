/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

UART.C  file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2009.03.07
版本：V1.1
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/

#include <AT91SAM7S64.H>
#include "UART.h"
#include "MyType.h"
#include "config.h"
#include "Led.h"

volatile uint8 Sending;

/********************************************************************
函数功能：串口中断处理。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void Uart0Isr(void) __irq
{
 unsigned char dat;
 if(*AT91C_AIC_ISR!=6)return; //不是该中断源，直接返回
 //*AT91C_AIC_ICCR=1<<6; //使用了向量中断，会自动清除中断，这里不用  
 //清除串口0状态标志
 if(!((*AT91C_US0_CSR)&(1<<0)))return; //不是接收完毕中断，返回
 dat=(unsigned char)(*AT91C_US0_RHR);  //读取数据
 *AT91C_AIC_EOICR=0;    //中断处理完毕
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：串口初始化。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void Uart0Init(void)
{
 //PIO模块
 *AT91C_PIOA_PDR=(1<<5)|(1<<6);    //PA5,PA6 作为外设使用
 *AT91C_PIOA_IDR=(1<<5)|(1<<6);    //禁止PA5、PA6 IO中断
 *AT91C_PIOA_PPUER=(1<<5)|(1<<6);  //使能PA5、PA6上拉电阻
 *AT91C_PIOA_ASR=(1<<5)|(1<<6);    //连接到A外设（即串口）
 
 //UART模块
 *AT91C_US0_CR = (1<<2)    //Reset Receiver
                |(1<<3)    //Reset Transmitter
                |(1<<8);   //Reset Status Bits
                
 *AT91C_US0_CR = (1<<4)    //Receiver Enable
                |(1<<6);   //Transmitter Enable
               
 *AT91C_US0_MR = (0<<0)    //Normal
                |(0<<4)    //MCK
                |(3<<6)    //8 bits
                |(0<<8)    //USART operates in Asynchronous Mode.
                |(4<<9)    //No parity
                |(0<<13)   //1 stop bit
                |(0<<14)   //Normal Mode
                |(0<<16)   //Least Significant Bit is sent/received first.
                |(0<<17)   //CHRL defines character length.
                |(0<<18)   //The USART does not drive the SCK pin.
                |(0<<19)   //16x Oversampling.
                |(1<<20);  //The NACK is not generated.
               
 *AT91C_US0_IER = (1<<0);    //RXRDY Interrupt Enable
 
 *AT91C_US0_BRGR = (Fmck/16/BitRate);  //产生指定的波特率
 
 *AT91C_US0_RTOR = 0;  //The Receiver Time-out is disabled.
 
 *AT91C_US0_TTGR = 0;  //The Transmitter Timeguard is disabled.
 
 
 //中断控制模块
 //初始化中断源6（串口0中断）
 AT91C_AIC_SMR[6] = (0<<0)  //The priority level = 0(lowest)
                   |(3<<5); //Positive edge triggered
 
 AT91C_AIC_SVR[6]=(int)Uart0Isr;  //set ISR entrance address
 
 *AT91C_AIC_IECR=(1<<6);  //Enable source ID 6 interrupt
}
////////////////////////End of function//////////////////////////////


/********************************************************************
函数功能：往串口发送一字节数据。
入口参数：d: 要发送的字节数据。
返    回：无。
备    注：无。
********************************************************************/
void UartPutChar(uint8 d)
{
 while(!((*AT91C_US0_CSR)&(1<<1))); //等待发送完毕
 *AT91C_US0_THR=d;
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：发送一个字符串。
入口参数：pd：要发送的字符串指针。
返    回：无。
备    注：无。
********************************************************************/
void Prints(uint8 * pd)
{
 while((*pd)!='\0') //发送字符串，直到遇到0才结束
 {
  UartPutChar(*pd); //发送一个字符
  pd++;  //移动到下一个字符
 }
}
////////////////////////End of function//////////////////////////////

#ifdef DEBUG1

/********************************************************************
函数功能：将整数转按十进制字符串发送。
入口参数：x：待显示的整数。
返    回：无。
备    注：无。
********************************************************************/
void PrintLongInt(uint32 x)
{
 int8 i;
 uint8 display_buffer[10];

 for(i=9;i>=0;i--)
 {
  display_buffer[i]='0'+x%10;
  x/=10;
 }
 for(i=0;i<9;i++)
 {
  if(display_buffer[i]!='0')break;
 }
 for(;i<10;i++)UartPutChar(display_buffer[i]);
}
////////////////////////End of function//////////////////////////////

#endif

const uint8 HexTable[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
/********************************************************************
函数功能：将短整数按十六进制发送。
入口参数：待发送的整数。
返    回：无。
备    注：无。
********************************************************************/
void PrintShortIntHex(uint16 x)
{
 uint8 i;
 uint8 display_buffer[7];
 display_buffer[6]=0;
 display_buffer[0]='0';
 display_buffer[1]='x';
 for(i=5;i>=2;i--) //将整数转换为4个字节的HEX值
 {
  display_buffer[i]=HexTable[(x&0xf)];
  x>>=4;
 }
 Prints(display_buffer);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：将长整数按十六进制发送。
入口参数：待发送的整数。
返    回：无。
备    注：无。
********************************************************************/
void PrintLongIntHex(uint32 x)
{
 uint8 i;
 uint8 display_buffer[11];
 display_buffer[10]=0;
 display_buffer[0]='0';
 display_buffer[1]='x';
 for(i=9;i>=2;i--) //将整数转换为4个字节的HEX值
 {
  display_buffer[i]=HexTable[(x&0xf)];
  x>>=4;
 }
 Prints(display_buffer);
}
////////////////////////End of function//////////////////////////////

#if (defined DEBUG0)||(defined DEBUG1)
/********************************************************************
函数功能：发送一个byte的数据。
入口参数：待发送的数据。
返    回：无。
备    注：无。
********************************************************************/
void Printc(uint8 x)
{
 UartPutChar(x);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：以HEX格式发送一个byte的数据。
入口参数：待发送的数据
返    回：无。
备    注：无。
********************************************************************/
void PrintHex(uint8 x)
{
 Printc('0');
 Printc('x');
 Printc(HexTable[x>>4]);
 Printc(HexTable[x&0xf]);
 Printc(' ');
}
////////////////////////End of function//////////////////////////////
#endif
