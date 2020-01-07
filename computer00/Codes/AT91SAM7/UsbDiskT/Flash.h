/******************************************************************
  本程序只供学习使用，未经作者许可，不得用于其它任何用途
  
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org
                             
 Flash.h  file
    
作者：Computer-lov
建立日期: 2009-03-18
修改日期: 2009-03-18
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#ifndef __FLASH_H__
#define __FLASH_H__

#include "MyType.h"

#define FlashGetRB() ((*AT91C_PIOA_PDSR)&(1<<29))

#define FlashGetData() (((*AT91C_PIOA_PDSR)>>17)&0xFF)

/*
#define FlashSetData(Val) {*AT91C_PIOA_CODR=(0xFF<<17); \
                           *AT91C_PIOA_SODR=(((uint32)((Val)&0xFF))<<17);}
*/

#define FlashSetData(Val) {*AT91C_PIOA_ODSR=(((uint32)(Val))<<17);}

#define FlashSetPortAsIn()  *AT91C_PIOA_ODR=(0xFF<<17)
#define FlashSetPortAsOut() *AT91C_PIOA_OER=(0xFF<<17)

#define FlashSetCle() *AT91C_PIOA_SODR=(1<<15)
#define FlashClrCle() *AT91C_PIOA_CODR=(1<<15)

#define FlashSetAle() *AT91C_PIOA_SODR=(1<<2)
#define FlashClrAle() *AT91C_PIOA_CODR=(1<<2)

#define FlashSetWe()  *AT91C_PIOA_SODR=(1<<26)
#define FlashClrWe()  *AT91C_PIOA_CODR=(1<<26)

#define FlashSetRe()  *AT91C_PIOA_SODR=(1<<27)
#define FlashClrRe()  *AT91C_PIOA_CODR=(1<<27)

#define FlashSetCe()  *AT91C_PIOA_SODR=(1<<28)
#define FlashClrCe()  *AT91C_PIOA_CODR=(1<<28)

#define FlashWait() while(FlashGetRB()==0)

//读一字节
#define FlashReadByte(Val) {FlashClrRe(); \
                            Val=FlashGetData(); \
                            FlashSetRe();}

//写一字节
#define FlashWriteByte(Val) {FlashClrWe(); \
                             FlashSetData((Val)); \
                             FlashSetWe();}


//FLASH最大能够达到的地址，是128M
#define FLASH_MAX_ADDR 0x7FFFFFF

//FLASH块大小，为128KB
#define FLASH_BLOCK_SIZE 0x20000

//FLASH页大小，为2KB
#define FLASH_PAGE_SIZE 0x800


//扇区大小
#define FLASH_SECTOR_SIZE 0x200

//用做交换区的块数
#define FLASH_SWAP_BLOCKS 10

//用做坏块重新影射的块数
#define FLASH_BAD_BLOCKS_REMAP 50

//保存坏块表的块数
#define FLASH_BLOCKS_TABLE 3

//最大能够达到的扇区地址
#define FLASH_MAX_SECTOR_ADDR (FLASH_MAX_ADDR-(FLASH_BAD_BLOCKS_REMAP+FLASH_SWAP_BLOCKS+FLASH_BLOCKS_TABLE)*FLASH_BLOCK_SIZE)

//交换区的起始地址
#define FLASH_SWAP_BLOCK_ADDR (FLASH_MAX_ADDR+1-FLASH_SWAP_BLOCKS*FLASH_BLOCK_SIZE)

//重影射坏块的起始地址
#define FLASH_BAD_BLOCK_REMAP_ADDR (FLASH_MAX_ADDR+1-(FLASH_BAD_BLOCKS_REMAP+FLASH_SWAP_BLOCKS)*FLASH_BLOCK_SIZE)

//定义保存坏块表的起始地址
#define FLASH_BLOCK_TABLE_ADDR (FLASH_MAX_ADDR+1-(FLASH_BAD_BLOCKS_REMAP+FLASH_SWAP_BLOCKS+FLASH_BLOCKS_TABLE)*FLASH_BLOCK_SIZE)

//影射区的状态常量定义
#define FLASH_BLOCK_OK    0
#define FLASH_BLOCK_BAD   1
#define FLASH_BLOCK_USED  2

void FlashReset(void);
void FlashLoadBadBlockTable(void);
void FlashInit(void);
uint8 FlashEraseBlock(uint32 Addr);
uint32 FlashManageSwapBlock(uint32 Op);
uint32 FlashWriteOneSector(uint32 Addr, uint8 * pBuf, uint32 Remain);
uint32 FlashReadOneSector(uint32 Addr, uint8 * pBuf, uint32 Remain);
void FlashReadId(uint8 *Buf);
void FlashSaveBadBlockTable(void);


/********************************************************************
函数功能：获取下一个可用的交换块地址。
入口参数：无。
返    回：下一个可用的交换块的地址。
备    注：无。
********************************************************************/
#define FlashSwapBlockInit()  FlashManageSwapBlock(0)
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：获取下一个可用的交换块地址。
入口参数：无。
返    回：下一个可用的交换块的地址。
备    注：无。
********************************************************************/
#define FlashGetNextSwapBlock()  FlashManageSwapBlock(1)
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：获取当前正在使用的交换块地址。
入口参数：无。
返    回：下一个可用的交换块的地址。
备    注：无。
********************************************************************/
#define FlashGetCurrentSwapBlock()  FlashManageSwapBlock(2)
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：标志当前交换块为坏块。
入口参数：无。
返    回：下一个可用的交换块的地址。
备    注：无。
********************************************************************/
#define FlashMarkBadCurrentSwapBlock()  FlashManageSwapBlock(3)
/////////////////////////End of function/////////////////////////////

extern uint8 FlashSectorBuf[FLASH_SECTOR_SIZE];

#endif