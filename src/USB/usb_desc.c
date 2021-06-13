/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptors for Joystick Mouse Demo
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* 引入 ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"

/* 私有类型定义(typedef) -------------------------------------------------*/
/* 私有定义(define) ------------------------------------------------------*/
/* 私有宏 macro ----------------------------------------------------------*/
/* 私有变量 --------------------------------------------------------------*/
/* 继承的变量 ------------------------------------------------------------*/
/* 私有函数原型 ----------------------------------------------------------*/
/* 私有函数 --------------------------------------------------------------*/

/**USB 标准设备描述符(Device Descriptor)**/
const uint8_t JKBD_DeviceDescriptor_Data[JKBD_SIZ_DEVICE_DESC] = {
    //注意双字节的项目高位在后边
    JKBD_SIZ_DEVICE_DESC,       //bLength				设备描述符长度								JKBD_SIZ_DEVICE_DESC(18)字节
    USB_DEVICE_DESCRIPTOR_TYPE, //bDescriptorType		描述符类型									设备描述符
    0x00,0x02,                  //bcdUSB         		USB Spec版本								高速传输
    0x00,                       //bDeviceClass   		USB设备类号									使用配置描述符中定义的类型(0x00)
    0x00,                       //bDeviceSubClass		USB设备子类									使用配置描述符中定义的类型(0x00)
    0x00,                       //bDeviceProtocol		设备的协议									使用接口中定义的类型(0x00)
    MAX_PACKET_SIZE,            //bMaxPacketSize		端点最大包长度								64字节(8/16/32/64)
    0x34,0x12,                  //idVendor				供应商Id									0x1234(瞎写的,不存在;0x0483==>STMicroelectronics;查询:http://www.linux-usb.org/usb.ids)
    0x21,0x43,                  //idProduct				产品Id										0x4321(瞎写的,不存在;查询同上)
    0x01,0x01,                  //bcdDevice				BCD编码的产品版本号							1.00
    1,                          //iManufacturer			制造商描述字符串在字符串描述符列表的索引	1
    2,                          //iProduct				产品描述字符串在字符串描述符列表的索引		2
    3,                          //iSerialNumber			设备序列号在字符串描述符列表的索引			3
    0x01                        //bNumConfigurations	设备支持的配置数量=1个
};

