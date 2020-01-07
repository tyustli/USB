/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

AT90USB.H  file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2009-05-10
版本：V1.1
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/


#ifndef __AT90USB_H__
#define __AT90USB_H__

#include "MyType.h"

//函数声明
void UsbDisconnect(void); //USB断开连接
void UsbConnect(void);    //USB连接
void UsbChipInit(void);
void UsbChipResetEndpoint(void);
void UsbChipSetConfig(uint8 Value);
void UsbChipWriteAddress(uint8 Addr);
void UsbChipSetAddressStatus(uint8 Value);
int  UsbChipIsSetup(uint8 Endp);
uint8 UsbChipReadEndpointBuffer(uint8 Endp, uint8 Len, uint8 *Buf);
void UsbChipClearBuffer(uint8 Endp);
void UsbChipAcknowledgeSetup(uint8 Endp);
void UsbChipSetEndpointEnable(uint8 Enable);

//注意CodeVision AVR不会自动转换指针类型，所以要写两个函数
uint8 DataInRam_WriteEndpointBuffer(uint8 Endp, uint8 Len, uint8 *Buf);
uint8 DataInFlash_WriteEndpointBuffer(uint8 Endp, uint8 Len, flash uint8 *Buf);

#define UsbChipWriteEndpointBuffer(Endp,Len,Buf) DataInRam_WriteEndpointBuffer(Endp,Len,Buf)

#endif
