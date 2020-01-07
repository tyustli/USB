/******************************************************************
  本程序只供学习使用，未经作者许可，不得用于其它任何用途
  
        欢迎访问我的USB专区：http://group.ednchina.com/93/
		欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
		                     http://computer00.21ic.org
							 
 SD.h  file
	
作者：Computer-lov
建立日期: 2009-04-07
修改日期: 2009-04-07
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#ifndef __SD_H__
#define __SD_H__

#include "my_type.h"

#define SECTOR_SIZE 512  //扇区大小为512字节

extern uint8 SdSectorBuf[SECTOR_SIZE]; //读写扇区用的缓冲区
extern uint32 SdCapacity; //保存容量，注意单位为扇区数

void SdInit(void);
uint32 SdReadOneSector(uint32 Addr, uint8 * pBuf, uint32 Remain);
uint32 SdWriteOneSector(uint32 Addr, uint8 * pBuf, uint32 Remain);

#endif