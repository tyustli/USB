/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
			
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

        感谢PCB赞助商——电子园： http://bbs.cepark.com/

main.c file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2012.01.24
版本：V1.1
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/

#include <AT89X52.H>  //头文件
#include "Key.h"
#include "Led.h"
#include "UART.h"
#include "pdiusbd12.h"
#include "UsbCore.h"
#include "config.h"
#include <math.h>

code uint8 HeadTable[][74]={
"********************************************************************\r\n",
"******             《圈圈教你玩USB》之 USB触摸屏              ******\r\n",
"******                      AT89S52 CPU                       ******\r\n",
"******                  建立日期：",__DATE__,"                 ******\r\n",
"******                   建立时间：",__TIME__,"                   ******\r\n",
"******                    作者：电脑圈圈                      ******\r\n",
"******                    欢迎访问作者的                      ******\r\n",
"******           USB专区：http://group.ednchina.com/93/       ******\r\n",
"******      BLOG1：http://www.ednchina.com/blog/computer00    ******\r\n",
"******                BLOG2：http://computer00.21ic.org       ******\r\n",
"******                  请按K1-K8分别进行测试                 ******\r\n",
"******    K1:回到中心  K2:画直线   K3:画三角形  K4:画正方形   ******\r\n",
"******    K5:画圆      K6:正弦曲线 K7:无        K8:无         ******\r\n",
"********************************************************************\r\n",
};

