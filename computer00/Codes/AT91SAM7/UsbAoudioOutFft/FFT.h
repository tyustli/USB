/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
   
        欢迎访问我的USB专区：http://group.ednchina.com/93/
          欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                               http://computer00.21ic.org
                               
FFT.h file
                              
作者：Computer-lov
建立日期: 2009-03-06
修改日期: 2009-03-06
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/

#ifndef __FFT_H__
#define __FFT_H__

#include <math.h>

//定义做FFT的点数
#define LENGTH 512
//定义这么多点的FFT需要多少个二进制位
//bL=log2(LENGTH)
#define bL 9

#define IN_TYPE  short int
#define OUT_TYPE long int
#define LEN_TYPE short int

#define PI 3.1415926535897932384626433832795

void InitBitRev(void);
void FftInput(IN_TYPE *pIn);
void FftExe(IN_TYPE *pIn, OUT_TYPE *pRe, OUT_TYPE *pIm);
void DftExe(IN_TYPE *pIn, OUT_TYPE *pRe, OUT_TYPE *pIm);

#endif