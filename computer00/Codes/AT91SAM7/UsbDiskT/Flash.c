/******************************************************************
  本程序只供学习使用，未经作者许可，不得用于其它任何用途
  
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org
                             
 Flash.c  file
    
作者：Computer-lov
建立日期: 2009-03-18
修改日期: 2009-03-26
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <AT91SAM7S64.H>
#include "Flash.h"
#include "MyType.h"

uint32 FlashCurrentWriteSectorAddr; //当前写的扇区地址
uint32 FlashCurrentReadSectorAddr;  //当前读的扇区地址
uint32 FlashNeedWriteBack; //需要回写的标志

uint8 FlashSectorBuf[FLASH_SECTOR_SIZE]; //读写扇区用的缓冲区

static uint32 FlashBadBlockTable[2][FLASH_BAD_BLOCKS_REMAP+1]; //用来管理坏块的表格
static uint32 FlashBadBlocksCount;  //有多少个坏块
static uint8 FlashRemapBlockStatus[FLASH_BAD_BLOCKS_REMAP+1]; //用来重影射用的块的状况
static uint32 FlashLastAccessAddr; //最后一次访问过的地址

/********************************************************************
函数功能：FLASH写命令。
入口参数：Val：要写的命令值。
返    回：无。
备    注：没有片选，必须在其他更高层的函数负责片选。
********************************************************************/
void FlashWriteCommand(uint8 Cmd)
{
 FlashSetCle(); //选择为命令
 FlashSetPortAsOut();
 FlashWriteByte(Cmd);
 FlashSetPortAsIn();
 FlashClrCle();
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：往FLASH写入四字节地址的函数。
入口参数：Addr：要写入的字节地址。
返    回：无。
备    注：没有片选，必须在其他更高层的函数负责片选。注意地址中间的0。
********************************************************************/
void FlashWriteAddr4Byte(uint32 Addr)
{
 uint i;
 
 //注意：该FLASH的地址中间有5bit必须置0
 //并将高16位左移5位
 Addr=((Addr<<5)&(~0xFFFF))|(Addr&0x07FF);

 FlashSetAle();  //ALE置高，选择为地址
 FlashSetPortAsOut(); //设置端口为输出状态
 for(i=0;i<4;i++) //写入四字节的地址
 {
  FlashWriteByte(Addr); //写一字节数据
  Addr>>=8;
 }
 FlashSetPortAsIn(); //设置端口为输入
 FlashClrAle();  //ALE置低
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：往FLASH写入二字节地址的函数。
入口参数：Addr：要写入的字节地址。
返    回：无。
备    注：没有片选，必须在其他更高层的函数负责片选。只写地址低位
********************************************************************/
void FlashWriteAddr2Byte(uint32 Addr)
{
 uint i;
 
 //只需要低11位
 Addr=Addr&(FLASH_PAGE_SIZE-1);

 FlashSetAle();  //ALE置高，选择为地址
 FlashSetPortAsOut(); //设置端口为输出状态
 for(i=0;i<2;i++)     //写入二字节的地址
 {
  FlashWriteByte(Addr);  //写一字节数据
  Addr>>=8;
 }
 FlashSetPortAsIn();  //设置端口为输入
 FlashClrAle();       //ALE置低
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：往FLASH写入二字节页地址的函数。
入口参数：Addr：要写入的字节地址。
返    回：无。
备    注：没有片选，必须在其他更高层的函数负责片选。
********************************************************************/
void FlashWritePageAddr(uint32 Addr)
{
 uint i;
 
 //计算页地址
 Addr/=FLASH_PAGE_SIZE;

 FlashSetAle();  //ALE置高，选择为地址
 FlashSetPortAsOut(); //设置端口为输出状态
 for(i=0;i<2;i++) //写入四字节的地址
 {
  FlashWriteByte(Addr); //写一字节数据
  Addr>>=8;
 }
 FlashSetPortAsIn(); //设置端口为输入
 FlashClrAle(); //ALE置低
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：Flash复位。
入口参数：无。
返    回：无。
备    注：没有片选，必须在其他更高层的函数负责片选。
********************************************************************/
void FlashReset(void)
{
 FlashWriteCommand(0xFF); //写复位命令
 FlashWait();  //等待命令完成
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：Flash初始化。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void FlashInit(void)
{
 *AT91C_PIOA_PER=(1<<2)|(1<<15)|(1<<26)|(1<<27)|(1<<28)|(0xFF<<17);  //Enable Flash IO
 *AT91C_PIOA_OER=(1<<2)|(1<<15)|(1<<26)|(1<<27)|(1<<28);  //Output Enable Flash IO
 *AT91C_PIOA_ODR=(1<<29);  //Output Disable PA29, R/B
 *AT91C_PIOA_SODR=(1<<26)|(1<<27)|(1<<28); //Set IO
 *AT91C_PIOA_OWER=(0xFF<<17);  //使能直接操作端口的模式
 FlashClrCle();
 FlashClrAle();
 FlashClrCe(); //选中芯片 
 FlashReset(); //FLASH复位
 FlashSetCe(); //释放芯片 
 FlashCurrentWriteSectorAddr=-1; //初始化当前读、写地址为无效
 FlashCurrentReadSectorAddr=-1;
 FlashNeedWriteBack=0;  //初始化不用回写
 FlashSwapBlockInit();  //初始化交换块
 FlashClrCe(); //选中芯片 
 FlashLoadBadBlockTable(); //从FLASH中加载坏块表
 FlashSetCe(); //释放芯片 
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：读取状态寄存器。
入口参数：无。
返    回：无。
备    注：FLASH操作状态。
********************************************************************/
uint8 FlashReadStatus(void)
{
 uint8 Status;
 FlashWriteCommand(0x70);  //写查询命令
 FlashReadByte(Status);    //读回状态
 return Status;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：擦除一个块。
入口参数：Addr：字节地址。
返    回：无。
备    注：无。
********************************************************************/
uint8 FlashEraseBlock(uint32 Addr)
{
 //擦除命令第一字节命令
 FlashWriteCommand(0x60);
 //写入两字节的页地址
 FlashWritePageAddr(Addr);
 //写入擦除命令第二字节
 FlashWriteCommand(0xD0);
 //等待擦除完成
 FlashWait();
 //返回操作状态
 return FlashReadStatus();
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：页写命令的后半部分。
入口参数：无。
返    回：无。
备    注：操作的状态。
********************************************************************/
uint8 FlashWritePage(void)
{
 FlashWriteCommand(0x10);  //页写命令
 FlashWait();  //等待写完成
 return FlashReadStatus();
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：复制一页数据。
入口参数：sAddr：源地址；dAddr：目标地址。
返    回：无。
备    注：操作的状态。
********************************************************************/
uint8 FlashCopyPage(uint32 sAddr, uint32 dAddr)
{
 //读源数据到内部缓存
 FlashWriteCommand(0x00);
 FlashWriteAddr4Byte(sAddr);
 FlashWriteCommand(0x35);
 FlashWait();
 
 //将数据写到目标页
 FlashWriteCommand(0x85);
 FlashWriteAddr4Byte(dAddr);
 FlashWriteCommand(0x10);
 FlashWait();
 return FlashReadStatus();
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：获取下一个可用的备用块。
入口参数：无。
返    回：找到的块地址。
备    注：在该函数中会先擦除，只有成功擦除的才被返回。
********************************************************************/
uint32 FlashGetNewRemapBlock(void)
{
 uint32 i,Addr;
 for(i=0;i<FLASH_BAD_BLOCKS_REMAP;i++)
 {
  if(FLASH_BLOCK_OK==FlashRemapBlockStatus[i]) //如果该块还未用
  {
   Addr=FLASH_BAD_BLOCK_REMAP_ADDR+i*FLASH_BLOCK_SIZE; //计算地址
   if(0x01==(FlashEraseBlock(Addr)&0x01))  //如果擦除失败
   {
    FlashRemapBlockStatus[i]=FLASH_BLOCK_BAD;  //标志该块为已经损坏
   }
   else //否则，擦除成功
   {
    FlashRemapBlockStatus[i]=FLASH_BLOCK_USED; //标志为该块已被使用    
    return Addr; //返回地址
   }
  }
 }
 return -1; //如果找不到，则返回-1。
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：标志当前重影射的块为坏块。
入口参数：Addr：要标志的块的地址。
返    回：无。
备    注：无。
********************************************************************/
void FlashMarkRemapBlockBad(uint32 Addr)
{
 uint32 i;
 i=(Addr-FLASH_BAD_BLOCK_REMAP_ADDR)/FLASH_BLOCK_SIZE;  //计算偏移量
 if(i>=FLASH_BAD_BLOCKS_REMAP)return; //出错
 FlashRemapBlockStatus[i]=FLASH_BLOCK_BAD;  //标志为已经损坏
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：地址重新影射，坏块管理用。
入口参数：Addr：需要影射的字节地址。
返    回：影射后的字节地址。
备    注：无。
********************************************************************/
uint32 FlashAddrRemap(uint32 Addr)
{
 static uint32 CurrentRemapBlockAddr;
 uint32 i,j;
 
 if(0==FlashBadBlocksCount)  //如果坏块数量为0，则不需要处理，直接返回地址
 {
  return Addr;
 }
 
 //如果最后一次访问的地址和本次访问的地址属于同一个块地址，那么不需要重新影射
 if(0==((Addr-FlashLastAccessAddr)&(FLASH_BLOCK_SIZE-1)))
 {
  return CurrentRemapBlockAddr+(Addr&(FLASH_BLOCK_SIZE-1)); //由当前块地址加上块内偏移得到完整地址
 }
 
 FlashLastAccessAddr=Addr; //保存最后一次访问过的地址
 
 if(1==FlashBadBlocksCount) //如果坏块数量为1，则直接影射
 {
  if((Addr&(~(FLASH_BLOCK_SIZE-1)))==FlashBadBlockTable[0][0]) //如果块地址相等，则直接影射
  {
   CurrentRemapBlockAddr=FlashBadBlockTable[1][0];
   return CurrentRemapBlockAddr+(Addr&(FLASH_BLOCK_SIZE-1)); //由当前块地址加上块内偏移得到完整地址
  }
  else //不用影射
  {
   CurrentRemapBlockAddr=Addr&(~(FLASH_BLOCK_SIZE-1));  //获取当前块地址
   return Addr;  //直接返回原来的地址
  }
 }
 else //坏块数量大于1
 {
  //如果地址比第一个坏块的地址还小或者比最后一个坏块的地址还大，
  //那么肯定不会是坏快，不需要重新影射
  if((Addr<FlashBadBlockTable[0][0])
   ||((Addr&(FLASH_BLOCK_SIZE-1))>FlashBadBlockTable[0][FlashBadBlocksCount-1]))
  {
   CurrentRemapBlockAddr=Addr&(~(FLASH_BLOCK_SIZE-1));  //获取当前块地址
   return Addr;  //直接返回原来的地址
  }
  else //属于坏块区间，使用二分查表法决定是否需要影射
  {
   i=0;
   j=FlashBadBlocksCount-1;
   while(1)
   {
    if((Addr&(~(FLASH_BLOCK_SIZE-1)))==FlashBadBlockTable[0][(i+j)/2]) //如果相等，则影射
    {
     CurrentRemapBlockAddr=FlashBadBlockTable[1][(i+j)/2];
     return CurrentRemapBlockAddr+(Addr&(FLASH_BLOCK_SIZE-1)); //由当前块地址加上块内偏移得到完整地址
    }
    if(i==j)break; //如果i和j相等，则退出查找
    if((Addr&(~(FLASH_BLOCK_SIZE-1)))<FlashBadBlockTable[0][(i+j)/2])  //如果小于
    {
     j=(i+j)/2-1; //搜索前半段
    }
    else //如果大于
    {
     i=(i+j)/2+1; //搜索后半段
    }
   }
  }
 }
 //没有在坏块表中找到，则说明不是坏块
 CurrentRemapBlockAddr=Addr&(~(FLASH_BLOCK_SIZE-1));  //获取当前块地址
 return Addr;  //直接返回原来的地址
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：从FLASH的特定位置加载坏块表。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void FlashLoadBadBlockTable(void)
{
 uint32 i,j,k,Sum,Ok;
 uint8 Data;

 Ok=0; //设置为不成功
 for(i=0;i<FLASH_BLOCKS_TABLE;i++) //查找没有准备擦除的块
 {
  //从该块中最后一页读回第一字节，看是否为0xFF，如果为0xFF，表示该块没有准备擦除
  FlashWriteCommand(0x00);
  FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*(i+1) - FLASH_PAGE_SIZE);
  FlashWriteCommand(0x30);
  FlashWait(); //等待数据读回
  FlashReadByte(Data);
  if(Data==0xFF)  //表示该块数据还未准备擦除
  {
   //从该块中倒数第二页读回第一字节，看是否为0，如果为0，表示该块已经写入了数据
   FlashWriteCommand(0x00);
   FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*(i+1) - 2*FLASH_PAGE_SIZE);
   FlashWriteCommand(0x30);
   FlashWait(); //等待数据读回
   FlashReadByte(Data);
   if(Data==0) //表示数据有效
   {
    FlashReadByte(Data); //读出校验和
    Sum=Data;
    FlashReadByte(Data); //读出校验和
    Sum=(Sum<<8)+Data;
    FlashReadByte(Data); //读出校验和
    Sum=(Sum<<8)+Data;
    FlashReadByte(Data); //读出校验和
    Sum=(Sum<<8)+Data;
    //从该块开始位置读
    FlashWriteCommand(0x00);
    FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i);
    FlashWriteCommand(0x30);
    FlashWait(); //等待数据读回
    //检查第1字节是否为0
    FlashReadByte(Data);
    if(Data!=0)continue;
    //检查第2字节是否为0x55
    FlashReadByte(Data);
    if(Data!=0x55)continue;
    //检查第3字节是否为0xAA
    FlashReadByte(Data);
    if(Data!=0xAA)continue;
    //检查第4字节是否为0xFF
    FlashReadByte(Data);
    if(Data!=0xFF)continue;
    Sum+=0x1FE;
    
    //读坏块数量
    FlashReadByte(Data);
    FlashBadBlocksCount=Data;
    Sum+=Data;
    FlashReadByte(Data);
    FlashBadBlocksCount=(FlashBadBlocksCount<<8)+Data;
    Sum+=Data;
    FlashReadByte(Data);
    FlashBadBlocksCount=(FlashBadBlocksCount<<8)+Data;
    Sum+=Data;
    FlashReadByte(Data);
    FlashBadBlocksCount=(FlashBadBlocksCount<<8)+Data;
    Sum+=Data;
    j=8;
    //读回坏块表
    for(k=0;k<sizeof(FlashBadBlockTable[0][0])*FLASH_BAD_BLOCKS_REMAP*2;k++)
    {
     if(0==(j&(FLASH_PAGE_SIZE-1))) //如果超过了页，则需要重新读新页
     {
      FlashWriteCommand(0x00);
      FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i+j);
      FlashWriteCommand(0x30);
      FlashWait(); //等待数据读回
     }
     FlashReadByte(Data);
     Sum+=Data; //求校验和
     ((uint8 *)FlashBadBlockTable)[k]=Data;  //读回一字节到坏块表中
     j++;
    }
    //读回重影射区的状态表
    for(k=0;k<sizeof(FlashRemapBlockStatus[0])*FLASH_BAD_BLOCKS_REMAP;k++)
    {
     if(0==(j&(FLASH_PAGE_SIZE-1))) //如果超过了页，则需要重新读新页
     {
      FlashWriteCommand(0x00);
      FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i+j);
      FlashWriteCommand(0x30);
      FlashWait(); //等待数据读回
     }
     FlashReadByte(Data);
     Sum+=Data; //求校验和
     ((uint8 *)FlashRemapBlockStatus)[k]=Data;   //读回一字节到重影射区状态表中
     j++;
    }
    if(Sum==0) //如果校验成功，则说明数据正确
    {
     Ok=0xFF; //设置为成功
     break;   //并退出循环
    }
   }
  }
 }
 
 if(Ok==0) //如果在已写入的表中找不到好的坏块表，再去准备擦除的中去找
 {
  for(i=0;i<FLASH_BLOCKS_TABLE;i++) //查找准备擦除的块
  {
   //从该块中最后一页读回第一字节，看是否为0，如果为0，表示该块已经准备擦除了
   FlashWriteCommand(0x00);
   FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*(i+1) - FLASH_PAGE_SIZE);
   FlashWriteCommand(0x30);
   FlashWait(); //等待数据读回
   FlashReadByte(Data);
   if(Data==0x00)  //表示该块数据准备擦除
   {
    //从该块中倒数第二页读回第一字节，看是否为0，如果为0，表示该块已经写入了数据
    FlashWriteCommand(0x00);
    FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*(i+1) - 2*FLASH_PAGE_SIZE);
    FlashWriteCommand(0x30);
    FlashWait(); //等待数据读回
    FlashReadByte(Data);
    if(Data==0) //表示数据有效
    {
     FlashReadByte(Data); //读出校验和
     Sum=Data;
     FlashReadByte(Data); //读出校验和
     Sum=(Sum<<8)+Data;
     FlashReadByte(Data); //读出校验和
     Sum=(Sum<<8)+Data;
     FlashReadByte(Data); //读出校验和
     Sum=(Sum<<8)+Data;
     //从该块开始位置读
     FlashWriteCommand(0x00);
     FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i);
     FlashWriteCommand(0x30);
     FlashWait(); //等待数据读回
     //检查第1字节是否为0
     FlashReadByte(Data);
     if(Data!=0)continue;
     //检查第2字节是否为0x55
     FlashReadByte(Data);
     if(Data!=0x55)continue;
     //检查第3字节是否为0xAA
     FlashReadByte(Data);
     if(Data!=0xAA)continue;
     //检查第4字节是否为0xFF
     FlashReadByte(Data);
     if(Data!=0xFF)continue;
     Sum+=0x1FE;
     
     //读坏块数量
     FlashReadByte(Data);
     FlashBadBlocksCount=Data;
     Sum+=Data;
     FlashReadByte(Data);
     FlashBadBlocksCount=(FlashBadBlocksCount<<8)+Data;
     Sum+=Data;
     FlashReadByte(Data);
     FlashBadBlocksCount=(FlashBadBlocksCount<<8)+Data;
     Sum+=Data;
     FlashReadByte(Data);
     FlashBadBlocksCount=(FlashBadBlocksCount<<8)+Data;
     Sum+=Data;
     j=8;
     //读回坏块表
     for(k=0;k<sizeof(FlashBadBlockTable[0][0])*FLASH_BAD_BLOCKS_REMAP*2;k++)
     {
      if(0==(j&(FLASH_PAGE_SIZE-1))) //如果超过了页，则需要重新读新页
      {
       FlashWriteCommand(0x00);
       FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i+j);
       FlashWriteCommand(0x30);
       FlashWait(); //等待数据读回
      }
      FlashReadByte(Data);
      Sum+=Data; //求校验和
      ((uint8 *)FlashBadBlockTable)[k]=Data;  //读回一字节到坏块表中
      j++;
     }
     //读回重影射区的状态表
     for(k=0;k<sizeof(FlashRemapBlockStatus[0])*FLASH_BAD_BLOCKS_REMAP;k++)
     {
      if(0==(j&(FLASH_PAGE_SIZE-1))) //如果超过了页，则需要重新读新页
      {
       FlashWriteCommand(0x00);
       FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i+j);
       FlashWriteCommand(0x30);
       FlashWait(); //等待数据读回
      }
      FlashReadByte(Data);
      Sum+=Data; //求校验和
      ((uint8 *)FlashRemapBlockStatus)[k]=Data;   //读回一字节到重影射区状态表中
      j++;
     }
     if(Sum==0) //如果校验成功，则说明数据正确
     {
      FlashSaveBadBlockTable(); //将其保存到FLASH中      
      Ok=0xFF; //设置为成功
      break;   //并退出循环
     }
    }
   }
  }
 }
 
 if(Ok==0) //如果还是没找到，那么只好重新初始化了
 {
  FlashBadBlocksCount=0; //坏块数设置为0
  for(i=0;i<FLASH_BAD_BLOCKS_REMAP;i++)
  {
   //所有影射块都设置为好块
   FlashRemapBlockStatus[i]=FLASH_BLOCK_OK;
   //所有影射关系设置为-1
   FlashBadBlockTable[0][i]=-1;
   FlashBadBlockTable[1][i]=-1;
  }
  //设置好之后保存起来
  FlashSaveBadBlockTable();
 }
 //设置当前访问过的地址为无效值
 FlashLastAccessAddr=-1;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：保存坏块表到FLASH的特定位置。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void FlashSaveBadBlockTable(void)
{
 uint32 i,j,k,Sum;
 
 for(i=0;i<FLASH_BLOCKS_TABLE;i++) //标志为准备擦除
 {
  FlashWriteCommand(0x80);
  FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*(i+1) - FLASH_PAGE_SIZE);
  FlashSetPortAsOut();  //总线设置为输出口
  FlashWriteByte(0x00);  //将第一字节设置为0，表示准备擦除
  //剩余字节写0xFF
  for(j=1;j<FLASH_PAGE_SIZE;j++)
  {
   FlashWriteByte(0xFF);
  }
  FlashWritePage(); //写页
 }
 
 for(i=0;i<FLASH_BLOCKS_TABLE;i++) //将坏块表写入这三块
 {
  FlashEraseBlock(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i); //擦除一块
  FlashWriteCommand(0x80);
  FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i); //写入第一块的开始位置
  FlashSetPortAsOut();  //总线设置为输出口
  FlashWriteByte(0x00);  //将第1字节设置为0
  FlashWriteByte(0x55);  //将第2字节设置为0x55
  FlashWriteByte(0xAA);  //将第3字节设置为0xAA
  FlashWriteByte(0xFF);  //将第4字节设置为0xFF
  Sum=0x1FE;
  //接着写坏块数量，并统计校验和
  FlashWriteByte((FlashBadBlocksCount>>24)&0xFF);
  Sum+=(FlashBadBlocksCount>>24)&0xFF;
  FlashWriteByte((FlashBadBlocksCount>>16)&0xFF);
  Sum+=(FlashBadBlocksCount>>16)&0xFF;
  FlashWriteByte((FlashBadBlocksCount>>8)&0xFF);
  Sum+=(FlashBadBlocksCount>>8)&0xFF;
  FlashWriteByte((FlashBadBlocksCount)&0xFF);
  Sum+=(FlashBadBlocksCount)&0xFF;
  j=8; //写了8字节
  //保存坏块表
  for(k=0;k<sizeof(FlashBadBlockTable[0][0])*FLASH_BAD_BLOCKS_REMAP*2;k++)
  {
   if(0==(j&(FLASH_PAGE_SIZE-1))) //如果超过了页，则需要重新写新页
   {
    FlashWritePage(); //写页
    FlashWriteCommand(0x80);
    FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i + j);
    FlashSetPortAsOut();  //总线设置为输出口
   }
   Sum+=((uint8 *)FlashBadBlockTable)[k]; //求校验和
   FlashWriteByte(((uint8 *)FlashBadBlockTable)[k]);  //写一字节
   j++;
  }
  //保存重影射区的状态表
  for(k=0;k<sizeof(FlashRemapBlockStatus[0])*FLASH_BAD_BLOCKS_REMAP;k++)
  {
   if(0==(j&(FLASH_PAGE_SIZE-1))) //如果超过了页，则需要重新写新页
   {
    FlashWritePage(); //写页
    FlashWriteCommand(0x80);
    FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*i + j);
    FlashSetPortAsOut();  //总线设置为输出口
   }
   Sum+=((uint8 *)FlashRemapBlockStatus)[k]; //求校验和
   FlashWriteByte(((uint8 *)FlashRemapBlockStatus)[k]);  //写一字节
   j++;
  }
  for(;0!=(j&(FLASH_PAGE_SIZE-1));j++) //将剩余部分写入0xFF
  {
   FlashWriteByte(0xFF);
  }
  FlashWritePage();   //写页
  
  //已完成写状态及校验和写入到该块的倒数第二页
  FlashWriteCommand(0x80);
  FlashWriteAddr4Byte(FLASH_BLOCK_TABLE_ADDR + FLASH_BLOCK_SIZE*(i+1) - 2*FLASH_PAGE_SIZE);
  FlashSetPortAsOut();  //总线设置为输出口
  FlashWriteByte(0x00);  //将第一字节设置为0，表示已经写入
  //将校验和取反加1，这样累加结果就为0
  Sum=(~Sum)+1;
  //写校验和
  FlashWriteByte((Sum>>24)&0xFF);
  FlashWriteByte((Sum>>16)&0xFF);
  FlashWriteByte((Sum>>8)&0xFF);
  FlashWriteByte((Sum)&0xFF);
  //剩余字节写0xFF
  for(j=5;j<FLASH_PAGE_SIZE;j++)
  {
   FlashWriteByte(0xFF);
  }
  FlashWritePage(); //写页
 }
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：更新坏块表。
入口参数：OldAddr：旧地址；NewAddr：影射之后的地址。
返    回：无。
备    注：无。
********************************************************************/
void FlashUpdateBadBlockTable(uint32 OldAddr,uint32 NewAddr)
{
 uint32 i,j;
 OldAddr&=~(FLASH_BLOCK_SIZE-1); //求得一块内起始地址
 NewAddr&=~(FLASH_BLOCK_SIZE-1);
 if(OldAddr>FLASH_MAX_SECTOR_ADDR) //如果比能够达到的最大地址还大，说明坏块地址本来就是被重新影射过的
 {
  //先要找到它原来影射的位置
  for(i=0;i<FlashBadBlocksCount;i++)
  {
   if(OldAddr==FlashBadBlockTable[1][i]) //如果与某个地址吻合，则说明就是该地址了
   {
    FlashBadBlockTable[1][i]=NewAddr; //重新影射到新的地址
    //并将原来的交换块设置为已损坏
    FlashMarkRemapBlockBad(OldAddr);
    break;
   }
  }
 }
 else //说明坏块地址是没有被影射过的
 {
  //查找比它大的块地址，将它插入到前面，排好序，方便二分查表
  for(i=0;i<FlashBadBlocksCount;i++) 
  {
   if(OldAddr<FlashBadBlockTable[0][i]) //找到比它大的地址
   {
    break;
   }
  }
  for(j=FlashBadBlocksCount;j>i;j--) //将上面的部分往上移动，腾出一个空位
  {
   FlashBadBlockTable[0][j]=FlashBadBlockTable[0][j-1];
   FlashBadBlockTable[1][j]=FlashBadBlockTable[1][j-1];
  }
  //将当前块的影射写入
  FlashBadBlockTable[0][j]=OldAddr;
  FlashBadBlockTable[1][j]=NewAddr;
  FlashBadBlocksCount++; //增加一个坏块计数
 }
 FlashSaveBadBlockTable(); //存储坏块表
 //修改当前访问过的地址为无效地址，这样下次操作时就会重新影射
 FlashLastAccessAddr=-1;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：FLASH坏块处理。
入口参数：Addr: 字节地址。
返    回：无。
备    注：处理后的地址。
********************************************************************/
uint32 FlashDealBadBlock(uint32 Addr, uint32 Type)
{
 uint32 i;
 uint32 RemapBlockAddr;
 uint32 SwapBlockAddr;
 while(1)
 {
  RemapBlockAddr=FlashGetNewRemapBlock();
  if(RemapBlockAddr==-1)  //如果已经找不到新的可用的替换品，那只好直接返回了。
  {
   return Addr;
  }
  switch(Type)
  {
   //擦除时遇到的坏块，不需要将当前页缓冲区数据写回
   //只需要返回新的地址即可。地址统一在最后返回，这里不用处理
   case 1:
    goto Exit;
   break;
   
   //复制页时遇到的坏块，需要将该块中前面的页及当前页从交换区中重新复制
   case 2:
   //从交换区去复制前面页以及当前页的数据
   SwapBlockAddr=FlashGetCurrentSwapBlock();  //获取当前所使用的交换块
   //复制前面以及当前页
   for(i=0;i<(Addr&(FLASH_BLOCK_SIZE-1))/FLASH_PAGE_SIZE+1;i++)
   {
    if(0x01==(FlashCopyPage(SwapBlockAddr+i*FLASH_PAGE_SIZE,RemapBlockAddr+i*FLASH_PAGE_SIZE)&0x01))
    {
     //如果复制失败，则说明该块有问题，需要找新的块
     goto BadRemapBlock;
    }
   }
   //复制完毕，则退出循环
   goto Exit;
   break;
   
   //写数据时遇到的坏块，需要将该块中前面的页从交换区中重新复制，
   //还需要将当前页从交换区中复制并将缓冲区的输入写入到当前页中
   //这里无法再获取到缓冲区的数据了，只好直接从原来的页复制数据
   case 3:  
   //从交换区去复制前面页数据
   SwapBlockAddr=FlashGetCurrentSwapBlock();  //获取当前所使用的交换块
   //复制前面的页
   for(i=0;i<(Addr&(FLASH_BLOCK_SIZE-1))/FLASH_PAGE_SIZE;i++)
   {
    if(0x01==(FlashCopyPage(SwapBlockAddr+i*FLASH_PAGE_SIZE,RemapBlockAddr+i*FLASH_PAGE_SIZE)&0x01))
    {
     //如果复制失败，则说明该块有问题，需要找新的块
     goto BadRemapBlock;
    }
   }
   //对于当前页，只好从刚刚写入的错误地址去复制
   if(0x01==(FlashCopyPage(Addr,RemapBlockAddr+i*FLASH_PAGE_SIZE)&0x01))
   {
    //如果复制失败，则说明该块有问题，需要找新的块
    goto BadRemapBlock;
   }   
   //复制完毕，则退出循环
   goto Exit;
   break;
   
   default:
   break;
  }
  BadRemapBlock:
  //如果操作过程中失败，则要标志该块已经损坏
  FlashMarkRemapBlockBad(RemapBlockAddr);
 }
 Exit:
 //更新坏块表
 FlashUpdateBadBlockTable(Addr,RemapBlockAddr);
 return RemapBlockAddr+(Addr&(FLASH_BLOCK_SIZE-1));
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：管理可用的交换块地址。
入口参数：Op：对应的操作。
返    回：下一个可用的交换块的地址。
备    注：无。
********************************************************************/
uint32 FlashManageSwapBlock(uint32 Op)
{
 static uint32 Current;
 static uint8 FlashSwapBlockStatus[FLASH_SWAP_BLOCKS];
 uint32 i;
 
 switch(Op)
 {
  case 0:  //如果操作为1，表示初始化
   Current=0;
   for(i=0;i<FLASH_SWAP_BLOCKS;i++)
   {
    FlashSwapBlockStatus[i]=0; //初始化所有交换块为好的
   }
  break;
  
  case 1: //如果操作为1，表示获取下一个可用的交换区
   while(1)//一直尝试，如果交换区都用完（坏）了，那么就死循环了，
   {
    Current++;
    if(Current>=FLASH_SWAP_BLOCKS)
    {
     Current=0;
    }
    if(FlashSwapBlockStatus[Current]==0)break; //如果该块标志为0，则说明未损坏
   }
  break;
  
  case 2: //如果操作为2，说明获取当前交换区地址
  break;
  
  case 3: //如果操作为3，设置当前交换块为坏块
   FlashSwapBlockStatus[Current]=FLASH_BLOCK_BAD;
  break;
  
  default:
  break;
 }
 return FLASH_SWAP_BLOCK_ADDR+Current*FLASH_BLOCK_SIZE; //返回可用的交换块地址
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：将一块数据复制到交换区。同时将原来的块删除，
          并将该块内Addr所在页前面的页面复制回原来的块。
入口参数：Addr：要复制出来的块地址。
返    回：原来块的地址。
备    注：如果在复制回去的过程中，出现错误，
          那么说明原来的块已经损坏，需要重新影射到一个好的块。
          这时返回的地址就是重新影射过后的地址。
********************************************************************/
uint32 FlashCopyBlockToSwap(uint32 Addr)
{
 uint32 SwapAddr;
 uint32 i;
 uint32 BlockStartAddr;
 
 BlockStartAddr=(Addr)&(~(FLASH_BLOCK_SIZE-1));  //计算块起始地址
 
 while(1)
 {
  SwapAddr=FlashGetNextSwapBlock(); //获取下一个交换区
  if(0x00==(FlashEraseBlock(SwapAddr)&0x01)) //如果擦除成功
  {
   for(i=0;i<FLASH_BLOCK_SIZE/FLASH_PAGE_SIZE;i++)  //将对应块中所有页复制到交换区中
   {
    //复制一页
    if(0x01&FlashCopyPage(BlockStartAddr+i*FLASH_PAGE_SIZE,SwapAddr+i*FLASH_PAGE_SIZE))
    {
     //如果复制失败，则说明该交换块已经损坏，查找下一个可用的交换块
     goto BadSwapBlock;
    }
   }
   //全部复制完毕，则擦除掉原来的块
   if(0x01==(FlashEraseBlock(BlockStartAddr)&0x01)) //如果擦除失败
   {
    Addr=FlashDealBadBlock(Addr,1); //处理擦除时遇到的坏块
    BlockStartAddr=(Addr)&(~(FLASH_BLOCK_SIZE-1));  //计算块起始地址
   }
   //将前面部分不会写到的页复制回去
   for(i=0;i<(Addr-BlockStartAddr)/FLASH_PAGE_SIZE;i++)
   {
    //复制一页
    if(0x01&FlashCopyPage(SwapAddr+i*FLASH_PAGE_SIZE,BlockStartAddr+i*FLASH_PAGE_SIZE))
    {
     //如果复制失败，则处理该坏块
     //注意FlashDealBadBlock返回的是当前正在操作的扇区地址，
     //需要取出其块地址加上Addr原来的扇区地址合成新的扇区地址
     Addr=(FlashDealBadBlock(BlockStartAddr+i*FLASH_PAGE_SIZE,2)&(~(FLASH_BLOCK_SIZE-1)))
         +(Addr&(FLASH_BLOCK_SIZE-1));
     BlockStartAddr=(Addr)&(~(FLASH_BLOCK_SIZE-1));  //计算块起始地址
    }
   }
   return Addr; //复制完毕，返回
  }
  else //否则，擦除失败
  {
   BadSwapBlock:
   //标志该块擦除时被损坏
   FlashMarkBadCurrentSwapBlock();
  }
 }
 return Addr;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：往FLASH中写一个扇区（FLASH_SECTOR_SIZE字节）。
入口参数：Addr: 字节地址；pBuf：保存数据的缓冲区；Remain：预计接下来还需要写多少扇区
返    回：写入的状态。0：成功。非0：失败。
备    注：当Remain不为0时，当前页以及该块内剩余部分将不会回写！
          如果数据传输结束，应该将Remain置0，将数据写回。
********************************************************************/
uint32 FlashWriteOneSector(uint32 Addr, uint8 * pBuf, uint32 Remain)
{
 uint32 i;
 uint32 SwapPageAddr;
 
 FlashClrCe(); //选中芯片
 if(Addr>FLASH_MAX_SECTOR_ADDR)return 1; //如果地址超出范围，则返回失败代码1，越界
 Addr=FlashAddrRemap(Addr); //重新影射地址
 if((Addr&(~(FLASH_PAGE_SIZE-1)))!=(FlashCurrentWriteSectorAddr&(~(FLASH_PAGE_SIZE-1)))) //如果跨page
 {
  if(FlashNeedWriteBack) //如果前面写了数据，则需要将当前读出的page写回
  {
   if(FlashWritePage()&0x01) //写入失败
   {
    Addr=FlashDealBadBlock(Addr-FLASH_PAGE_SIZE,3)+FLASH_PAGE_SIZE;  //坏块处理
   }
  }
  if((Addr&(~(FLASH_BLOCK_SIZE-1)))!=(FlashCurrentWriteSectorAddr&(~(FLASH_BLOCK_SIZE-1))))  //如果跨block，则需要擦除新的块，
  {
   //在擦除之前，要先将原来的块复制到交换区，并且将该块前面部分数据写回
   //该函数除了将整块数据复制到交换区以外，并且还将擦除掉原来的块，然后将前面部分复制回原来的块
   Addr=FlashCopyBlockToSwap(Addr);
  }
  //从交换区中读出对应的一页
  FlashWriteCommand(0x00);
  FlashWriteAddr4Byte(FlashGetCurrentSwapBlock()+(Addr&(FLASH_BLOCK_SIZE-1)));
  FlashWriteCommand(0x35);
  FlashWait();
  //随机写
  FlashWriteCommand(0x85);
  FlashWriteAddr4Byte(Addr); //写4字节地址
  FlashSetPortAsOut();  //总线设置为输出口
  for(i=0;i<FLASH_SECTOR_SIZE;i++)
  {
   FlashWriteByte(pBuf[i]);
  }
  FlashSetPortAsIn(); //总线设置为输入口
  FlashNeedWriteBack=1; //需要写回
 }
 else  //没有超过一页地址，则直接写数据
 {
  //随机写
  FlashWriteCommand(0x85);
  FlashWriteAddr2Byte(Addr);
  FlashSetPortAsOut();  //总线设置为输出口
  for(i=0;i<FLASH_SECTOR_SIZE;i++)
  {
   FlashWriteByte(pBuf[i]);
  }
  FlashSetPortAsIn(); //总线设置为输入口
  FlashNeedWriteBack=1; //需要写回
 }
 FlashCurrentWriteSectorAddr=Addr; //保存本次地址 
 if(Remain==0) //剩余扇区数为0，不会再写了，需要写回
 {
  if(FlashNeedWriteBack) //如果前面写了数据，则需要将当前读出的page写回
  {
   if(FlashWritePage()&0x01) //写入失败
   {
    Addr=FlashDealBadBlock(Addr,3);  //坏块处理
   }
  }
  //计算剩余页数
  Remain=(((Addr+FLASH_BLOCK_SIZE)&(~(FLASH_BLOCK_SIZE-1)))-(Addr&(~(FLASH_PAGE_SIZE-1))))/FLASH_PAGE_SIZE-1;
  //计算在交换块中的起始页地址
  SwapPageAddr=FlashGetCurrentSwapBlock()+(Addr&(FLASH_BLOCK_SIZE-1));
  
  for(i=0;i<Remain;i++)  //将该块内保存在交换块中剩余部分页的数据复制回该块
  {
   Addr+=FLASH_PAGE_SIZE;   //从下一页开始写
   SwapPageAddr+=FLASH_PAGE_SIZE;   
   if(0x01==(FlashCopyPage(SwapPageAddr,Addr)&0x01)) //如果复制失败
   {
    Addr=FlashDealBadBlock(Addr,2);  //处理坏块
   }
  }
  FlashNeedWriteBack=0; //清除需要写回标志
  FlashCurrentWriteSectorAddr=-1;
 }
 FlashSetCe(); //释放FLASH芯片
 return 0;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：从FLASH中读出一扇区数据保存到缓冲区中。
入口参数：Addr: 字节地址；pBuf：保存数据的缓冲区；Remain：预计接下来还需要读多少扇区
返    回：读取的状态。0：成功。非0：失败。
备    注：当Remain不为0时，将保存当前地址以备后面的继续读当前页，当不为0时，
          设置当前读地址为无效，从而下次读时必须重新使用读命令将数据从flash中读入到页缓存。
********************************************************************/
uint32 FlashReadOneSector(uint32 Addr, uint8 * pBuf, uint32 Remain)
{
 uint32 i;
 FlashClrCe(); //选中芯片
 if(Addr>FLASH_MAX_SECTOR_ADDR)return 1; //如果地址超出范围，则返回失败代码1，越界
 Addr=FlashAddrRemap(Addr); //重新影射地址
 if((Addr&(~(FLASH_PAGE_SIZE-1)))
    !=(FlashCurrentReadSectorAddr&(~(FLASH_PAGE_SIZE-1)))) //如果跨page
 {
  //如果跨页的，则写读数据命令
  FlashWriteCommand(0x00);
  FlashWriteAddr4Byte(Addr);
  FlashWriteCommand(0x30);
  FlashWait(); //等待数据读回
 }
 else
 {
  //如果没有跨页，则可以直接读
  FlashWriteCommand(0x05);
  FlashWriteAddr2Byte(Addr);
  FlashWriteCommand(0xE0);
  FlashWait(); //等待数据读回
 }
 for(i=0;i<FLASH_SECTOR_SIZE;i++)
 {
  FlashReadByte(pBuf[i]);  //读一字节数据
 }
 FlashCurrentReadSectorAddr=Addr; //保存当前操作的地址
 if(Remain==0) //如果不会接着读，那么就设置当前读过的地址为无效值
 {
  FlashCurrentReadSectorAddr=-1;
 }
 FlashSetCe(); //释放总线
 return 0;
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：读FLASH的ID号。
入口参数：Buf：保存ID号的缓冲区。
返    回：无。
备    注：无。
********************************************************************/
void FlashReadId(uint8 *Buf)
{
 uint8 i;
 FlashClrCe();
 FlashWriteCommand(0x90);
 FlashSetAle();  //ALE置高，选择为地址
 FlashSetPortAsOut(); //设置端口为输出状态
 FlashWriteByte(0); //写一字节数据
 FlashSetPortAsIn(); //设置端口为输入
 FlashClrAle();      //ALE置低
 for(i=0;i<5;i++)  //读5字节的ID
 {
  FlashReadByte(Buf[i]);
 }
 FlashSetCe();
}
/////////////////////////End of function/////////////////////////////
