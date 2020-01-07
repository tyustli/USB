/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

UART.H  file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2009.05.11
版本：V1.1
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/

#include "config.h"
#include "MyType.h"

#define BUF_LEN 64
extern uint8 UartBuffer[BUF_LEN];
extern uint8 UsbEp4Buffer[BUF_LEN];
extern uint8 UartBufferOutputPoint;
extern uint8 UartBufferInputPoint;
extern uint8 UartByteCount;
extern uint8 UsbEp4ByteCount;
extern uint8 UsbEp4BufferOutputPoint;

uint32 UartSetBitRate(uint32 NewBitRate);

#ifndef __UART_C__
#define __UART_C__

void Uart1Init(void);
void UartPutChar(uint8);

#if (defined DEBUG0)||(defined DEBUG1)

void Prints(flash uint8 *);
void PrintLongInt(uint32);
void PrintShortIntHex(uint16 x);
void Printc(uint8);
void PrintHex(uint8 x);
void PrintLongIntHex(uint32 x);

#endif

#endif
