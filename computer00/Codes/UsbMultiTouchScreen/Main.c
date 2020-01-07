/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
			
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

        感谢PCB赞助商——电子园： http://bbs.cepark.com/

main.c file

作者：电脑圈圈
建立日期: 2008.06.27
修改日期: 2012.01.26
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

#define PI 3.1415926

#undef SHOW_MORE_MSG

#ifdef SHOW_MORE_MSG //节省ROM空间
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
#endif

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
函数功能：产生单点触摸事件。
入口参数：x：x轴坐标；y：y轴坐标；touch：是否触摸。
返    回：无。
备    注：无。
********************************************************************/
void SinglePointReport(uint16 x, uint16 y, uint8 touch)
{
 //需要返回的6字节报告的缓冲
 //Buf[0]为报告ID，值为REPORTID_STOUCH。
 //Buf[1]的D0就是左键，D1就是右键，D2就是中键
 //Buf[2]为X轴低字节，Buf[3]为X轴高字节，
 //Buf[4]为Y轴低字节，Buf[5]为Y轴高字节。
 uint8 Buf[6]={0,0,0,0,0,0};
 Buf[0] = REPORTID_STOUCH;
 Buf[1] = touch;
 Buf[2] = x & 0xFF;
 Buf[3] = (x >> 8) & 0xFF;
 Buf[4] = y & 0xFF;
 Buf[5] = (y >> 8) & 0xFF;
 while(Ep1InIsBusy) //等待之前的数据发送完毕
 {
  ProcessInterrupt();  //处理中断
 }
 if(ConfigValue == 0) return;
 //报告准备好了，通过端点1返回，长度为6字节。
 D12WriteEndpointBuffer(3, 6, Buf);
 Ep1InIsBusy=1;  //设置端点忙标志。
}
////////////////////////End of function//////////////////////////////

#define MoveTo(x,y) SinglePointReport(x, y, 0)
#define LineTo(x,y) SinglePointReport(x, y, 1)