/********************************************************************
函数功能：中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void ProcessInterrupt(void)
{
 int8 InterruptSource;
 if(D12GetIntPin() == 0) //如果有中断发生
 {
  //有中断发生，处理之
  D12WriteCommand(READ_INTERRUPT_REGISTER);  //写读中断寄存器的命令
  InterruptSource=D12ReadByte(); //读回第一字节的中断寄存器
  if(InterruptSource&0x80)UsbBusSuspend(); //总线挂起中断处理
  if(InterruptSource&0x40)UsbBusReset();   //总线复位中断处理
  if(InterruptSource&0x01)UsbEp0Out();     //端点0输出中断处理
  if(InterruptSource&0x02)UsbEp0In();      //端点0输入中断处理
  if(InterruptSource&0x04)UsbEp1Out();     //端点1输出中断处理
  if(InterruptSource&0x08)UsbEp1In();      //端点1输入中断处理
  if(InterruptSource&0x10)UsbEp2Out();     //端点2输出中断处理
  if(InterruptSource&0x20)UsbEp2In();      //端点2输入中断处理
 }
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：移动光标。
入口参数：x：x轴坐标；y：y轴坐标
返    回：无。
备    注：无。
********************************************************************/
void MoveTo(uint16 x, uint16 y)
{
 //需要返回的5字节报告的缓冲
 //Buf[0]的D0就是左键，D1就是右键，D2就是中键
 //Buf[1]为X轴低字节，Buf[2]为X轴高字节，
 //Buf[3]为Y轴低字节，Buf[4]为Y轴高字节，
 uint8 Buf[5]={0,0,0,0,0};
 Buf[0] = 0x00;
 Buf[1] = x & 0xFF;
 Buf[2] = (x >> 8) & 0xFF;
 Buf[3] = y & 0xFF;
 Buf[4] = (y >> 8) & 0xFF;
 while(Ep1InIsBusy) //等待之前的数据发送完毕
 {
  ProcessInterrupt();  //处理中断
 }
 if(ConfigValue == 0) return;
 //报告准备好了，通过端点1返回，长度为5字节。
 D12WriteEndpointBuffer(3, 5, Buf);
 Ep1InIsBusy=1;  //设置端点忙标志。
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：画线段的函数。
入口参数：x：x轴坐标；y：y轴坐标
返    回：无。
备    注：无。
********************************************************************/
void LineTo(uint16 x, uint16 y)
{
 //需要返回的5字节报告的缓冲
 //Buf[0]的D0就是左键，D1就是右键，D2就是中键
 //Buf[1]为X轴低字节，Buf[2]为X轴高字节，
 //Buf[3]为Y轴低字节，Buf[4]为Y轴高字节，
 uint8 Buf[5]={0,0,0,0,0};
 Buf[0] = 0x01; //左键按下
 Buf[1] = x & 0xFF;
 Buf[2] = (x >> 8) & 0xFF;
 Buf[3] = y & 0xFF;
 Buf[4] = (y >> 8) & 0xFF;
 while(Ep1InIsBusy) //等待之前的数据发送完毕
 {
  ProcessInterrupt(); //处理中断
 }
 if(ConfigValue == 0) return;
 //报告准备好了，通过端点1返回，长度为5字节。
 D12WriteEndpointBuffer(3, 5, Buf);
 Ep1InIsBusy=1;  //设置端点忙标志。
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：画圆的函数。
入口参数：x：x轴坐标；y：y轴坐标；r：半径
返    回：无。
备    注：无。
********************************************************************/
void DrawCircle(uint16 x, uint16 y, uint16 r)
{
 uint16 i, px, py;
 MoveTo(x + r, y);
 LineTo(x + r, y);
 for(i = 0; i < 360; i ++)
 {
  px = x + r * cos((i * 1.0) / 180 * 3.1415926);
  py = y + r * sin((i * 1.0) / 180 * 3.1415926);
  LineTo(px, py);
 }
 MoveTo(x + r, y);
}
////////////////////////End of function//////////////////////////////


/********************************************************************
函数功能：画正弦曲线的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void DrawSinCurve(void)
{
 uint16 px, py;
 MoveTo(4000, 2000);
 LineTo(4000, 2000);
 LineTo(400, 2000);
 for(px = 400; px <= 4000; px += 10)
 {
  py = 2000 - 1000 * sin(((px - 400) / 2.5) / 180 * 3.1415926);
  LineTo(px, py);
 }
 MoveTo(px, py);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：根据按键情况返回报告的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void SendReport(void)
{
 if(KeyDown & (KEY1))
 {
  MoveTo(4096 / 2, 4096 /2); //移动到屏幕中心
 }
 if(KeyDown & (KEY2)) //画直线
 {
  MoveTo(1000, 1000); //移动到（1000，1000）
  LineTo(1000, 1000); //开始画线
  LineTo(3000, 3000); //画线到（3000，3000）
  MoveTo(3000, 3000); //松开鼠标左键
 }
 if(KeyDown & (KEY3)) //画三角形
 {
  MoveTo(2000, 1000); //移动到（2000，1000）
  LineTo(2000, 1000); //开始画线
  LineTo(1000, 3000); //画线到（1000，3000）
  LineTo(3000, 3000); //画线到（3000，3000）
  LineTo(2000, 1000); //画线到（2000，1000）
  MoveTo(2000, 1000); //松开鼠标左键
 }
 if(KeyDown & (KEY4)) //画正方形
 {
  MoveTo(1000, 1000); //移动到（1000，1000）
  LineTo(1000, 1000); //开始画线
  LineTo(1000, 3000); //画线到（1000，3000）
  LineTo(3000, 3000); //画线到（3000，3000）
  LineTo(3000, 1000); //画线到（3000，1000）
  LineTo(1000, 1000); //画线到（1000，1000）
  MoveTo(1000, 1000); //松开鼠标左键
 }
 if(KeyDown & (KEY5))
 {
  DrawCircle(2000, 2000, 1000); //画一个圆心在（2000，2000），半径为1000的圆
 }
 if(KeyDown & (KEY6))
 {
  DrawSinCurve(); //画正弦曲线
 }
 //记得清除KeyUp和KeyDown
 KeyUp=0;
 KeyDown=0;
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：主函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void main(void)  //主函数
{
 uint8 i;
 uint16 id;
 
 EA=1; //打开中断
 InitKeyboard(); //初始化按键
 InitUART();  //初始化串口
 
 for(i=0;i<18;i++)	  //显示信息
 {
  Prints(HeadTable[i]);
 }
 
 id=D12ReadID();

 Prints("Your D12 chip\'s ID is: ");
 PrintShortIntHex(id);

 if(id==0x1012)
 {
  Prints(". ID is correct! Congratulations!\r\n\r\n");
 }
 else
 {
  Prints(". ID is incorrect! What a pity!\r\n\r\n");
 }
 
 UsbDisconnect();  //先断开USB连接
 UsbConnect();  //将USB连接上
 ConfigValue=0; //配置值初始化为0
 
 while(1)  //死循环
 {
  ProcessInterrupt(); //处理中断
  if(ConfigValue!=0) //如果已经设置为非0的配置，则可以返回报告数据
  {
   LEDs=~KeyPress;  //利用板上8个LED显示按键状态，按下时亮
   if(!Ep1InIsBusy)  //如果端点1输入没有处于忙状态，则可以发送数据
   {
    KeyCanChange=0;  //禁止按键扫描
    if(KeyUp||KeyDown) //如果有按键事件发生
    {
     SendReport();  //则返回报告
    }
    KeyCanChange=1;  //允许按键扫描
   }
  }
 }
}
////////////////////////End of function//////////////////////////////
