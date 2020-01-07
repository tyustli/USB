/******************************************************************
   本程序只供学习使用，未经作者许可，不得用于其它任何用途
			
        欢迎访问我的USB专区：http://group.ednchina.com/93/
        欢迎访问我的blog：   http://www.ednchina.com/blog/computer00
                             http://computer00.21ic.org

        感谢PCB赞助商——电子园： http://bbs.cepark.com/

UsbCore.c file

作者：电脑圈圈
建立日期: 2008.06.29
修改日期: 2012.01.26
版本：V1.1
版权所有，盗版必究。
Copyright(C) 电脑圈圈 2008-2018
All rights reserved            
*******************************************************************/

#include "config.h"
#include "pdiusbd12.h"
#include "uart.h"
#include "usbcore.h"

idata uint8 Buffer[16];  //读端点0用的缓冲区

//USB设备请求的各字段
uint8  bmRequestType;
uint8  bRequest;
uint16 wValue;
uint16 wIndex;
uint16 wLength;
//当前发送数据的位置
uint8 * pSendData;
//需要发送数据的长度
uint16 SendLength;
//是否需要发送0数据包的标志。在USB控制传输的数据过程中，
//当返回的数据包字节数少于最大包长时，会认为数据过程结束。
//当请求的字节数比实际需要返回的字节数长，而实际返回的字节
//数又刚好是端点0大小的整数倍时，就需要返回一个0长度的数据包
//来结束数据过程。因此这里增加一个标志，供程序决定是否需要返回
//一个0长度的数据包。
uint8 NeedZeroPacket;

//当前的配置值。只有在设置非0配置后
uint8 ConfigValue;

//端点1缓冲是否忙的标志。当缓冲区中有数据时，该标志为真。
//当缓冲区中空闲时，该标志为假。
uint8 Ep1InIsBusy;

//USB设备描述符的定义
code uint8 DeviceDescriptor[0x12]=  //设备描述符为18字节
{
//bLength字段。设备描述符的长度为18(0x12)字节
 0x12,
 
//bDescriptorType字段。设备描述符的编号为0x01
 0x01,
 
//bcdUSB字段。这里设置版本为USB1.1，即0x0110。
//由于是小端结构，所以低字节在先，即0x10，0x01。
 0x10,
 0x01,
 
//bDeviceClass字段。我们不在设备描述符中定义设备类，
//而在接口描述符中定义设备类，所以该字段的值为0。
 0x00,
 
//bDeviceSubClass字段。bDeviceClass字段为0时，该字段也为0。
 0x00,
 
//bDeviceProtocol字段。bDeviceClass字段为0时，该字段也为0。
 0x00,
 
//bMaxPacketSize0字段。PDIUSBD12的端点0大小的16字节。
 0x10,
 
//idVender字段。厂商ID号，我们这里取0x8888，仅供实验用。
//实际产品不能随便使用厂商ID号，必须跟USB协会申请厂商ID号。
//注意小端模式，低字节在先。
 0x88,
 0x88,
 
//idProduct字段。产品ID号，由于是第二版的第二个实验，我们这里取0x0102。
//注意小端模式，低字节应该在前。
 0x02,
 0x01,
 
//bcdDevice字段。我们这个USB触摸屏刚开始做，就叫它1.0版吧，即0x0100。
//小端模式，低字节在先。
 0x00,
 0x01,
 
//iManufacturer字段。厂商字符串的索引值，为了方便记忆和管理，
//字符串索引就从1开始吧。
 0x01,

//iProduct字段。产品字符串的索引值。刚刚用了1，这里就取2吧。
//注意字符串索引值不要使用相同的值。
 0x02,
 
//iSerialNumber字段。设备的序列号字符串索引值。
//这里取3就可以了。
 0x03,

//bNumConfigurations字段。该设备所具有的配置数。
//我们只需要一种配置就行了，因此该值设置为1。
 0x01
};
//////////////////////////设备描述符完毕//////////////////////////////

//WIN7多点触摸的白皮书文档下载地址：
//http://download.microsoft.com/download/a/d/f/adf1347d-08dc-41a4-9084-623b1194d4b2/DigitizerDrvs_touch.docx

