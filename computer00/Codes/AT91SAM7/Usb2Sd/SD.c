/******************************************************************
  本程序只供学习使用，未经作者许可，不得用于其它任何用途
  
        欢迎访问我的USB专区：http://group.ednchina.com/93/
		欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
		                     http://computer00.21ic.org
							 
 SD.c  file
	
作者：Computer-lov
建立日期: 2009-04-07
修改日期: 2009-04-08
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include "SD.h"
#include <AT91SAM7S64.H>

uint8 SdCid[16];  //保存16字节的CID
uint8 SdCsd[16];  //保存16字节的CSD
uint32 SdCapacity; //保存容量，注意单位为扇区数
uint32 SdNAC;  //保存NAC的数目
uint8 SdSectorBuf[SECTOR_SIZE]; //读写扇区用的缓冲区
uint8 SdMulBlockReadCmdSent; //是否发送过多块读命令的标志
uint8 SdMulBlockWriteCmdSent; //是否发送过多块写命令的标志

/********************************************************************
函数功能：SPI初始化。
入口参数：无。
返    回：无。
备    注：设置相应的IO模式，配置SPI模式。
********************************************************************/
void SpiInit(void)
{
 //PA11先作为普通IO口，并设置为输入口，产生高电平
 *AT91C_PIOA_PER=0x01<<11;
 //禁止输出，靠上拉电阻拉高
 *AT91C_PIOA_ODR=0x01<<11;
 //PA12,PA13,PA14 作为外设使用
 *AT91C_PIOA_PDR=(0x0E<<11);
 //禁止PA11,PA12,PA13,PA14 IO中断
 *AT91C_PIOA_IDR=(0x0F<<11);
 //使能PA11,PA12,PA13,PA14上拉电阻
 *AT91C_PIOA_PPUER=(0x0F<<11);
 //连接到A外设(即SPI口)
 *AT91C_PIOA_ASR=(0x0F<<11);
 *AT91C_SPI_CR=(1<<0);   //SPI Enable
 *AT91C_SPI_MR= (1<<0)   //SPI is in Master mode,
               |(0<<1)   //Fixed Peripheral Select,
               |(0<<2)   //The chip selects are directly
                         //connected to a peripheral device.
               |(1<<4)   //Mode fault detection is disabled.
               |(0<<7)   //Local loopback path disabled.
               |(0<<16)  //Peripheral Chip Select = 0
               |(6<<24);  //Delay Between Chip Selects = 6
 *AT91C_SPI_IDR=0x3F;  //禁止所有SPI中断
 AT91C_SPI_CSR[0]= (1<<0) //CPOL=1,
                  |(0<<1) //NCPHA=0
                  |(1<<3) //The Peripheral Chip Select does
                          //not rise after the last transfer is achieved.
                  |(0<<4) //8bits Per Transfer
                  |(2<<8) //SCBR: Serial Clock Baud Rate = 2.
                          //SPI Clock is 48MHz/2=24MHz
                  |(0<<16) //DLYBS: Delay Before SPCK = 0
                  |(0<<24);//DLYBCT: Delay Between Consecutive Transfers = 0
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：通过SPI发送一字节数据，并返回读出的数据。
入口参数：Value: 要传输的数据。
返    回：读出的一字节数据。
备    注：无。
********************************************************************/
uint8 SpiWriteReadByte(uint8 Value)
{
 *AT91C_SPI_TDR=Value;  //发送数据
 while(!((*AT91C_SPI_SR)&0x01));  //等待数据接收完毕
 return (uint8)(*AT91C_SPI_RDR);
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：获取响应状态。
入口参数：无。
返    回：状态。
备    注：无。
********************************************************************/
uint8 SdReadResponse(void)
{
 uint32 i;
 uint8 Res;
 for(i=0;i<9;i++)  //最多等待8个数据
 {
  Res=SpiWriteReadByte(0xFF);
  if(!(Res&0x80))return Res;  //如果最高位为0，则返回
 }
 return 0xFF;  //如果读响应超时，则返回0xFF
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：等待数据读出。
入口参数：无。
返    回：状态：0xFF：超时错误；0xFE：完成。
备    注：无。
********************************************************************/
uint8 SdWaitRead(void)
{
 uint32 i;
 uint8 Res;
 for(i=0;i<SdNAC;i++)  //等待数据读出
 {
  Res=SpiWriteReadByte(0xFF);
  if(Res==0xFE)return Res;  //如果为0xFE，则返回
 }
 return 0xFF;  //如果读响应超时，则返回0xFF
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：SD卡部分初始化。
入口参数：无。
返    回：无。
备    注： 初始化SPI，写CMD0进入到SPI模式。
********************************************************************/
void SdInit(void)
{
 uint32 i;
 uint8 Res;
 
 SpiInit(); //芯片的SPI控制器初始化
 
 for(i=0;i<16;i++) //初始化CID和CSD
 {
  SdCid[i]=0;
  SdCsd[i]=0;
 }
 
 //初始化都没有发送过读、写命令
 SdMulBlockReadCmdSent=0;
 SdMulBlockWriteCmdSent=0;
 
 //初始化容量为0
 SdCapacity=0;
 
 for(i=0;i<100/8;i++) //在CS0为1状态下发送大于74个全1时钟
 {
  SpiWriteReadByte(0xFF);
 }
 *AT91C_PIOA_PDR=(0x01<<11);    //PA11切换到SPI状态
 
 //发送CMD0命令，此时CS为0，卡将进入SPI模式
 SpiWriteReadByte(0x40|0);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x95);
 Res=SdReadResponse();
 SpiWriteReadByte(0xFF); //发送一字节填充时钟 
 if(Res!=0x01) return;  //此时应该返回0x01，如果不是，则返回
 //尝试激活卡
 while(1)
 {
  //发送CMD55命令，切换到用户命令
  SpiWriteReadByte(0x40|55);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0xFF);
  Res=SdReadResponse();
  SpiWriteReadByte(0xFF); //发送一字节填充时钟  

  //发送ACMD41命令，激活卡
  SpiWriteReadByte(0x40|41);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0xFF);
  Res=SdReadResponse();
  SpiWriteReadByte(0xFF); //发送一字节填充时钟
  if(Res==0x00) break;  //如果为0，则表示卡已激活
  if(Res!=0x01) return; //其它状态，出现错误，返回
 }
 
 SdNAC=9; //初始化设置为NCR的值
 //读卡的CSD
 //发送CMD9命令，准备读取CSD
 SpiWriteReadByte(0x40|9);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0xFF);
 Res=SdReadResponse();
 if(Res!=0x00) return;  //此时应该返回0x00，如果不是，则返回
 if(SdWaitRead()!=0xFE)return; //如果超时，则返回
 for(i=0;i<16;i++) //读取16字节的CSD数据
 {
  SdCsd[i]=SpiWriteReadByte(0xFF);
 }
 for(i=0;i<3;i++) //读取2字节CRC及发送一字节填充时钟
 {
  SpiWriteReadByte(0xFF);
 }
 
 //计算容量（扇区数）
 SdCapacity=1;
 for(i=0;i<((SdCsd[5]&0x0F)-9);i++) //每个扇区的大小(READ_BL_LEN)
 {
  SdCapacity*=2;
 }
 for(i=0;i<(((SdCsd[9]&0x03)<<1)+((SdCsd[10]>>7)&0x01)+2);i++) //C_SIZE_MULT
 {
  SdCapacity*=2;
 }
 SdCapacity*=(((uint32)(SdCsd[6]&0x03))<<10)
            +(((uint32)SdCsd[7])<<2)
            +(((SdCsd[8]>>6)&0x03))
            +1;
            
 //计算延时TAAC
 SdNAC=1;
 for(i=0;i<(SdCsd[1]&0x07);i++)
 {
  SdNAC*=10;
 }
 switch((SdCsd[1]>>3)&0x0F)
 {
  case 0:
  case 1:
  break;
  case 2:
   SdNAC*=1.2;
  break;
  case 3:
   SdNAC*=1.3;
  break;
  default:
   SdNAC*=(((SdCsd[1]>>3)&0x0F)-1)*0.5;
  break;
 }
 //SdNAC单位为ns，需要将其换算为字节数
 SdNAC/=1000; //换算为us
 SdNAC*=24/8;   //时钟为24MHz，每个字节8个时钟
 //还要加上NSAC
 SdNAC+=100*SdCsd[2]/8;

 //读卡的CID
 //发送CMD10命令，准备读取CID
 SpiWriteReadByte(0x40|10);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0x00);
 SpiWriteReadByte(0xFF);
 Res=SdReadResponse();
 if(Res!=0x00) return;  //此时应该返回0x00，如果不是，则返回
 if(SdWaitRead()!=0xFE)return; //如果超时，则返回
 for(i=0;i<16;i++) //读取16字节的CID数据
 {
  SdCid[i]=SpiWriteReadByte(0xFF);
 }
 for(i=0;i<3;i++) //读取2字节CRC及发送一字节填充时钟
 {
  SpiWriteReadByte(0xFF);
 }
 
 //设置块长度，为SECTOR_SIZE字节
 //发送CMD16命令，设置块长度
 SpiWriteReadByte(0x40|16);
 SpiWriteReadByte((SECTOR_SIZE>>24)&0xFF);
 SpiWriteReadByte((SECTOR_SIZE>>16)&0xFF);
 SpiWriteReadByte((SECTOR_SIZE>>8)&0xFF);
 SpiWriteReadByte((SECTOR_SIZE)&0xFF);
 SpiWriteReadByte(0xFF);
 Res=SdReadResponse();
 SpiWriteReadByte(0xFF); //发送一字节填充时钟 
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：将缓冲区中的一扇区数据写入到SD卡中。
入口参数：Addr: 字节地址；pBuf：保存数据的缓冲区；
          Remain：预计接下来还需要写多少扇区
