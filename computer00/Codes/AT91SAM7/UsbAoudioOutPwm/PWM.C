/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
				欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
				                     http://computer00.21ic.org
									 
PWM.c  file
								 
作者：Computer-lov
建立日期: 2009-05-15
修改日期: 2009-05-15
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include <AT91SAM7S64.H>

/********************************************************************
函数功能：PWM初始化。
入口参数：无，
返    回：无。
备    注：无。
********************************************************************/
void PwmInit(void)
{
 //将PA0连接到外设A，即PWM0
 //PIO模块
 *AT91C_PIOA_PDR=(1<<0);    //PA0
 *AT91C_PIOA_IDR=(1<<0);    //禁止PA0中断
 *AT91C_PIOA_PPUER=(1<<0);  //使能PA0上拉电阻
 *AT91C_PIOA_ASR=(1<<0);    //连接到A外设（即PWM0）
 
 //PWM模块
 *AT91C_PWMC_MR=0x00;  //不使用CLKA和CLKB
 *AT91C_PWMC_IDR=0x0F;  //禁止所有PWM中断
 *AT91C_PWMC_CH0_CMR = (0x00<<0)    //时钟选择为MCK
                      |(0x00<<8)    //CALGCALG: Channel Alignment，左对齐
                      |(0x01<<9)    //CPOL: Channel Polarity,
                                    //开始输出为高电平
                      |(0x00<<10);  //CPD: Channel Update Period,
                                    //PWM_CUPDx寄存器将设置占空比
 *AT91C_PWMC_CH0_CDTYR = 0x00;  //占空比设置为0
 *AT91C_PWMC_CH0_CPRDR = 0xFF;  //频率设置为48MHz/256=187.5kHz
 *AT91C_PWMC_ENA=0x01;  //使能PWM0
}
/////////////////////////End of function/////////////////////////////