//USB报告描述符的定义
code uint8 ReportDescriptor[]=
{
 /***********************以下是单点触摸报告描述符********************/
 //每行开始的第一字节为该条目的前缀，前缀的格式为：
 //D7~D4：bTag。D3~D2：bType；D1~D0：bSize。以下分别对每个条目注释。
 
 //这是一个全局（bType为1）条目，选择用途页为普通桌面Generic Desktop Page(0x01)
 //后面跟一字节数据（bSize为1），后面的字节数就不注释了，
 //自己根据bSize来判断。
 0x05, 0x01, // USAGE_PAGE (Generic Desktop)
 
 //这是一个局部（bType为2）条目，说明接下来的应用集合用途用于鼠标
 0x09, 0x02, // USAGE (Mouse)
 
 //这是一个主（bType为0）条目，开集合，后面跟的数据0x01表示
 //该集合是一个应用集合。它的性质在前面由用途页和用途定义为
 //普通桌面用的鼠标。
 0xa1, 0x01, // COLLECTION (Application)
 
 //全局条目，定义单点触摸的报告ID为REPORTID_STOUCH(usbcore.h中定义)
 0x85, REPORTID_STOUCH, //   REPORT_ID (Single Touch)
 
 //这是一个局部条目。说明用途为指针集合
 0x09, 0x01, //   USAGE (Pointer)
 
 //这是一个主条目，开集合，后面跟的数据0x00表示该集合是一个
 //物理集合，用途由前面的局部条目定义为指针集合。
 0xa1, 0x00, //   COLLECTION (Physical)
 
 //这是一个全局条目，选择用途页为按键（Button Page(0x09)）
 0x05, 0x09, //     USAGE_PAGE (Button)
 
 //这是一个局部条目，说明用途的最小值为1。实际上是鼠标左键。
 0x19, 0x01, //     USAGE_MINIMUM (Button 1)
 
 //这是一个局部条目，说明用途的最大值为3。实际上是鼠标中键。
 0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
 
 //这是一个全局条目，说明返回的数据的逻辑值（就是我们返回的数据域的值啦）
 //最小为0。因为我们这里用Bit来表示一个数据域，因此最小为0，最大为1。
 0x15, 0x00, //     LOGICAL_MINIMUM (0)
 
 //这是一个全局条目，说明逻辑值最大为1。
 0x25, 0x01, //     LOGICAL_MAXIMUM (1)
 
 //这是一个全局条目，说明数据域的数量为三个。
 0x95, 0x03, //     REPORT_COUNT (3)
 
 //这是一个全局条目，说明每个数据域的长度为1个bit。
 0x75, 0x01, //     REPORT_SIZE (1)
 
 //这是一个主条目，说明有3个长度为1bit的数据域（数量和长度
 //由前面的两个全局条目所定义）用来做为输入，
 //属性为：Data,Var,Abs。Data表示这些数据可以变动，Var表示
 //这些数据域是独立的，每个域表示一个意思。Abs表示绝对值。
 //这样定义的结果就是，第一个数据域bit0表示按键1（左键）是否按下，
 //第二个数据域bit1表示按键2（右键）是否按下，第三个数据域bit2表示
 //按键3（中键）是否按下。
 0x81, 0x02, //     INPUT (Data,Var,Abs)
 
 //这是一个全局条目，说明数据域数量为1个
 0x95, 0x01, //     REPORT_COUNT (1)
 
 //这是一个全局条目，说明每个数据域的长度为5bit。
 0x75, 0x05, //     REPORT_SIZE (5)
 
 //这是一个主条目，输入用，由前面两个全局条目可知，长度为5bit，
 //数量为1个。它的属性为常量（即返回的数据一直是0）。
 //这个只是为了凑齐一个字节（前面用了3个bit）而填充的一些数据
 //而已，所以它是没有实际用途的。
 0x81, 0x03, //     INPUT (Cnst,Var,Abs)
  
 //这是一个全局条目，选择用途页为普通桌面Generic Desktop Page(0x01)
 0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
 
 //这是一个局部条目，说明用途为X轴
 0x09, 0x30, //     USAGE (X)
 
 //这是一个局部条目，说明用途为Y轴
 0x09, 0x31, //     USAGE (Y)

 //下面两个为全局条目，说明返回的逻辑最小和最大值。
 //这里定义X和Y轴的逻辑最小值为0，即坐标原点
 //X和Y轴的逻辑最大值为4096，即屏幕右下方的坐标为（4096，4096）。
 //由于4096超过了一字节的范围，所以需要用2字节的格式表示最大值
 0x15, 0x00, //     LOGICAL_MINIMUM (0)
 0x26, 0x00, 0x10, //     LOGICAL_MAXIMUM (4096)

 //下面两个为全局条目，说明返回的物理最小和最大值。
 //这里定义X和Y轴的物理最小值为0，即坐标原点
 //X和Y轴的物理最大值为4096，即屏幕右下方的坐标为（4096，4096）。
 //由于4096超过了一字节的范围，所以需要用2字节的格式表示最大值
 0x35, 0x00,    //Physical Minimum (0)
 0x46, 0x00, 0x10, //Physical Maximum(4096)

 //这是一个全局条目，说明数据域的长度为16bit。
 0x75, 0x10, //     REPORT_SIZE (16)
 
 //这是一个全局条目，说明数据域的个数为2个。
 0x95, 0x02, //     REPORT_COUNT (2)
 
 //这是一个主条目。它说明这两个16bit的数据域是输入用的，
 //属性为：Data,Var,Abs。Data说明数据是可以变的，Var说明
 //这些数据域是独立的，Abs表示这些值是绝对值。
 0x81, 0x02, //     INPUT (Data,Var,Abs)
 
 //下面这两个主条目用来关闭前面的集合用。
 //我们开了两个集合，所以要关两次。bSize为0，所以后面没数据。
 0xc0,       //   END_COLLECTION
 0xc0,       // END_COLLECTION
 /***********************单点触摸描述完毕***************************/

 /***********************以下是多点触摸报告描述*********************/
 //这是一个全局条目，选择用途页为Digitizers
 0x05, 0x0d,  // USAGE_PAGE (Digitizers)

 //这是一个局部条目，说明接下来的应用集合用途用于Touch Screen
 0x09, 0x04,  // USAGE (Touch Screen)

 //这是一个主条目，开集合，后面跟的数据0x01表示该集合是一个应用集合。
 //它的性质在前面由用途页和用途定义为Digitizers的Touch Screen。
 0xa1, 0x01,  // COLLECTION (Application)
 
 //全局条目，定义多点触摸的报告ID为REPORTID_MTOUCH(usbcore.h中定义)
 0x85, REPORTID_MTOUCH,  //   REPORT_ID (Touch)
 
 //这是一个局部条目，选择用途为Digitizers页中的Finger
 0x09, 0x22,  //   USAGE (Finger)
 
 //这是一个主条目，开集合，后面跟的数据0x02表示该集合是一个逻辑集合。
 //它的性质在前面由用途页和用途定义为Digitizers的Finger。
 0xa1, 0x02,  //     COLLECTION (Logical)
 
 //这是一个局部条目，选择用途为Digitizers页中的Tip Switch，
 //本来用来表示触摸笔是否接触到屏幕，这里用来表示是否有手指触摸。
 0x09, 0x42,  //       USAGE (Tip Switch)
 
 //这是一个全局条目，说明返回的数据的逻辑最小值为0。
 0x15, 0x00,  //       LOGICAL_MINIMUM (0)
 
 //这是一个全局条目，说明返回的数据的逻辑最大值为1。
 0x25, 0x01,  //       LOGICAL_MAXIMUM (1)
 
 //这是一个全局条目，说明每个数据域的长度为1个bit。
 0x75, 0x01,  //       REPORT_SIZE (1)
 
 //这是一个全局条目，说明数据域的数量为1个。
 0x95, 0x01,  //       REPORT_COUNT (1)
 
 //这是一个主条目，说明有1个长度为1bit的数据域用于输入。
 //通过前面的描述我们可以知道，这个bit是用来表示是否触摸的，
 //0表示没有触摸，1表示触摸。
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 
 //这是一个局部条目，选择用途为Digitizers页中的In Range，
 //用来表示触摸的区域是否有效。
 0x09, 0x32,  //       USAGE (In Range)
 
 //这是一个主条目，说明有1个长度为1bit的数据域用于输入。
 //通过前面的描述我们可以知道，这个bit是用来表示触摸区域是否有效的，
 //0表示无效，1表示有效。
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 
 //这是一个局部条目，选择用途为Digitizers页中的Touch Valid，
 //用来表示触摸是否有效。
 0x09, 0x47,  //       USAGE (Touch Valid)
 
 //这是一个主条目，说明有1个长度为1bit的数据域用于输入。
 //通过前面的描述我们可以知道，这个bit是用来表示触摸是否有效的，
 //0表示无效，1表示有效。
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 
 //这是一个全局条目，说明数据域的数量为5个。
 0x95, 0x05,  //       REPORT_COUNT (5)
 
 //这是一个主条目，输入用，由前面两个全局条目可知，长度为1bit，
 //数量为5个。它的属性为常量（即返回的数据一直是0）。
 //这个只是为了凑齐一个字节（前面用了3个bit）而填充的一些数据
 //而已，所以它是没有实际用途的。
 0x81, 0x03,  //       INPUT (Cnst,Ary,Abs)
 
 //这是一个全局条目，说明返回的数据的逻辑最大值为MAX_TOUCH_POINT。
 0x25, MAX_TOUCH_POINT,  // LOGICAL_MAXIMUM (MAX_TOUCH_POINT)
 
 //这是一个全局条目，说明每个数据域的长度为8个bit。
 0x75, 0x08,  //       REPORT_SIZE (8)
 
 //这是一个局部条目，选择用途为Digitizers页中的Contact Identifier，
 //用来表示触摸的ID号。
 0x09, 0x51,  //       USAGE (Contact Identifier)
 
 //这是一个全局条目，说明数据域的数量为1个。
 0x95, 0x01,  //       REPORT_COUNT (1)
 
 //这是一个主条目，说明有1个长度为8bit的数据域用于输入。
 //通过前面的描述我们可以知道，这个8bit是用来表示触摸ID号的，
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 
 //这是一个全局条目，选择用途页为Generic Desktop
 0x05, 0x01,  //       USAGE_PAGE (Generic Desktop)
 
 //这是一个全局条目，说明返回的数据的逻辑最大值为4096。
 0x26, 0x00, 0x10,  //       LOGICAL_MAXIMUM (4096)
 
 //这是一个全局条目，说明每个数据域的长度为16个bit。
 0x75, 0x10,  //       REPORT_SIZE (16)
 
 //这是一个全局条目，说明物理单位的指数为0
 0x55, 0x00,  //       UNIT_EXPONENT (0)
 
 //这是一个全局条目，说明没有物理单位
 0x65, 0x00,  //       UNIT (None)
 
 //这是一个局部条目，选择用途为通用桌面中的X轴
 0x09, 0x30,  //       USAGE (X)
 
 //这是一个全局条目，说明物理最小值为0。由于这里用不到物理最小、
 //最大值，所以将物理最小、最大值描述成0，并将单位描述成无。
 0x35, 0x00,  //       PHYSICAL_MINIMUM (0)
 
 //这是一个全局条目，说明物理最大值为0。
 0x46, 0x00, 0x00,  //       PHYSICAL_MAXIMUM (0)
 
 //这是一个主条目。它说明这个16bit的数据域是输入用的，
 //属性为：Data,Var,Abs。Data说明数据是可以变的，Var说明
 //这些数据域是独立的，Abs表示这些值是绝对值。它其实就是
 //我们要返回的X坐标数据。
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 
 //这是一个局部条目，选择用途为通用桌面中的Y轴
 0x09, 0x31,  //       USAGE (Y)
 
 //这是一个全局条目，说明物理最大值为0。
 0x46, 0x00, 0x00,  //       PHYSICAL_MAXIMUM (0)
 
 //这是一个主条目。它说明这个16bit的数据域是输入用的，
 //属性为：Data,Var,Abs。Data说明数据是可以变的，Var说明
 //这些数据域是独立的，Abs表示这些值是绝对值。它其实就是
 //我们要返回的Y坐标数据。
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 
 //这是一个主条目，关闭前面开的逻辑集合
 0xc0,        //    END_COLLECTION
 
 //从这里一直到下面的END_COLLECTION，基本上是上一个逻辑集合的重复，
 //这里就不再给出注释了，请读者自行分析。
 0xa1, 0x02,  //    COLLECTION (Logical)
 0x05, 0x0d,  //     USAGE_PAGE (Digitizers)
 0x09, 0x42,  //       USAGE (Tip Switch)
 0x15, 0x00,  //       LOGICAL_MINIMUM (0)
 0x25, 0x01,  //       LOGICAL_MAXIMUM (1)
 0x75, 0x01,  //       REPORT_SIZE (1)
 0x95, 0x01,  //       REPORT_COUNT (1)
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 0x09, 0x32,  //       USAGE (In Range)
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 0x09, 0x47,  //       USAGE (Touch Valid)
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 0x95, 0x05,  //       REPORT_COUNT (5)
 0x81, 0x03,  //       INPUT (Cnst,Ary,Abs)
 0x25, MAX_TOUCH_POINT,  // LOGICAL_MAXIMUM (MAX_TOUCH_POINT)
 0x75, 0x08,  //       REPORT_SIZE (8)
 0x09, 0x51,  //       USAGE ( Contact Identifier)
 0x95, 0x01,  //       REPORT_COUNT (1)
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 0x05, 0x01,  //       USAGE_PAGE (Generic Desk..
 0x26, 0x00, 0x10,  //       LOGICAL_MAXIMUM (4096)
 0x75, 0x10,  //       REPORT_SIZE (16)
 0x55, 0x00,  //       UNIT_EXPONENT (0)
 0x65, 0x00,  //       UNIT (None)
 0x09, 0x30,  //       USAGE (X)
 0x35, 0x00,  //       PHYSICAL_MINIMUM (0)
 0x46, 0x00, 0x00,  //       PHYSICAL_MAXIMUM (0)
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 0x09, 0x31,  //       USAGE (Y)
 0x46, 0x00, 0x00,  //       PHYSICAL_MAXIMUM (0)
 0x81, 0x02,  //       INPUT (Data,Var,Abs)
 0xc0,        //    END_COLLECTION
 
 //这是一个全局条目，选择用途页为Digitizers
 0x05, 0x0d,  //    USAGE_PAGE (Digitizers)
 
 //这是一个局部条目，选择用途为Contact Count，即当前有多少点触摸
 0x09, 0x54,  //    USAGE (Contact Count)
 
 //这是一个全局条目，说明数据域的数量为1个。
 0x95, 0x01,  //    REPORT_COUNT (1)
 
 //这是一个全局条目，说明每个数据域的长度为8个bit。
 0x75, 0x08,  //    REPORT_SIZE (8)
 
 //这是一个全局条目，说明返回的数据的逻辑最小值为0。
 0x15, 0x00,  //    LOGICAL_MINIMUM (0)
 
 //这是一个全局条目，说明返回的数据的逻辑最大值为MAX_TOUCH_POINT。
 0x25, MAX_TOUCH_POINT,  //    LOGICAL_MAXIMUM (MAX_TOUCH_POINT)
 
 //这是一个主条目，说明有1个长度为8bit的数据域用于输入。
 //通过前面的描述我们可以知道，这个8bit是用来表示当前有几个点触摸，
 //0表示没有触摸，1表示触摸1个点触摸，2表示2个点触摸等等。
 0x81, 0x02,  //    INPUT (Data,Var,Abs)
 
 //这是一个局部条目，选择用途为Contact Count Maximum，
 //表示最多支持多少点同时触摸。
 0x09, 0x55,  //    USAGE(Contact Count Maximum)
 
 //这是一个主条目，说明有1个长度为8bit的数据域用于特性报告。
 //通过前面的描述我们可以知道，这个8bit是用来表示最多支持几个点触摸。
 0xb1, 0x02,  //    FEATURE (Data,Var,Abs)
 
 //这是一个主条目，关集合
 0xc0,        // END_COLLECTION
 /***********************多点触摸描述完毕***************************/
 
 /*****************以下是配置设备的特性报告描述符*******************/
 //这是一个全局条目，选择用途页为Digitizers
 0x05, 0x0d,  //    USAGE_PAGE (Digitizers)
 
 //这是一个局部条目，选择用途为Device Configuration
 0x09, 0x0E,  // USAGE (Device Configuration)
 
 //这是一个主条目，开集合，后面跟的数据0x01表示该集合是一个应用集合。
 //它的性质在前面由用途页和用途定义为Digitizers的Device Configuration。
 0xa1, 0x01,  // COLLECTION (Application)
 
 //全局条目，定义特性报告ID为REPORTID_FEATURE(usbcore.h中定义)
 0x85, REPORTID_FEATURE,  //   REPORT_ID (Configuration)
 
 //这是一个局部条目，选择用途为Device Settings
 0x09, 0x23,  //   USAGE (Device Settings)
 
 //这是一个主条目，开集合，后面跟的数据0x02表示该集合是一个逻辑集合。
 0xa1, 0x02,  //   COLLECTION (logical)
 
 //这是一个局部条目，选择用途为Device Mode
 0x09, 0x52,  //    USAGE (Device Mode)
 
 //这是一个局部条目，选择用途为Device Identifier
 0x09, 0x53,  //    USAGE (Device Identifier)
 
 //这是一个全局条目，说明返回的数据的逻辑最小值为0。
 0x15, 0x00,  //    LOGICAL_MINIMUM (0)
 
 //这是一个全局条目，说明返回的数据的逻辑最大值为10。
 0x25, 0x0a,  //    LOGICAL_MAXIMUM (10)
 
 //这是一个全局条目，说明每个数据域的长度为8个bit。
 0x75, 0x08,  //    REPORT_SIZE (8)
 
 //这是一个全局条目，说明数据域的数量为2个。
 0x95, 0x02,  //    REPORT_COUNT (2)
 
 //这是一个主条目，说明有2个长度为8bit的数据域用于特性报告。
 //通过前面的描述我们可以知道，这2个8bit分别用来表示Device Mode和
 //Device Identifier。
 0xb1, 0x02,  //   FEATURE (Data,Var,Abs)
 0xc0,        //   END_COLLECTION
 0xc0        // END_COLLECTION
 /***********************特性报告描述完毕***************************/
};
//通过上面的报告描述符的定义，我们知道总共定义了三个报告。
/////////////////////////////////////////////////////////////////////
//第一个报告为单点触摸的报告，第一字节为报告ID，值为REPORTID_STOUCH。
//第二字节的低3位用来表示按键是否按下的，高5位为常数0，无用。
//第三、四字节表示X轴的绝对坐标，第五、六字节表示Y轴的绝对坐标。
/////////////////////////////////////////////////////////////////////
//第二个报告为多点触摸的报告，第一字节为报告ID，值为REPORTID_MTOUCH。
//第二字节的低3位用来表示第一点状态的，高5位为常数0，无用。
//第三字节表示第一点触摸的ID号。
//第四、五字节表示第一点X轴的绝对坐标，第六、七字节表示第一点Y轴的绝对坐标。
//第八字节的低3位用来表示第二点状态的，高5位为常数0，无用。
//第九字节表示第二点触摸的ID号。
//第十、十一字节表示第二点X轴的绝对坐标，第十二、十三节表示第二点Y轴的绝对坐标。
//第十四字节表示当前有多少点触摸。
//另外还有一个两字节的特性报告，第一字节为报告ID，值为REPORTID_MTOUCH，
//第二字节为设备支持的最大触摸点数，主机会通过GET_REPORT请求来获取它。
/////////////////////////////////////////////////////////////////////
//第三个报告为特性报告，第一字节为报告ID，值为REPORTID_FEATURE。
//第二字节表示Device Mode，第三字节表示Device Identifier。
/////////////////////////////////////////////////////////////////////
///////////////////////////报告描述符完毕////////////////////////////