返    回：写的状态。0：成功。非0：失败。
备    注：当Remain不为0时，将直接写入数据。当Remain为0时，
          将发送结束写命令。
********************************************************************/
uint32 SdWriteOneSector(uint32 Addr, uint8 * pBuf, uint32 Remain)
{
 uint32 i;
 uint8 Res;
 
 //如果之前没有发送多块写命令，则发送一个
 if(!SdMulBlockWriteCmdSent)
 {
  SdMulBlockWriteCmdSent=0xFF; //标志块写为已发送
  //发送CMD25命令，写多个块
  SpiWriteReadByte(0x40|25);
  SpiWriteReadByte((Addr>>24)&0xFF);
  SpiWriteReadByte((Addr>>16)&0xFF);
  SpiWriteReadByte((Addr>>8)&0xFF);
  SpiWriteReadByte((Addr)&0xFF);
  SpiWriteReadByte(0xFF);
  Res=SdReadResponse();
  SpiWriteReadByte(0xFF); //NWR
  //此时应该返回0x00，如果不是，则返回失败码1
  if(Res!=0x00)
  {
   return 0x01;
  }
 }

 //写Start Block
 SpiWriteReadByte(0xFC);
 //写数据
 for(i=0;i<SECTOR_SIZE;i++)
 {
  SpiWriteReadByte(pBuf[i]); //写一字节数据
 }
 //写2字节的CRC16
 SpiWriteReadByte(0xFF);
 SpiWriteReadByte(0xFF);
 Res=SpiWriteReadByte(0xFF); //读回响应
 SpiWriteReadByte(0xFF); //发送一字节填充时钟
 while(SpiWriteReadByte(0xFF)!=0xFF); //当卡忙时，等待
 if(Remain==0) //如果是最后一次写，则发送停止写Stop Tran
 {
  SdMulBlockWriteCmdSent=0x00; //标志块写命令为未发送
  //发送Stop Tran，停止写操作
  SpiWriteReadByte(0xFD);
  SpiWriteReadByte(0xFF); //发送一字节填充时钟
  while(SpiWriteReadByte(0xFF)!=0xFF); //等待忙结束
 }
 if((Res&0x1F)!=0x05)
  return 0x02; //返回失败代码2
 return 0;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：从Sd卡中读出一扇区数据保存到缓冲区中。
入口参数：Addr: 字节地址；pBuf：保存数据的缓冲区；
          Remain：预计接下来还需要读多少扇区
返    回：读取的状态。0：成功。非0：失败。
备    注：当Remain不为0时，将直接读取数据。当Remain为0时，将发送结束读命令。
********************************************************************/
uint32 SdReadOneSector(uint32 Addr, uint8 * pBuf, uint32 Remain)
{
 uint32 i;
 uint8 Res;
 //如果之前没有发送多块读命令，则发送一个
 if(!SdMulBlockReadCmdSent)
 {
  SdMulBlockReadCmdSent=0xFF; //标志块读命令为已发送
  //发送CMD18命令，读多个块
  SpiWriteReadByte(0x40|18);
  SpiWriteReadByte((Addr>>24)&0xFF);
  SpiWriteReadByte((Addr>>16)&0xFF);
  SpiWriteReadByte((Addr>>8)&0xFF);
  SpiWriteReadByte((Addr)&0xFF);
  SpiWriteReadByte(0xFF);
  Res=SdReadResponse();
  //此时应该返回0x00，如果不是，则返回失败码1
  if(Res!=0x00) return 0x01;
 }
 //等待数据返回，如果超时，则返回2
 if(SdWaitRead()!=0xFE)return 0x02;
 for(i=0;i<SECTOR_SIZE;i++)
 {
  pBuf[i]=SpiWriteReadByte(0xFF); //读取一字节数据
 }
 //读回2字节的CRC16
 SpiWriteReadByte(0xFF);
 SpiWriteReadByte(0xFF);
 if(Remain==0) //如果是最后一次读，则发送停止读命令CMD12
 {
  SdMulBlockReadCmdSent=0x00; //标志块读命令为未发送
  //发送CMD12命令，停止读操作
  SpiWriteReadByte(0x40|12);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0x00);
  SpiWriteReadByte(0xFF);
  SpiWriteReadByte(0xFF); //发送一字节填充时钟
  Res=SdReadResponse();
  SpiWriteReadByte(0xFF); //发送一字节填充时钟
  while(SpiWriteReadByte(0xFF)!=0xFF); //等待忙结束
  //此时应该返回0x00，如果不是，则返回失败码1
  if(Res!=0x00) return 0x01;
 }
 return 0;
}
/////////////////////////End of function/////////////////////////////