/**USB 配置描述符(Configuration Descriptor)*/
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
/*   所有描述符(配置,接口,端点,类别,Vendor*/
const uint8_t JKBD_ConfigDescriptor_Data[JKBD_SIZ_CONFIG_DESC] = {
    //注意双字节的项目高位在后边
    /***************************** 配置描述符 *****************************/
    0x09,								// bLength				描述符长度										7字节
    USB_CONFIGURATION_DESCRIPTOR_TYPE,	// bDescriptorType		描述符类型										配置描述符
    JKBD_SIZ_CONFIG_DESC,0x00,          // wTotalLength			配置描述符总长度								配置描述符+接口描述符+端点描述符
    0x03,								// bNumInterfaces		配置支持的接口数量								2个
    0x01,								// bConfigurationValue	主机使用SetConfiguration时定位此配置的配置号	0x01
    0x00,								// iConfiguration		描述本配置的字符串描述符序列号					0x00
    0xC0,								/* bmAttributes			配置属性										1100 0000
    															Bit			value	Usage
    															D7			1		1:保留必须置1
    															D6			1		0:外供电模式
    																				1:自供电模式
    															D5			0		0:不支持远程唤醒
    																				1:支持远程唤醒
    															D4~D0		0		保留位00
										*/
    0x96,								// bMaxPower			最大电流										300mA(单位2mA,这里要足了供电,外设好驱动)
    /***************************** 接口描述符(0):标准键盘 *****************************/
    /* 09 */
    0x09,								// bLength				描述符长度			9字节
    USB_INTERFACE_DESCRIPTOR_TYPE,		// bDescriptorType		描述符类型			接口描述符
    0x00,								// bInterfaceNumber		接口编号			0x00
    0x00,								// bAlternateSetting	接口可替换设置号	0x00
    0x02,								// bNumEndpoints		使用的端点数量		2
    0x03,								// bInterfaceClass		接口实现的USB类		HID(Human Interface Device)
    0x01,								// bInterfaceSubClass	USB子类 			1=BOOT, 0=no boot
    0x01,								// nInterfaceProtocol	接口协议			无(0=none, 1=keyboard, 2=mouse)
    4,									// iInterface			描述该接口的字符串	第0个
    /** 接口0:HID描述符 **/
    /* 18 */
    0x09,								// bLength				描述符长度			9字节
    HID_DESCRIPTOR_TYPE,				// bDescriptorType		描述符类型			HID描述符
    0x10,0x01,							// bcdHID				HID协议版本			1.0
    0x00,								// bCountryCode			设备目标国家代号	0x00
    0x01,								// bNumDescriptors		下级描述符数量		1个
    0x22,								// bDescriptorType		下级描述符类型		HID报告描述符(0x22)
    JKBD_SIZ_STDKBD_REPORT_DESC,0x00,	// wItemLength			HID报告描述符长度	JKBD_SIZ_STDKBD_REPORT_DESC(60)字节
    /** 接口0:端点1:主机输入用于键盘按键采集(IN) **/
    /* 27 */
    0x07,								// bLength			描述符长度		7字节
    USB_ENDPOINT_DESCRIPTOR_TYPE,		// bDescriptorType	描述符类型		端点描述符
    0x81,								/* bEndpointAddress	端点地址		1 (IN)
															Bit		value	Usage
															D7		1		0:输出
																			1:输入
															D0~D3	1		端点使用的地址 */
    0x03,								// bmAttributes		端点属性		中断传输11b(00:控制 01:同步 10:批量 11:中断)
    EP1_TX_MAX_PACKET,0x00,				// wMaxPacketSize	最大传输包大小	8字节
    0x0A,								// bInterval			传输时间间隔为	32 ms(单位毫秒)
    /** 接口0:端点1:主机输出用于键盘状态输出(OUT) **/
    /* 34 */
    0x07,								// bLength			描述符长度		7字节
    USB_ENDPOINT_DESCRIPTOR_TYPE,		// bDescriptorType	描述符类型		端点描述符
    0x01,								// bEndpointAddress	端点地址		1 (OUT)
    0x03,								// bmAttributes		端点属性		中断传输
    EP1_RX_MAX_PACKET,0x00,				// wMaxPacketSize	最大传输包		8字节
    0x0A,								// bInterval		数据传输间隔	32ms(单位毫秒)


    /***************************** 接口描述符(1):扩展HID设备 *****************************/
    /* 41 */
    0x09,								// bLength				描述符长度			9字节
    USB_INTERFACE_DESCRIPTOR_TYPE,		// bDescriptorType		描述符类型			接口描述符
    0x01,								// bInterfaceNumber		接口编号			0x01
    0x00,								// bAlternateSetting	接口可替换设置号	0x00
    0x02,								// bNumEndpoints		使用的端点数量		2
    0x03,								// bInterfaceClass		接口实现的USB类		HID(Human Interface Device)
    0x01,								// bInterfaceSubClass	USB子类 			1=BOOT, 0=no boot
    0x01,								// nInterfaceProtocol	接口协议			键盘(0=none, 1=keyboard, 2=mouse)
    5,									// iInterface			描述该接口的字符串	第0个
    /** 接口1:HID描述符 **/
    /* 50 */
    0x09,								// bLength				描述符长度			9字节
    HID_DESCRIPTOR_TYPE,				// bDescriptorType		描述符类型			HID描述符
    0x10,0x01,							// bcdHID				HID协议版本			1.0
    0x00,								// bCountryCode			设备目标国家代号	0x00
    0x01,								// bNumDescriptors		下级描述符数量		1个
    0x22,								// bDescriptorType		下级描述符类型		HID报告描述符(0x22)
    JKBD_SIZ_EXTKBD_REPORT_DESC,0x00,	// wItemLength			HID报告描述符长度	JKBD_SIZ_EXTKBD_REPORT_DESC
    /** 接口1:端点2:主机输入用于键盘按键采集(IN) **/
    /* 59 */
    0x07,								// bLength			描述符长度		7字节
    USB_ENDPOINT_DESCRIPTOR_TYPE,		// bDescriptorType	描述符类型		端点描述符
    0x82,								/* bEndpointAddress	端点地址		2 (IN)
																			Bit		value	Usage
																			D7		1		0:输出
																							1:输入
																			D0~D3	1		端点使用的地址*/
    0x03,								// bmAttributes		端点属性		中断传输11b(00:控制 01:同步 10:批量 11:中断)
    EP2_TX_MAX_PACKET,0x00,				// wMaxPacketSize	最大传输包大小	8字节
    0x0A,								// bInterval			传输时间间隔为	32 ms(单位毫秒)
    /** 接口1:端点2:主机输出用于键盘状态输出(OUT) **/
    /* 66 */
    0x07,								// bLength			描述符长度		7字节
    USB_ENDPOINT_DESCRIPTOR_TYPE,		// bDescriptorType	描述符类型		端点描述符
    0x02,								// bEndpointAddress	端点地址		2 (OUT)
    0x03,								// bmAttributes		端点属性		中断传输
    EP2_RX_MAX_PACKET,0x00,				// wMaxPacketSize	端点最大传输包	8字节
    0x0A,								// bInterval		数据传输间隔	32ms(单位毫秒)


    /***************************** 接口描述符(2):上位机通信接口 *****************************/
    /* 73 */
    0x09,								// bLength				描述符长度			9字节
    USB_INTERFACE_DESCRIPTOR_TYPE,		// bDescriptorType		描述符类型			接口描述符
    0x02,								// bInterfaceNumber		接口编号			0x02
    0x00,								// bAlternateSetting	接口可替换设置号	0x00
    0x02,								// bNumEndpoints		使用的端点数量		2
    0xDC,								// bInterfaceClass		接口实现的USB类		Diagnostic Device(DCH)
    0xA0,								// bInterfaceSubClass	USB子类 			A0
    0xE0,								// nInterfaceProtocol	接口协议			E0
    6,									// iInterface			描述该接口的字符串	第0个
    /** 接口2:端点3:主机输入,下位机输出(IN) **/
    /* 82 */
    0x07,         						// bLength			描述符长度		7字节
    USB_ENDPOINT_DESCRIPTOR_TYPE, 		// bDescriptorType	描述符类型		端点描述符
    0x83,         						// bEndpointAddress	端点地址		3 (IN)
    0x02,								// bmAttributes		端点属性		Bulk
    EP3_TX_MAX_PACKET,0x00,    			// wMaxPacketSize	端点最大传输包	64字节(40H)
    0x00,         						// bInterval		数据传输间隔	Bulk下无效
    /** 接口2:端点3:主机输出,下位机输入(OUT) **/
	/* 89 */
	0x07,         						// bLength			描述符长度		7字节
    USB_ENDPOINT_DESCRIPTOR_TYPE, 		// bDescriptorType	描述符类型		端点描述符
    0x03,         						// bEndpointAddress	端点地址		3 (OUT)
    0x02,								// bmAttributes		端点属性		Bulk
    EP3_RX_MAX_PACKET,0x00,    			// wMaxPacketSize	端点最大传输包	64字节(40H)
    0x00,         						// bInterval		数据传输间隔	Bulk下无效

	/* 96 */
};
/* 标准键盘 HID 报告描述符 */
const uint8_t JKBD_StdKbdReportDescriptor_Data[JKBD_SIZ_STDKBD_REPORT_DESC] = {
    0x05, 0x01,         // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,         // USAGE (Keyboard)
    0xa1, 0x01,         // COLLECTION (Application)
    0x05, 0x07,         //     USAGE_PAGE (Keyboard/Keypad)
    /**    {BYTE0:八个控制键}        */
    0x19, 0xe0,         //     USAGE_MINIMUM (Keyboard LeftControl) [最小值为左Control]
    0x29, 0xe7,         //     USAGE_MAXIMUM (Keyboard Right GUI)   [最大为右GUI,即WinKey键]
    0x15, 0x00,         //     LOGICAL_MINIMUM (0)                  [逻辑最小为0==>不按下]
    0x25, 0x01,         //     LOGICAL_MAXIMUM (1)                  [逻辑最大为1==>按下]
    0x95, 0x08,         //     REPORT_COUNT (8)                     [控制键:8个报告-键盘输出8个控制键的信息]
    0x75, 0x01,         //     REPORT_SIZE (1)                      [控制键:1个报告1bit==>一个控制键一个bit,共计一个字节]
    0x81, 0x02,         //     INPUT (Data,Var,Abs)                 [按键为输入(相对于主机),数据,变量,绝对值]
    /**    {BYTE1:保留一个字节}      */
    0x95, 0x01,         //     REPORT_COUNT (1)                     [保留字节:一个报告]
    0x75, 0x08,         //     REPORT_SIZE (8)                      [保留字节:每个报告8Bit]
    0x81, 0x03,         //     INPUT (Cnst,Var,Abs)                 [保留字节,常量,变量,绝对值]
    /**    {BYTE2~8:其他按键输出}    */
    0x95, 0x06,         //     REPORT_COUNT (6)                     [按键:同时允许6个按键同时按下,6个报告]
    0x75, 0x08,         //     REPORT_SIZE (8)                      [按键:每个按键描述符由一个字节组成]
    0x15, 0x00,         //     LOGICAL_MINIMUM (0) [按键描述符最小为1]
    0x26, 0xff, 0x00,   //     LOGICAL_MAXIMUM (255)[按键描述符最大为255]
    0x19, 0x00,         //     USAGE_MINIMUM (Reserved (no event indicated))    [最小值为保留按键]
    0x29, 0x65,         //     USAGE_MAXIMUM (Keyboard Application)             [最大值为键盘应用]
    0x81, 0x00,         //     INPUT (Data,Ary,Abs)                 [按键为输入(相对于主机),传输数据,数组,绝对值]
    /**    {BYTE3:状态灯}            */
    0x05, 0x08,         //     USAGE_PAGE (LEDs)                    [用于灯]
    0x95, 0x08,         //     REPORT_COUNT (8)                     [2个灯]
    0x75, 0x01,         //     REPORT_SIZE (1)                      [每个灯用1bit表示]
    0x15, 0x00,         //     LOGICAL_MINIMUM (0)                  [逻辑最小0]
    0x25, 0x01,         //     LOGICAL_MAXIMUM (1)                  [逻辑最大1]
    0x19, 0x01,         //     USAGE_MINIMUM (Num Lock)             [最小位是NumLock灯]
    0x29, 0x08,         //     USAGE_MAXIMUM (Do Not Distrurb)      [最大位是CapsLock灯]
    0x91, 0x02,         //     OUTPUT (Data,Var,Abs)                [LED位输出(相对于主机),传输数据,变量,绝对值]
    0xc0                // END_COLLECTION
};
/** 扩展键盘报告描述符 **/
const uint8_t JKBD_ExtKbdReportDescriptor_Data[JKBD_SIZ_EXTKBD_REPORT_DESC]= {
    0x05, 0x0c,                    // USAGE_PAGE (Consumer Devices)
    0x09, 0x01,                    // USAGE (Consumer Control)
    0xa1, 0x01,                    // COLLECTION (Application)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0xe9,                    //     USAGE (Volume Up)     bit0    01
    0x09, 0xea,                    //     USAGE (Volume Down)   bit1    02
    0x09, 0xe2,                    //     USAGE (Mute)          bit2    04
    0x09, 0xcd,                    //     USAGE (Play/Pause)    bit3    08
    0x09, 0x6f,                    //     USAGE (Bright Up)     bit4    10
    0x09, 0x70,                    //     USAGE (Bright Down)   bit5    20
    0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
    0x45, 0x07,                    //     PHYSICAL_MAXIMUM (7)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x06,                    //     REPORT_COUNT (6)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x01,                    //     INPUT (Cnst,Ary,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};
/**USB字符串描述符(可选)**/
/* 设备语言*/
const uint8_t JKBD_StringLangID_Data[JKBD_SIZ_STRING_LANGID] = {
    JKBD_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,0x04      /* LangID = 0x0409: U.S. English */
};

/* 设备制造商 */
const uint8_t JKBD_StringVendor_Data[JKBD_SIZ_STRING_VENDOR] = {
    JKBD_SIZ_STRING_VENDOR, /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
    /* Manufacturer: "STMicroelectronics" */
    'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
    'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
    'c', 0, 's', 0
};
/* 产品名称 */
const uint8_t JKBD_StringProduct_Data[JKBD_SIZ_STRING_PRODUCT] = {
    JKBD_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
	0x00,0x4E,	/* "一" unicode */
	0x2A,0x4E,	/* "个" unicode */
	0x0D,0x4E,	/* "不" unicode */
	0x3F,0x61,	/* "愿" unicode */
	0x0F,0x90,	/* "透" unicode */
	0x32,0x97,	/* "露" unicode */
	0xD3,0x59,	/* "姓" unicode */
	0x0D,0x54,	/* "名" unicode */
	0x84,0x76,	/* "的" unicode */
	0x2E,0x95,	/* "键" unicode */
	0xD8,0x76,	/* "盘" unicode */
};
/* 产品序列号 */
const uint8_t JKBD_StringSerial_Data[JKBD_SIZ_STRING_SERIAL] = {
    JKBD_SIZ_STRING_SERIAL,             /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,             /* bDescriptorType */
    'S', 0, 'T', 0, 'M', 0, '3', 0, '2', 0
};
/* 标准键盘接口描述 */
const uint8_t JKBD_StringStdKbdInterfaceDesc_Data[JKBD_SIZ_STRING_STDKBD_INTERFACE_DESC] = {
	JKBD_SIZ_STRING_STDKBD_INTERFACE_DESC,	/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,				/* bDescriptorType */
	0x07,0x68,	/* "标" unicode */
	0xC6,0x51,	/* "准" unicode */
	0x2E,0x95,	/* "键" unicode */
	0xD8,0x76,	/* "盘" unicode */
};
/* 扩展HID接口描述 */
const uint8_t JKBD_StringExtKbdInterfaceDesc_Data[JKBD_SIZ_STRING_EXTKBD_INTERFACE_DESC] = {
	JKBD_SIZ_STRING_EXTKBD_INTERFACE_DESC,	/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,				/* bDescriptorType */
	0x69,0x62,	/* "扩" unicode */
	0x55,0x5C,	/* "展" unicode */
	0xBA,0x4E,	/* "人" unicode */
	0x53,0x4F,	/* "体" unicode */
	0x93,0x8F,	/* "输" unicode */
	0x65,0x51,	/* "入" unicode */
	0x66,0x5B,	/* "学" unicode */
	0xBE,0x8B,	/* "设" unicode */
	0x07,0x59,	/* "备" unicode */
};
/* 数据通信接口描述 */
const uint8_t JKBD_StringIOPortInterfaceDesc_Data[JKBD_SIZ_STRING_IOPORT_INTERFACE_DESC] = {
	JKBD_SIZ_STRING_IOPORT_INTERFACE_DESC,	/* bLength */
	USB_STRING_DESCRIPTOR_TYPE,				/* bDescriptorType */
	0x70,0x65,	/* "数" unicode */
	0x6E,0x63,	/* "据" unicode */
	0x1A,0x90,	/* "通" unicode */
	0xE1,0x4F,	/* "信" unicode */
	0xA5,0x63,	/* "接" unicode */
	0xE3,0x53,	/* "口" unicode */
};
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