//最多支持的点数的报告，这里固定为MAX_TOUCH_POINT，
//即最多只能同时支持MAX_TOUCH_POINT点触摸
code uint8 MaxContactCountReport[2] =
{
 REPORTID_MTOUCH, //报告ID
 MAX_TOUCH_POINT  //最多同时支持MAX_TOUCH_POINT点触摸
};

//USB配置描述符集合的定义
//配置描述符总长度为9+9+9+7字节
code uint8 ConfigurationDescriptor[9+9+9+7]=
{
 /***************配置描述符***********************/
 //bLength字段。配置描述符的长度为9字节。
 0x09,
 
 //bDescriptorType字段。配置描述符编号为0x02。
 0x02,
 
 //wTotalLength字段。配置描述符集合的总长度，
 //包括配置描述符本身、接口描述符、类描述符、端点描述符等。
 sizeof(ConfigurationDescriptor)&0xFF, //低字节
 (sizeof(ConfigurationDescriptor)>>8)&0xFF, //高字节
 
 //bNumInterfaces字段。该配置包含的接口数，只有一个接口。
 0x01,
 
 //bConfiguration字段。该配置的值为1。
 0x01,
 
 //iConfigurationz字段，该配置的字符串索引。这里没有，为0。
 0x00,
 
 //bmAttributes字段，该设备的属性。由于我们的板子是总线供电的，
 //并且我们不想实现远程唤醒的功能，所以该字段的值为0x80。
 0x80,
 
 //bMaxPower字段，该设备需要的最大电流量。由于我们的板子
 //需要的电流不到100mA，因此我们这里设置为100mA。由于每单位
 //电流为2mA，所以这里设置为50(0x32)。
 0x32,
 
 /*******************接口描述符*********************/
 //bLength字段。接口描述符的长度为9字节。
 0x09,
 
 //bDescriptorType字段。接口描述符的编号为0x04。
 0x04,
 
 //bInterfaceNumber字段。该接口的编号，第一个接口，编号为0。
 0x00,
 
 //bAlternateSetting字段。该接口的备用编号，为0。
 0x00,
 
 //bNumEndpoints字段。非0端点的数目。由于USB触摸屏只需要一个
 //中断输入端点，因此该值为1。
 0x01,
 
 //bInterfaceClass字段。该接口所使用的类。USB触摸屏是HID类，
 //HID类的编码为0x03。
 0x03,
 
 //bInterfaceSubClass字段。该接口所使用的子类。在HID1.1协议中，
 //只规定了一种子类：支持BIOS引导启动的子类。
 //USB键盘、鼠标属于该子类，子类代码为0x01。
 0x00,
 
 //bInterfaceProtocol字段。如果子类为支持引导启动的子类，
 //则协议可选择鼠标和键盘。键盘代码为0x01，鼠标代码为0x02。
 0x00,
 
 //iConfiguration字段。该接口的字符串索引值。这里没有，为0。
 0x00,
 
 /******************HID描述符************************/
 //bLength字段。本HID描述符下只有一个下级描述符。所以长度为9字节。
 0x09,
 
 //bDescriptorType字段。HID描述符的编号为0x21。
 0x21,
 
 //bcdHID字段。本协议使用的HID1.1协议。注意低字节在先。
 0x10,
 0x01,
 
 //bCountyCode字段。设备适用的国家代码，这里选择为美国，代码0x21。
 0x21,
 
 //bNumDescriptors字段。下级描述符的数目。我们只有一个报告描述符。
 0x01,
 
 //bDescritporType字段。下级描述符的类型，为报告描述符，编号为0x22。
 0x22,
 
 //bDescriptorLength字段。下级描述符的长度。下级描述符为报告描述符。
 sizeof(ReportDescriptor)&0xFF,
 (sizeof(ReportDescriptor)>>8)&0xFF,
 
 /**********************端点描述符***********************/
 //bLength字段。端点描述符长度为7字节。
 0x07,
 
 //bDescriptorType字段。端点描述符编号为0x05。
 0x05,
 
 //bEndpointAddress字段。端点的地址。我们使用D12的输入端点1。
 //D7位表示数据方向，输入端点D7为1。所以输入端点1的地址为0x81。
 0x81,
 
 //bmAttributes字段。D1~D0为端点传输类型选择。
 //该端点为中断端点。中断端点的编号为3。其它位保留为0。
 0x03,
 
 //wMaxPacketSize字段。该端点的最大包长。端点1的最大包长为16字节。
 //注意低字节在先。
 0x10,
 0x00,
 
 //bInterval字段。端点查询的时间，我们设置为10个帧时间，即10ms。
 0x0a
};
////////////////////////配置描述符集合完毕//////////////////////////

