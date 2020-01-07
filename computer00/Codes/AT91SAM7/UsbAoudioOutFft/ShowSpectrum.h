/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途

        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

ShowSpectrum.h file

作者：Computer-lov
建立日期: 2009-03-04
修改日期: 2009-03-10
版本：V1.0
版权所有，盗版必究。
Copyright(C) Computer-lov 2009-2019
All rights reserved
*******************************************************************/


#ifndef __SHOW_SPECTRUM_H__
#define __SHOW_SPECTRUM_H__

#include "MyType.h"
#include "FFT.h"

void ShowSpectrum(void);

extern int16 AudioOutBuf[650];   //需要采样512点
extern uint32 AudioOutByteCount;  //输出字节计数器

#endif