/********************************************************************
函数功能：画圆的函数。
入口参数：x：x轴坐标；y：y轴坐标；r：半径
返    回：无。
备    注：无。
********************************************************************/
void DrawCirle(uint16 x, uint16 y, uint16 r)
{
 uint16 i, px, py;
 MoveTo(x + r, y);
 LineTo(x + r, y);
 for(i = 0; i < 360; i ++)
 {
  px = x + r * cos((i * 1.0) / 180 * PI);
  py = y + r * sin((i * 1.0) / 180 * PI);
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
  py = 2000 - 1000 * sin(((px - 400) / 2.5) / 180 * PI);
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
  DrawCirle(2000, 2000, 1000); //画一个圆心在（2000，2000），半径为1000的圆
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
函数功能：产生多点触摸事件。
入口参数：x：x轴坐标数组；y：y轴坐标数组；
          s：状态数组，例如是否触摸，是否有效；n：触摸的点数。
返    回：无。
备    注：无。
********************************************************************/
void MultiPointReport(uint16 *x, uint16 *y, uint8 *s, uint8 n)
{
 uint8 i;
 uint8 ReportBuf[14]; //总共为14字节，第1字节为报告ID。
 //第2字节为第一点状态，第3字节为第一点的触摸ID号；
 //第4、5字节为第一点x轴，第6、7字节为第一点y轴；
 //第8字节为第二点状态，第9字节为第二点的触摸ID号；
 //第10、11字节为第二点x轴，第12、13字节为第二点y轴；
 //第14字节为当前触摸的点数。我们定义的报告中，每次只能
 //发送2个点，如果超过两个点，则另外再增加额外的报告，
 //这时额外的报告的触摸点数都要设置为0。
 if(n == 0) return;
 if(n > MAX_TOUCH_POINT) //如果超过最大支持的点数，则只发送最多点数
 {
  n = MAX_TOUCH_POINT;
 }
 ReportBuf[0] = REPORTID_MTOUCH; //多点报告的报告ID为REPORTID_MTOUCH
 for(i = 0; i < n;) //分别发送各个点
 {
  ReportBuf[1] = s[i]; //状态
  ReportBuf[2] = i + 1; //ID号
  ReportBuf[3] = x[i] & 0xFF; //X轴低8位
  ReportBuf[4] = (x[i] >> 8) & 0xFF; //X轴高8位
  ReportBuf[5] = y[i] & 0xFF; //Y轴低8位
  ReportBuf[6] = (y[i] >> 8) & 0xFF; //Y轴高8位
  if(i == 0) //第一个包
  {
   ReportBuf[13] = n; //触摸的点数
  }
  else //其它包，设置为0
  {
   ReportBuf[13] = 0;
  }
  i ++;
  if(i < n) //还有数据需要发送
  {
   ReportBuf[7] = s[i]; //状态
   ReportBuf[8] = i + 1; //ID号
   ReportBuf[9] = x[i] & 0xFF; //X轴低8位
   ReportBuf[10] = (x[i] >> 8) & 0xFF; //X轴高8位
   ReportBuf[11] = y[i] & 0xFF; //Y轴低8位
   ReportBuf[12] = (y[i] >> 8) & 0xFF; //Y轴高8位
   i ++;
  }
  else //没有更多的数据需要发送，后面的清0
  {
   uint8 j;
   for(j = 7; j < 13; j++)
   {
    ReportBuf[j] = 0;
   }
  }
  while(Ep1InIsBusy) //等待之前的数据发送完毕
  {
   ProcessInterrupt();  //处理中断
  }
  if(ConfigValue == 0) return;
  //报告准备好了，通过端点1返回，长度为14字节。
  D12WriteEndpointBuffer(3, 14, ReportBuf);
  Ep1InIsBusy=1;  //设置端点忙标志。
 }
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：多点模式点击屏幕中央。
入口参数：touch: 0表示松开，1表示触摸。
返    回：无。
备    注：无。
********************************************************************/
void MultiPointGoToCenter(uint8 touch)
{
 uint16 x, y;
 uint8 s;
 x = 4096 / 2;
 y = 4096 / 2;
 if(touch)
 {
  s = 0x07; //点击
 }
 else
 {
  s = 0x00; //松开
 }
 MultiPointReport(&x, &y, &s, 1);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：多点模式画一条线段。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void MultiPointDrawLine()
{
 uint16 x, y;
 uint8 s;
 x = 1000;
 y = 1000;
 s = 0x06; //数据有效，但是无点击
 MultiPointReport(&x, &y, &s, 1); //移动到(1000, 1000)
 s = 0x07; //数据有效，且有点击
 MultiPointReport(&x, &y, &s, 1); //开始画线
 x = 3000;
 y = 3000;
 MultiPointReport(&x, &y, &s, 1); //画到(3000, 3000)
 s = 0x00; //停止触摸
 MultiPointReport(&x, &y, &s, 1);
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：同时画二个三角形。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void MultiPointDrawTriangle()
{
 uint16 x[2], y[2];
 uint8 s[2];
 x[0] = 2000;
 y[0] = 1000;
 s[0] = 0x06;
 x[1] = x[0];
 y[1] = y[0] + 500;
 s[1] = 0x06;
 MultiPointReport(x, y, s, 2); //移动到起点
 s[0] = 0x07;
 s[1] = 0x07;
 MultiPointReport(x, y, s, 2); //开始画线
 x[0] = 1000;
 y[0] = 3000;
 x[1] = x[0] + 500 * cos(30 * PI / 180);
 y[1] = y[0] - 500 * sin(30 * PI / 180);
 MultiPointReport(x, y, s, 2); //画左边两条线
 x[0] = 3000;
 y[0] = 3000;
 x[1] = x[0] - 500 * cos(30 * PI / 180);
 y[1] = y[0] - 500 * sin(30 * PI / 180);
 MultiPointReport(x, y, s, 2); //画底边两条线
 x[0] = 2000;
 y[0] = 1000;
 x[1] = x[0];
 y[1] = y[0] + 500;
 MultiPointReport(x, y, s, 2); //画右边两条线
 s[0] = 0;
 s[1] = 0;
 MultiPointReport(x, y, s, 2); //停止触摸
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：同时画四个正方形。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void MultiPointDrawSquare()
{
 uint16 x[4], y[4];
 uint8 s[4];
 uint8 i;
 x[0] = 1000;
 y[0] = 1000;
 s[0] = 0x06;
 for(i = 1; i < 4; i ++)
 {
  x[i] = x[i - 1] + 100;
  y[i] = y[i - 1] + 100;
  s[i] = s[0];
 }
 MultiPointReport(x, y, s, 4); //移动到起点
 for(i = 0; i < 4; i ++)
 {
  s[i] = 0x07;
 }
 MultiPointReport(x, y, s, 4); //开始画线
 y[0] = 3000;
 y[1] = 3000 - 100;
 y[2] = 3000 - 200;
 y[3] = 3000 - 300;
 MultiPointReport(x, y, s, 4); //画左边4条线
 x[0] = 3000;
 x[1] = 3000 - 100;
 x[2] = 3000 - 200;
 x[3] = 3000 - 300;
 MultiPointReport(x, y, s, 4); //画左边4条线
 y[0] = 1000;
 y[1] = 1000 + 100;
 y[2] = 1000 + 200;
 y[3] = 1000 + 300;
 MultiPointReport(x, y, s, 4); //画右边4条线
 x[0] = 1000;
 x[1] = 1000 + 100;
 x[2] = 1000 + 200;
 x[3] = 1000 + 300;
 MultiPointReport(x, y, s, 4); //画上边4条线
 for(i = 0; i < 4; i ++)
 {
  s[i] = 0;
 }
 MultiPointReport(x, y, s, 4); //停止触摸
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：同时画三个圆。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void MultiPointDrawCircle(void)
{
 #define CENTER_X 2000
 #define CENTER_Y 2000
 #define R0       1000
 #define R1       800
 #define R2       600
 
 uint16 x[3], y[3];
 uint8 s[3];
 uint16 i;
 float vsin, vcos;
 
 x[0] = CENTER_X + R0;
 x[1] = CENTER_X + R1;
 x[2] = CENTER_X + R2;
 for(i = 0; i < 3; i ++)
 {
  y[i] = CENTER_Y;
  s[i] = 0x06;
 }
 MultiPointReport(x, y, s, 3); //移动到起点
 for(i = 0; i < 3; i ++)
 {
  s[i] = 0x07;
 }
 MultiPointReport(x, y, s, 3); //开始画线
 for(i = 0; i < 360; i ++)
 {
  vsin = sin((i * 1.0) / 180 * PI);
  vcos = cos((i * 1.0) / 180 * PI);
  x[0] = CENTER_X + R0 * vcos;
  y[0] = CENTER_Y + R0 * vsin;
  x[1] = CENTER_X + R1 * vcos;
  y[1] = CENTER_Y + R1 * vsin;
  x[2] = CENTER_X + R2 * vcos;
  y[2] = CENTER_Y + R2 * vsin;
  MultiPointReport(x, y, s, 3); //画线
 }
 for(i = 0; i < 3; i ++)
 {
  s[i] = 0x00;
 }
 MultiPointReport(x, y, s, 3); //停止触摸
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：同时画正弦、余弦曲线。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void MultiPointDrawSinCosCurve()
{
 uint16 x[2], y[2];
 uint8 s[2];
 uint16 i;
 x[0] = 400;
 y[0] = 2000;
 x[1] = 400;
 y[1] = 2000;
 s[0] = 0x06;
 s[1] = 0x06;
 MultiPointReport(x, y, s, 2); //移动到起点
 s[0] = 0x07;
 s[1] = 0x07;
 MultiPointReport(x, y, s, 2); //开始画线
 for(i = 400; i <= 4000; i += 10)
 {
  x[0] = i;
  x[1] = i;
  y[0] = 2000 - 1000 * sin(((i - 400) / 2.5) / 180 * PI);
  y[1] = 2000 - 1000 * cos(((i - 400) / 2.5) / 180 * PI);
  MultiPointReport(x, y, s, 2); //画线
 }
 s[0] = 0x00;
 s[1] = 0x00;
 MultiPointReport(x, y, s, 2); //停止触摸
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：根据按键情况返回多点报告的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void MultiPointSendReport(void)
{
 if(KeyUp & (KEY1))
 {
  MultiPointGoToCenter(0); //松开
 }
 if(KeyPress & (KEY1)) //长按KEY1将弹出右键
 {
  MultiPointGoToCenter(1); //多点模式点击屏幕中央
 }
 if(KeyDown & (KEY2))
 {
  MultiPointDrawLine(); //多点模式画一条直线
 }
 if(KeyDown & (KEY3))
 {
  MultiPointDrawTriangle(); //同时画二个三角形
 }
 if(KeyDown & (KEY4))
 {
  MultiPointDrawSquare(); //同时画四个正方形
 }
 if(KeyDown & (KEY5))
 {
  MultiPointDrawCircle(); //同时画三个圆
 }
 if(KeyDown & (KEY6))
 {
  MultiPointDrawSinCosCurve(); //同时画正弦、余弦曲线
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
#ifdef SHOW_MORE_MSG //节省ROM空间
 uint8 i;
#endif

 uint16 id;
 
 EA=1; //打开中断
 InitKeyboard(); //初始化按键
 InitUART();  //初始化串口
 
#ifdef SHOW_MORE_MSG //节省ROM空间
 for(i=0;i<18;i++)	  //显示信息
 {
  Prints(HeadTable[i]);
 }
#endif
 
 id=D12ReadID();

 Prints("Your D12 chip\'s ID is: ");
 PrintShortIntHex(id);

#ifdef SHOW_MORE_MSG //节省ROM空间
 if(id==0x1012)
 {
  Prints(". ID is correct! Congratulations!\r\n\r\n");
 }
 else
 {
  Prints(". ID is incorrect! What a pity!\r\n\r\n");
 }
#endif

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
    if(KeyUp || KeyDown || KeyPress) //如果有按键事件发生
    {
     if(TouchMode == MULTI_TOUCH_MODE) //多点触摸模式
     {
      MultiPointSendReport();  //则返回多点报告
     }
     else //鼠标或单点模式
     {
      SendReport();  //则返回单点报告
     }
    }
    KeyCanChange=1;  //允许按键扫描
   }
  }
 }
}
////////////////////////End of function//////////////////////////////