/************************语言ID的定义********************/
code uint8 LanguageId[4]=
{
 0x04, //本描述符的长度
 0x03, //字符串描述符
 //0x0409为美式英语的ID
 0x09,
 0x04
};
////////////////////////语言ID完毕//////////////////////////////////

/**************************************************/
/*********        本转换结果来自         **********/
/********* Http://computer00.21ic.org    **********/
/*********        作者: 电脑圈圈         **********/
/*********         欢迎大家使用          **********/
/*********    版权所有，盗版请写明出处   **********/
/**************************************************/

//http://computer00.21ic.org/user1/2198/archives/2007/42769.html
//字符串“电脑圈圈的USB专区 Http://group.ednchina.com/93/”的Unicode编码
//8位小端格式
code uint8 ManufacturerStringDescriptor[82]={
82,         //该描述符的长度为82字节
0x03,       //字符串描述符的类型编码为0x03
0x35, 0x75, //电
0x11, 0x81, //脑
0x08, 0x57, //圈
0x08, 0x57, //圈
0x84, 0x76, //的
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x13, 0x4e, //专
0x3a, 0x53, //区
0x20, 0x00, // 
0x48, 0x00, //H
0x74, 0x00, //t
0x74, 0x00, //t
0x70, 0x00, //p
0x3a, 0x00, //:
0x2f, 0x00, ///
0x2f, 0x00, ///
0x67, 0x00, //g
0x72, 0x00, //r
0x6f, 0x00, //o
0x75, 0x00, //u
0x70, 0x00, //p
0x2e, 0x00, //.
0x65, 0x00, //e
0x64, 0x00, //d
0x6e, 0x00, //n
0x63, 0x00, //c
0x68, 0x00, //h
0x69, 0x00, //i
0x6e, 0x00, //n
0x61, 0x00, //a
0x2e, 0x00, //.
0x63, 0x00, //c
0x6f, 0x00, //o
0x6d, 0x00, //m
0x2f, 0x00, ///
0x39, 0x00, //9
0x33, 0x00, //3
0x2f, 0x00  ///
};
/////////////////////////厂商字符串结束/////////////////////////////

