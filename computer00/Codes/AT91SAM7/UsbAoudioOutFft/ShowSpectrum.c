/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

ShowSpectrum.c  file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-03-10
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#include "ShowSpectrum.h"
#include "Lcd.h"

OUT_TYPE Re[LENGTH];
OUT_TYPE Im[LENGTH];

unsigned char Peak[64];
unsigned char PeakTime[64];

#define MAX_HEIGHT 39

#define HOLD_TIME 3   //峰值保留时间

/********************************************************************
函数功能：对需要显示的峰值进行处理。
入口参数：x：横坐标；y：纵坐标值。
返    回：无。
备    注：无。
********************************************************************/
void ProcPeak(unsigned char x, unsigned char y)
{
 if(PeakTime[x]>0)
 {
  PeakTime[x]--;
 }
 else
 {
  if(Peak[x]<MAX_HEIGHT)Peak[x]+=2;
 }
 if(y<Peak[x])
 {
  Peak[x]=y; //修改峰值
  PeakTime[x]=HOLD_TIME; //保持
 }
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：幅度压缩。
入口参数：y：输入的幅度。
返    回：无。
备    注：压缩后的幅度。
********************************************************************/
int Compress(int y)
{
 return sqrt(y);  //开根号压缩
}
/////////////////////////End of function/////////////////////////////

/********************************************************************
函数功能：在LCD上显示频谱。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void ShowSpectrum(void)
{
 int i,x,y,j,p;
 
 //对输入信号进行缩小
 for(i=0;i<LENGTH;i++)
 {
  AudioOutBuf[i]/=32;
 }
 
 FftInput(AudioOutBuf);       //位倒序
 FftExe(AudioOutBuf,Re,Im);   //做FFT运算
  
 LcdCls();    //清屏
 
 //显示X、Y轴坐标
 LcdSetPoint(0,35);
 LcdPrints("0V");
 LcdDrawLine(13,39,15,39);
 LcdSetPoint(0,23);
 LcdPrints("1V");
 LcdDrawLine(13,27,15,27);
 LcdSetPoint(0,11);
 LcdPrints("2V");
 LcdDrawLine(13,15,15,15);
 LcdSetPoint(0,0);
 LcdPrints("3V");
 LcdDrawLine(13,3,15,3);
 
 LcdSetPoint(14,40);
 LcdPrints("0");
 LcdSetPoint(13+26,40);
 LcdPrints("3K");
 LcdSetPoint(84-20,40);
 LcdPrints("16K");
 
 //直流分量直接显示
 x=17;
 y=MAX_HEIGHT-(Re[0]*MAX_HEIGHT/LENGTH)/1024;
 LcdDrawLine(x,y,x,MAX_HEIGHT);
 ProcPeak(0,y); //处理峰值

 //每1点显示
 for(i=1;i<14;i++)
 {
  x++;
  y=sqrt(Re[i]*Re[i]+Im[i]*Im[i]);   //计算模值
  y/=10;
  y=Compress(y);  //压缩
  y=MAX_HEIGHT-y;  //换算为屏幕y轴位置
  ProcPeak(i,y); //处理峰值
  LcdDrawLine(x,y,x,MAX_HEIGHT);     //画线
 }
 
 //每2点显示
 p=14;
 for(i=14;i<24;i++)
 {
  x++;
  y=0;
  for(j=0;j<2;j++)
  {
   y+=sqrt(Re[p]*Re[p]+Im[p]*Im[p]);   //计算模值
   p++;
  }
  y/=20; //取平均值
  y=Compress(y);  //压缩
  y=MAX_HEIGHT-y;  //换算为屏幕y轴位置
  ProcPeak(i,y); //处理峰值
  LcdDrawLine(x,y,x,MAX_HEIGHT);     //画线
 }

 //每3点显示
 for(i=24;i<34;i++)
 {
  x++;
  y=0;
  for(j=0;j<3;j++)
  {
   y+=sqrt(Re[p]*Re[p]+Im[p]*Im[p]);   //计算模值
   p++;
  }
  y/=30; //取平均值
  y=Compress(y);  //压缩
  y=MAX_HEIGHT-y;  //换算为屏幕y轴位置
  ProcPeak(i,y); //处理峰值
  LcdDrawLine(x,y,x,MAX_HEIGHT);     //画线
 }

 //每4点显示
 for(i=34;i<44;i++)
 {
  x++;
  y=0;
  for(j=0;j<4;j++)
  {
   y+=sqrt(Re[p]*Re[p]+Im[p]*Im[p]);   //计算模值
   p++;
  }
  y/=40; //取平均值
  y=Compress(y);  //压缩
  y=MAX_HEIGHT-y;  //换算为屏幕y轴位置
  ProcPeak(i,y); //处理峰值
  LcdDrawLine(x,y,x,MAX_HEIGHT);     //画线
 }
 
 //每6点显示
 for(i=44;i<54;i++)
 {
  x++;
  y=0;
  for(j=0;j<6;j++)
  {
   y+=sqrt(Re[p]*Re[p]+Im[p]*Im[p]);   //计算模值
   p++;
  }
  y/=60; //取平均值
  y=Compress(y);  //压缩
  y=MAX_HEIGHT-y;  //换算为屏幕y轴位置
  ProcPeak(i,y); //处理峰值
  LcdDrawLine(x,y,x,MAX_HEIGHT);     //画线
 }
 
 //每9点显示
 for(i=54;i<64;i++)
 {
  x++;
  y=0;
  for(j=0;j<9;j++)
  {
   y+=sqrt(Re[p]*Re[p]+Im[p]*Im[p]);   //计算模值
   p++;
  }
  y/=90; //取平均值
  y=Compress(y);  //压缩
  y=MAX_HEIGHT-y;  //换算为屏幕y轴位置
  ProcPeak(i,y);   //处理峰值
  LcdDrawLine(x,y,x,MAX_HEIGHT);     //画线
 }
 
 for(i=0;i<64;i++)
 {
  LcdDrawPoint(i+17,Peak[i]);
 }
 LcdRefresh(); //刷新
}
/////////////////////////End of function/////////////////////////////