//字符串“《圈圈教你玩USB》之USB多点触摸屏设备”的Unicode编码
//8位小端格式
code uint8 ProductStringDescriptor[44]={
44,         //该描述符的长度为44字节
0x03,       //字符串描述符的类型编码为0x03
0x0a, 0x30, //《
0x08, 0x57, //圈
0x08, 0x57, //圈
0x59, 0x65, //教
0x60, 0x4f, //你
0xa9, 0x73, //玩
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x0b, 0x30, //》
0x4b, 0x4e, //之
0x55, 0x00, //U
0x53, 0x00, //S
0x42, 0x00, //B
0x1a, 0x59, //多
0xb9, 0x70, //点
0xe6, 0x89, //触
0x78, 0x64, //摸
0x4f, 0x5c, //屏
0xbe, 0x8b, //设
0x07, 0x59  //备
};
////////////////////////产品字符串结束////////////////////////////

//字符串“2012-01-25”的Unicode编码
//8位小端格式
code uint8 SerialNumberStringDescriptor[22]={
22,         //该描述符的长度为22字节
0x03,       //字符串描述符的类型编码为0x03
0x32, 0x00, //2
0x30, 0x00, //0
0x31, 0x00, //1
0x32, 0x00, //2
0x2d, 0x00, //-
0x30, 0x00, //0
0x31, 0x00, //1
0x2d, 0x00, //-
0x32, 0x00, //2
0x35, 0x00  //5
};
//////////////////////产品序列号字符串结束/////////////////////////

/********************************************************************
函数功能：延时x毫秒函数。
入口参数：x：延时的毫秒数。
返    回：无。
备    注：无。
********************************************************************/
void DelayXms(uint16 x)                
{
 uint16 i;
 uint16 j;
 for(i=0;i<x;i++)
 for(j=0;j<227;j++); //循环语句延时
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：USB断开连接函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbDisconnect(void)
{
#ifdef DEBUG0
 Prints("断开USB连接。\r\n");
#endif
 D12WriteCommand(D12_SET_MODE);  //写设置模式命令
 D12WriteByte(0x06); //设置模式的第一字节
 D12WriteByte(0x47); //设置模式的第二字节
 DelayXms(1000);  //延迟1秒
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：USB连接函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbConnect(void)
{
#ifdef DEBUG0
 Prints("连接USB。\r\n");
#endif
 D12WriteCommand(D12_SET_MODE);  //写设置模式命令
 D12WriteByte(0x16); //设置模式的第一字节
 D12WriteByte(0x47); //设置模式的第二字节
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：总线挂起中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbBusSuspend(void)
{
#ifdef DEBUG0
 Prints("USB总线挂起。\r\n");
#endif
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：总线复位中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbBusReset(void)
{
#ifdef DEBUG0
 Prints("USB总线复位。\r\n");
#endif
 Ep1InIsBusy=0; //复位后端点1输入缓冲区空闲。
 TouchMode = MOUSE_MODE; //默认为鼠标模式
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：根据pData和SendLength将数据发送到端点0的函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbEp0SendData(void)
{
 //将数据写到端点中去准备发送
 //写之前要先判断一下需要发送的数据是否比端点0
 //最大长度大，如果超过端点大小，则一次只能发送
 //最大包长的数据。端点0的最大包长在DeviceDescriptor[7]
 if(SendLength>DeviceDescriptor[7])
 {
  //按最大包长度发送
  D12WriteEndpointBuffer(1,DeviceDescriptor[7],pSendData);
  //发送后剩余字节数减少最大包长
  SendLength-=DeviceDescriptor[7];
  //发送一次后指针位置要调整
  pSendData+= DeviceDescriptor[7];
 }
 else
 {
  if(SendLength!=0)
  {
   //不够最大包长，可以直接发送
   D12WriteEndpointBuffer(1,SendLength,pSendData);
   //发送完毕后，SendLength长度变为0
   SendLength=0;
  }
  else //如果要发送的数据包长度为0
  {
   if(NeedZeroPacket==1) //如果需要发送0长度数据
   {
    D12WriteEndpointBuffer(1,0,pSendData); //发送0长度数据包
    NeedZeroPacket=0; //清需要发送0长度数据包标志
   }
  }
 }
}
////////////////////////End of function//////////////////////////////

uint8 TouchMode = MOUSE_MODE; //默认为鼠标模式
/********************************************************************
函数功能：USB端点0数据过程数据处理函数。
入口参数：无。
返    回：无。
备    注：该函数用来处理0端点控制传输的数据或状态过程。
********************************************************************/
void UsbEp0DataOut(void)
{
 //由于本程序中只有一个请求输出数据，所以可以直接使用if语句判断条件，
 //如果有很多请求的话，使用if语句就不方便了，而应该使用switch语句散转。
 if((bmRequestType == 0x21) && //是类请求
    (bRequest == SET_REPORT) && //且请求类型是SET_REPORT
    ((wValue & 0xFF00) == (0x03 << 8)) && //且报告类型为3(Feature)
     ((wValue & 0xFF) == REPORTID_FEATURE) && //且报告ID为REPORTID_FEATURE
     (wIndex == 0) && (wLength == 3)) //且wIndex为0且wLength为3
 {
  uint8 ReportBuf[3];
  uint8 Length;
  //读回端点0中的数据
  Length=D12ReadEndpointBuffer(0,3,ReportBuf);
  D12ClearBuffer(); //清除缓冲区
  #ifdef DEBUG0
   Prints("设置特性报告(Device Mode和Device Identifier).\r\n");
  #endif
  
  if(Length == wLength) //如果读取数据的长度正确
  {
   if(ReportBuf[0] == REPORTID_FEATURE) //报告ID正确
   {
    TouchMode = ReportBuf[1];
    switch(TouchMode)
    {
     case MOUSE_MODE:
     #ifdef DEBUG0
      Prints("鼠标模式.\r\n");
     #endif
     break;
     
     case SINGLE_TOUCH_MODE:
     #ifdef DEBUG0
      Prints("单点触摸模式.\r\n");
     #endif
     break;
     
     case MULTI_TOUCH_MODE:
     #ifdef DEBUG0
      Prints("多点触摸模式.\r\n");
     #endif
     break;
     
     default:
     #ifdef DEBUG0
      Prints("未知模式.\r\n");
     #endif
     break;
    }
   }
  }
  //返回0长度的状态数据包。
  D12WriteEndpointBuffer(1,0,0);
 }
 else  //其它请求的数据过程或者状态过程
 {
  D12ReadEndpointBuffer(0,16,Buffer);
  D12ClearBuffer();
 }
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：端点0输出中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbEp0Out(void)
{
#ifdef DEBUG0
 Prints("USB端点0输出中断。\r\n");
#endif
 //读取端点0输出最后传输状态，该操作清除中断标志
 //并判断第5位是否为1，如果是，则说明是建立包
 if(D12ReadEndpointLastStatus(0)&0x20)
 {
  D12ReadEndpointBuffer(0,16,Buffer); //读建立过程数据
  D12AcknowledgeSetup(); //应答建立包
  D12ClearBuffer(); //清缓冲区
  //将缓冲数据填到设备请求的各字段中
  bmRequestType=Buffer[0];
  bRequest=Buffer[1];
  wValue=Buffer[2]+(((uint16)Buffer[3])<<8);
  wIndex=Buffer[4]+(((uint16)Buffer[5])<<8);
  wLength=Buffer[6]+(((uint16)Buffer[7])<<8);
  //下面的代码判断具体的请求，并根据不同的请求进行相关操作
  //如果D7位为1，则说明是输入请求
  if((bmRequestType&0x80)==0x80)
  {
   //根据bmRequestType的D6~5位散转，D6~5位表示请求的类型
   //0为标准请求，1为类请求，2为厂商请求。
   switch((bmRequestType>>5)&0x03)
   {
    case 0:  //标准请求
     #ifdef DEBUG0
      Prints("USB标准输入请求：");
     #endif
     //USB协议定义了几个标准输入请求，我们实现这些标准请求即可
     //请求的代码在bRequest中，对不同的请求代码进行散转
     //事实上，我们还需要对接收者进行散转，因为不同的请求接收者
     //是不一样的。接收者在bmRequestType的D4~D0位中定义。
     //我们这里为了简化操作，有些就省略了对接收者的判断。
     //例如获取描述符的请求，只根据描述符的类型来区别。
     switch(bRequest)
     {
      case GET_CONFIGURATION: //获取配置
       #ifdef DEBUG0
        Prints("获取配置。\r\n");
       #endif
      break;
      
      case GET_DESCRIPTOR:  //获取描述符
       #ifdef DEBUG0
        Prints("获取描述符——");
       #endif
       //对描述符类型进行散转，对于全速设备，
       //标准请求只支持发送到设备的设备、配置、字符串三种描述符
       switch((wValue>>8)&0xFF)
        {
         case DEVICE_DESCRIPTOR: //设备描述符
          #ifdef DEBUG0
           Prints("设备描述符。\r\n");
          #endif
          pSendData=DeviceDescriptor;  //需要发送的数据
          //判断请求的字节数是否比实际需要发送的字节数多
          //这里请求的是设备描述符，因此数据长度就是
          //DeviceDescriptor[0]。如果请求的比实际的长，
          //那么只返回实际长度的数据
          if(wLength>DeviceDescriptor[0])
          {
           SendLength=DeviceDescriptor[0];
           if(SendLength%DeviceDescriptor[7]==0) //并且刚好是整数个数据包时
           {
            NeedZeroPacket=1; //需要返回0长度的数据包
           }
          }
          else
          {
           SendLength=wLength;
          }
          //将数据通过EP0返回
          UsbEp0SendData();
         break;
         
         case CONFIGURATION_DESCRIPTOR:  //配置描述符
          #ifdef DEBUG0
           Prints("配置描述符。\r\n");
          #endif
          pSendData=ConfigurationDescriptor; //需要发送的数据为配置描述符
          //判断请求的字节数是否比实际需要发送的字节数多
          //这里请求的是配置描述符集合，因此数据长度就是
          //ConfigurationDescriptor[3]*256+ConfigurationDescriptor[2]。
          //如果请求的比实际的长，那么只返回实际长度的数据
          SendLength=ConfigurationDescriptor[3];
          SendLength=SendLength*256+ConfigurationDescriptor[2];
          if(wLength>SendLength)
          {
           if(SendLength%DeviceDescriptor[7]==0) //并且刚好是整数个数据包时
           {
            NeedZeroPacket=1; //需要返回0长度的数据包
           }
          }
          else
          {
           SendLength=wLength;
          }
          //将数据通过EP0返回
          UsbEp0SendData();
         break;
         
         case STRING_DESCRIPTOR:  //字符串描述符
          #ifdef DEBUG0
           Prints("字符串描述符");
          #endif
          switch(wValue&0xFF)  //根据wValue的低字节（索引值）散转
          {
           case 0:  //获取语言ID
            #ifdef DEBUG0
             Prints("(语言ID)。\r\n");
            #endif
            pSendData=LanguageId;
            SendLength=LanguageId[0];
           break;
           
           case 1:  //厂商字符串的索引值为1，所以这里为厂商字符串
           #ifdef DEBUG0
             Prints("(厂商描述)。\r\n");
            #endif
            pSendData=ManufacturerStringDescriptor;
            SendLength=ManufacturerStringDescriptor[0];
           break;
           
           case 2:  //产品字符串的索引值为2，所以这里为产品字符串
           #ifdef DEBUG0
             Prints("(产品描述)。\r\n");
            #endif
            pSendData=ProductStringDescriptor;
            SendLength=ProductStringDescriptor[0];
           break;
           
           case 3:  //产品序列号的索引值为3，所以这里为序列号
           #ifdef DEBUG0
             Prints("(产品序列号)。\r\n");
            #endif
            pSendData=SerialNumberStringDescriptor;
            SendLength=SerialNumberStringDescriptor[0];
           break;
           
           default :
            #ifdef DEBUG0
             Prints("(未知的索引值)。\r\n");
            #endif
            //对于未知索引值的请求，返回一个0长度的包
            SendLength=0;
            NeedZeroPacket=1;
           break;
          }
          //判断请求的字节数是否比实际需要发送的字节数多
          //如果请求的比实际的长，那么只返回实际长度的数据
          if(wLength>SendLength)
          {
           if(SendLength%DeviceDescriptor[7]==0) //并且刚好是整数个数据包时
           {
            NeedZeroPacket=1; //需要返回0长度的数据包
           }
          }
          else
          {
           SendLength=wLength;
          }
          //将数据通过EP0返回
          UsbEp0SendData();         
         break;

         case REPORT_DESCRIPTOR:  //报告描述符
          #ifdef DEBUG0
           Prints("报告描述符。\r\n");
          #endif
          pSendData=ReportDescriptor; //需要发送的数据为报告描述符
          SendLength=sizeof(ReportDescriptor); //需要返回的数据长度         
          //判断请求的字节数是否比实际需要发送的字节数多
          //如果请求的比实际的长，那么只返回实际长度的数据
          if(wLength>SendLength)
          {
           if(SendLength%DeviceDescriptor[7]==0) //并且刚好是整数个数据包时
           {
            NeedZeroPacket=1; //需要返回0长度的数据包
           }
          }
          else
          {
           SendLength=wLength;
          }
          //将数据通过EP0返回
          UsbEp0SendData();
         break;
                 
         default:  //其它描述符
          #ifdef DEBUG0
           Prints("其他描述符，描述符代码：");
           PrintHex((wValue>>8)&0xFF);
           Prints("\r\n");
          #endif
         break;
        }
       break;
      
      case GET_INTERFACE: //获取接口
       #ifdef DEBUG0
        Prints("获取接口。\r\n");
       #endif
      break;
      
      case GET_STATUS: //获取状态
       #ifdef DEBUG0
        Prints("获取状态。\r\n");
       #endif
      break;
      
      case SYNCH_FRAME: //同步帧
       #ifdef DEBUG0
        Prints("同步帧。\r\n");
       #endif
      break;
      
      default:  //未定义的标准请求
       #ifdef DEBUG0
        Prints("错误：未定义的标准输入请求。\r\n");
       #endif       
      break;
     }
    break;
    
    case 1:  //类请求
     #ifdef DEBUG0
      Prints("USB类输入请求：\r\n");
     #endif
     switch(bRequest)
     {
      case GET_REPORT:
      #ifdef DEBUG0
       Prints("获取报告......\r\n");
      #endif
      //报告类型为3(Feature)且报告ID为REPORTID_MTOUCH
      if(wValue == ((0x03 << 8) | REPORTID_MTOUCH))
      {
       if((wIndex == 0) && (wLength == 2)) //wIndex为0且wLength为2
       {
        //通过端点0返回我们的特性报告，这个特性报告就是我们在多点触摸
        //报告描述符中，最后描述的那个Contact Count Maximum，即系统要
        //获取能够支持最大的触摸点数，我们这里返回一个固定的值。
        //之所以这里请求的是2字节，是因为有一字节的报告ID。
        pSendData=MaxContactCountReport; //需要发送的数据
        SendLength=wLength; //需要返回的数据长度
        //将数据通过EP0返回
        UsbEp0SendData();
       }
      }
      break;
      
      default:
      #ifdef DEBUG0
       Prints("未知USB类输入请求。\r\n");
      #endif
      break;
     }
    break;
    
    case 2:  //厂商请求
     #ifdef DEBUG0
      Prints("USB厂商输入请求：\r\n");
     #endif
    break;
    
    default: //未定义的请求。这里只显示一个报错信息。
     #ifdef DEBUG0
      Prints("错误：未定义的输入请求。\r\n");
     #endif
    break;
   }
  }
  //否则说明是输出请求
  else //if(bmRequestType&0x80==0x80)之else
  {
   //根据bmRequestType的D6~5位散转，D6~5位表示请求的类型
   //0为标准请求，1为类请求，2为厂商请求。
   switch((bmRequestType>>5)&0x03)
   {
    case 0:  //标准请求
     #ifdef DEBUG0
      Prints("USB标准输出请求：");
     #endif
     //USB协议定义了几个标准输出请求，我们实现这些标准请求即可
     //请求的代码在bRequest中，对不同的请求代码进行散转
     switch(bRequest)
     {
      case CLEAR_FEATURE: //清除特性
       #ifdef DEBUG0
        Prints("清除特性。\r\n");
       #endif
      break;
      
      case SET_ADDRESS:  //设置地址
       #ifdef DEBUG0
        Prints("设置地址。地址为：");
        PrintHex(wValue&0xFF); //显示所设置的地址
        Prints("\r\n");
       #endif
       D12SetAddress(wValue&0xFF); //wValue中的低字节是设置的地址值
       //设置地址没有数据过程，直接进入到状态过程，返回一个0长度的数据包
       SendLength=0;
       NeedZeroPacket=1;
       //将数据通过EP0返回
       UsbEp0SendData();
      break;
      
      case SET_CONFIGURATION: //设置配置
       #ifdef DEBUG0
        Prints("设置配置。\r\n");
       #endif
       //使能非0端点。非0端点只有在设置为非0的配置后才能使能。
       //wValue的低字节为配置的值，如果该值为非0，才能使能非0端点。
       //保存当前配置值
       ConfigValue=wValue&0xFF;
       D12SetEndpointEnable(ConfigValue);
       //返回一个0长度的状态数据包
       SendLength=0;
       NeedZeroPacket=1;
       //将数据通过EP0返回
       UsbEp0SendData();
      break;
      
      case SET_DESCRIPTOR: //设置描述符
       #ifdef DEBUG0
        Prints("设置描述符。\r\n");
       #endif
      break;
      
      case SET_FEATURE: //设置特性
       #ifdef DEBUG0
        Prints("设置特性。\r\n");
       #endif
      break;

      case SET_INTERFACE: //设置接口
       #ifdef DEBUG0
        Prints("设置接口。\r\n");
       #endif
      break;      
      
      default:  //未定义的标准请求
       #ifdef DEBUG0
        Prints("错误：未定义的标准输出请求。\r\n");
       #endif       
      break;
     }
    break;
    
    case 1:  //类请求
     #ifdef DEBUG0
      Prints("USB类输出请求：");
     #endif
     switch(bRequest)
     {
      case SET_IDLE:
       #ifdef DEBUG0
        Prints("设置空闲。\r\n");
       #endif
       //只需要返回一个0长度的数据包即可
       SendLength=0;
       NeedZeroPacket=1;
       //将数据通过EP0返回
       UsbEp0SendData();
      break;
      
      case SET_REPORT:
       #ifdef DEBUG0
        Prints("设置报告。\r\n");
       #endif
       //该请求为设置报告，但是实际的数据并不在设置过程发出，
       //而是在之后的数据过程发出。这里不用做任何处理，在数据过程
       //完成后返回0长度的状态包。
      break;
      
      default:
       #ifdef DEBUG0
        Prints("未知请求。\r\n");
       #endif
      break;
     }
    break;
    
    case 2:  //厂商请求
     #ifdef DEBUG0
      Prints("USB厂商输出请求：\r\n");
     #endif
    break;
    
    default: //未定义的请求。这里只显示一个报错信息。
     #ifdef DEBUG0
      Prints("错误：未定义的输出请求。\r\n");
     #endif
    break;
   }
  }
 }
 //普通数据输出
 else //if(D12ReadEndpointLastStatus(0)&0x20)之else
 {
  UsbEp0DataOut();
 }
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：端点0输入中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbEp0In(void)
{
#ifdef DEBUG0
 Prints("USB端点0输入中断。\r\n");
#endif
 //读最后发送状态，这将清除端点0的中断标志位
 D12ReadEndpointLastStatus(1);
 //发送剩余的字节数
 UsbEp0SendData();
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：端点1输出中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbEp1Out(void)
{
#ifdef DEBUG0
 Prints("USB端点1输出中断。\r\n");
#endif
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：端点1输入中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbEp1In(void)
{
#ifdef DEBUG0
 Prints("USB端点1输入中断。\r\n");
#endif
 //读最后发送状态，这将清除端点1输入的中断标志位
 D12ReadEndpointLastStatus(3);
 //端点1输入处于空闲状态
 Ep1InIsBusy=0;
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：端点2输出中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbEp2Out(void)
{
#ifdef DEBUG0
 Prints("USB端点2输出中断。\r\n");
#endif
}
////////////////////////End of function//////////////////////////////

/********************************************************************
函数功能：端点2输入中断处理函数。
入口参数：无。
返    回：无。
备    注：无。
********************************************************************/
void UsbEp2In(void)
{
#ifdef DEBUG0
 Prints("USB端点2输入中断。\r\n");
#endif
}
////////////////////////End of function//////////////////////////////
