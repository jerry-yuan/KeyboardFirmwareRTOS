/**
  ******************************************************************************
  * @file    usb_prop.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   All processing related to Joystick Mouse Demo
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


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "hw_config.h"

/* 私有类型声明(Private typedef) ---------------------------------------------*/
/* 私有声明(Private define) --------------------------------------------------*/
/* 私有宏(Private macro) -----------------------------------------------------*/
/* 私有变量(Private variables) -----------------------------------------------*/
uint32_t ProtocolValue; //协议值

/* -------------------------------------------------------------------------- */
/*  结构体初始化(Structures initializations)                                  */
/* -------------------------------------------------------------------------- */
// 设备表
DEVICE Device_Table = {
    EP_NUM,     // 端点数量:2
    1           // 1个配置描述符
};
// 设备属性
DEVICE_PROP Device_Property = {
    JKBD_init,                      // 初始化回调
    JKBD_Reset,                     // 复位回调
    JKBD_Status_In,                 // Status In
    JKBD_Status_Out,                // Status Out
    JKBD_Data_Setup,                // DataSetup
    JKBD_NoData_Setup,              //
    JKBD_Get_Interface_Setting,     // 获取接口设定
    JKBD_GetDeviceDescriptor,       // 获得设备描述符
    JKBD_GetConfigDescriptor,       // 获得配置描述符
    JKBD_GetStringDescriptor,       // 获取字符串描述符
    0,                                  // V4.0USB库不再使用,用于向前兼容
    0x40                                // 最大包大小
};
// 用户标准请求
USER_STANDARD_REQUESTS User_Standard_Requests = {
    JKBD_GetConfiguration,          // 取得配置请求处理函数
    JKBD_SetConfiguration,          // 设定配置请求处理函数
    JKBD_GetInterface,              // 获得接口请求处理函数
    JKBD_SetInterface,              // 设定接口请求处理函数
    JKBD_GetStatus,                 // 取得状态请求处理函数
    JKBD_ClearFeature,              // 清除Feature请求处理函数
    JKBD_SetEndPointFeature,        // 设定端点特性请求处理函数
    JKBD_SetDeviceFeature,          // 设定设备特性
    JKBD_SetDeviceAddress           // 设定设备地址
};
// 设备描述符
ONE_DESCRIPTOR Device_Descriptor = {
    (uint8_t*)JKBD_DeviceDescriptor,
    JKBD_SIZ_DEVICE_DESC
};
// 配置描述符
ONE_DESCRIPTOR Config_Descriptor = {
    (uint8_t*)JKBD_ConfigDescriptor,
    JKBD_SIZ_CONFIG_DESC
};
// 标准键盘HID报告描述符
ONE_DESCRIPTOR JKBD_Std_Kbd_Report_Descriptor = {
    (uint8_t *)JKBD_StdKbdReportDescriptor,
    JKBD_SIZ_STDKBD_REPORT_DESC
};
// 扩展键盘HID报告描述符
ONE_DESCRIPTOR JKBD_Ext_Kbd_Report_Descriptor = {
    (uint8_t *)JKBD_ExtKbdReportDescriptor,
    JKBD_SIZ_EXTKBD_REPORT_DESC
};
// 标准键盘HID描述符
ONE_DESCRIPTOR JKBD_Std_Kbd_Hid_Descriptor = {
    (uint8_t*)JKBD_ConfigDescriptor + JKBD_OFF_STDKBD_HID_DESC,
    JKBD_SIZ_STDKBD_HID_DESC
};
// 扩展键盘HID描述符
ONE_DESCRIPTOR JKBD_Ext_Kbd_Hid_Descriptor = {
    (uint8_t*)JKBD_ConfigDescriptor + JKBD_OFF_EXTKBD_HID_DESC,
    JKBD_SIZ_EXTKBD_HID_DESC
};
// 字符串描述符
ONE_DESCRIPTOR String_Descriptor[4] = {
    {(uint8_t*)JKBD_StringLangID, JKBD_SIZ_STRING_LANGID},
    {(uint8_t*)JKBD_StringVendor, JKBD_SIZ_STRING_VENDOR},
    {(uint8_t*)JKBD_StringProduct, JKBD_SIZ_STRING_PRODUCT},
    {(uint8_t*)JKBD_StringSerial, JKBD_SIZ_STRING_SERIAL}
};

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : JKBD_init.
* Description    : 初始化流程(Joystick Mouse init routine.)
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void JKBD_init(void) {

    /* Update the serial number string descriptor with the data from the unique ID*/
    //Get_SerialNum();

    pInformation->Current_Configuration = 0;
    /* Connect the device */
    PowerOn();

    /* Perform basic device initialization operations */
    USB_SIL_Init();

    bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : JKBD_Reset.
* Description    : Joystick Mouse reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void JKBD_Reset(void) {
    /* Set JKBD_DEVICE as not configured */
    pInformation->Current_Configuration = 0;
    pInformation->Current_Interface = 0;/*the default Interface*/

    /* Current Feature initialization */
    pInformation->Current_Feature = JKBD_ConfigDescriptor[7];
    SetBTABLE(BTABLE_ADDRESS);
    /* Initialize Endpoint 0 */
    SetEPType(ENDP0, EP_CONTROL);
    SetEPTxStatus(ENDP0, EP_TX_STALL);
    SetEPRxAddr(ENDP0, ENDP0_RXADDR);
    SetEPTxAddr(ENDP0, ENDP0_TXADDR);
    Clear_Status_Out(ENDP0);
    SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
    SetEPRxValid(ENDP0);

    /* Initialize Endpoint 1 */
    SetEPType(ENDP1, EP_INTERRUPT);
    /* Initialize Endpoint IN 1 */
    SetEPTxAddr(ENDP1,ENDP1_TXADDR);
    SetEPTxCount(ENDP1, 8);
    SetEPTxStatus(ENDP1, EP_TX_NAK);

    /* Initialize Endpoint OUT 1 */
    SetEPRxAddr(ENDP1,ENDP1_RXADDR);
    SetEPRxCount(ENDP1, 1);
    SetEPRxStatus(ENDP1,EP_RX_VALID);

    /* Initialize Endpoint 2 */
    SetEPType(ENDP2, EP_INTERRUPT);

    /* Initialize Endpoint IN 2 */
    SetEPTxAddr(ENDP2,ENDP2_TXADDR);
    SetEPTxCount(ENDP2, 1);
    SetEPTxStatus(ENDP2, EP_TX_NAK);

    /* Initialize Endpoint OUT 2*/
    SetEPRxAddr(ENDP2,ENDP2_RXADDR);
    SetEPRxCount(ENDP2, 1);
    SetEPRxStatus(ENDP2,EP_RX_VALID);

    /* Set this device to response on default address */
    SetDeviceAddress(0);
    bDeviceState = ATTACHED;
}
/*******************************************************************************
* Function Name  : JKBD_SetConfiguration.
* Description    : Update the device state to configured.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void JKBD_SetConfiguration(void) {
    DEVICE_INFO *pInfo = &Device_Info;

    if (pInfo->Current_Configuration != 0) {
        /* Device configured */
        bDeviceState = CONFIGURED;
    }
}
/*******************************************************************************
* Function Name  : JKBD_SetConfiguration.
* Description    : Update the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void JKBD_SetDeviceAddress (void) {
    bDeviceState = ADDRESSED;
}

/*******************************************************************************
* Function Name  : JKBD_Data_Setup
* Description    : Handle the data class specific requests.
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT JKBD_Data_Setup(uint8_t RequestNo) {
    uint8_t *(*CopyRoutine)(uint16_t);
    /* Type_Recipient=(pInformation->USBbmRequestType & 	(REQUEST_TYPE | RECIPIENT))
    	 					D7:		数据方向	0:主机器->设备  	 0x60	      0x1F
    											1:从->主	  	  0110 0000		0001 1111
    						D6~D5:	请求类型	0:标准请求(STANDARD_REQUEST)
    											1:类请求(CLASS_REQUEST)
    											2:制造商(VENDOR_REQUEST)
    						D4~D0:	接收者		0:设备(DEVICE_RECIPIENT)
    											1:接口(INTERFACE_RECIPIENT)
    											2:端点(ENDPOINT_RECIPIENT)
    0000 0000*/
    CopyRoutine = NULL;

    if(RequestNo == GET_DESCRIPTOR) {
        // 主机希望GET_DESCRIPTOR
        if(Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT)) {
            // 标准请求 => 接口
            if(pInformation->USBwValue1 == REPORT_DESCRIPTOR) {
                // 主机 要 报告描述符
                switch(pInformation->USBwIndex0) {
                case 0:
                    CopyRoutine=JKBD_GetStdKbdReportDescriptor;
                    break;
                case 1:
                    CopyRoutine=JKBD_GetExtKbdReportDescriptor;
                    break;
                default:
                    break;
                }
            } else if(pInformation->USBwValue1==HID_DESCRIPTOR_TYPE) {
                // 主机 要 HID描述符
                switch(pInformation->USBwIndex0) {
                case 0:
                    CopyRoutine=JKBD_GetStdKbdHIDDescriptor;
                    break;
                case 1:
                    CopyRoutine=JKBD_GetExtKbdHIDDescriptor;
                    break;
                default:
                    break;
                }
            }
        }
    } else if(RequestNo==GET_PROTOCOL) {
        // 系统GET_PROTOCOL
        if(Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)) {
            CopyRoutine = JKBD_GetProtocolValue;
        }
    }







    if (CopyRoutine == NULL) {
        return USB_UNSUPPORT;
    }
    pInformation->Ctrl_Info.CopyData = CopyRoutine;
    pInformation->Ctrl_Info.Usb_wOffset = 0;
    (*CopyRoutine)(0);
    return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : JKBD_NoData_Setup
* Description    : handle the no data class specific requests
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT JKBD_NoData_Setup(uint8_t RequestNo) {
    if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
            && (RequestNo == SET_PROTOCOL)) {
        return JKBD_SetProtocol();
    }

    else {
        return USB_UNSUPPORT;
    }
}

/*******************************************************************************
* Function Name  : JKBD_GetDeviceDescriptor.
* Description    : Gets the device descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the device descriptor.
*******************************************************************************/
uint8_t *JKBD_GetDeviceDescriptor(uint16_t Length) {
    return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
* Function Name  : JKBD_GetConfigDescriptor.
* Description    : Gets the configuration descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *JKBD_GetConfigDescriptor(uint16_t Length) {
    return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
* Function Name  : JKBD_GetStringDescriptor
* Description    : 获取字符串描述符
* Input          : Length
* Output         : None.
* Return         : 字符串描述符地址
*******************************************************************************/
uint8_t *JKBD_GetStringDescriptor(uint16_t Length) {
    uint8_t wValue0 = pInformation->USBwValue0;
    if (wValue0 > sizeof(String_Descriptor)/sizeof(ONE_DESCRIPTOR)) {
        return NULL;
    } else {
        return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
    }
}

/*******************************************************************************
* Function Name  : JKBD_GetStdKbdReportDescriptor.
* Description    : 获取标准键盘的报告描述符.
* Input          : 长度
* Output         : None.
* Return         : 配置描述符的地址.
*******************************************************************************/
uint8_t* JKBD_GetStdKbdReportDescriptor(uint16_t Length) {
    return Standard_GetDescriptorData(Length, &JKBD_Std_Kbd_Report_Descriptor);
}

/*******************************************************************************
* Function Name  : JKBD_GetExtKbdReportDescriptor.
* Description    : 获取扩展键盘的报告描述符.
* Input          : 长度
* Output         : None.
* Return         : 配置描述符的地址.
*******************************************************************************/
uint8_t* JKBD_GetExtKbdReportDescriptor(uint16_t Length) {
    return Standard_GetDescriptorData(Length, &JKBD_Ext_Kbd_Report_Descriptor);
}

/*******************************************************************************
* Function Name  : JKBD_GetHIDDescriptor.
* Description    : Gets the HID descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
uint8_t *JKBD_GetStdKbdHIDDescriptor(uint16_t Length) {
    return Standard_GetDescriptorData(Length, &JKBD_Std_Kbd_Hid_Descriptor);
}
uint8_t *JKBD_GetExtKbdHIDDescriptor(uint16_t Length) {
    return Standard_GetDescriptorData(Length, &JKBD_Ext_Kbd_Hid_Descriptor);
}

/*******************************************************************************
* Function Name  : JKBD_Get_Interface_Setting.
* Description    : tests the interface and the alternate setting according to the
*                  supported one.
* Input          : - Interface : interface number.
*                  - AlternateSetting : Alternate Setting number.
* Output         : None.
* Return         : USB_SUCCESS or USB_UNSUPPORT.
*******************************************************************************/
RESULT JKBD_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting) {
    if (AlternateSetting > 0) {
        return USB_UNSUPPORT;
    } else if (Interface > 0) {
        return USB_UNSUPPORT;
    }
    return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : JKBD_SetProtocol
* Description    : Joystick Set Protocol request routine.
* Input          : None.
* Output         : None.
* Return         : USB SUCCESS.
*******************************************************************************/
RESULT JKBD_SetProtocol(void) {
    uint8_t wValue0 = pInformation->USBwValue0;
    ProtocolValue = wValue0;
    return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : JKBD_GetProtocolValue
* Description    : get the protocol value
* Input          : Length.
* Output         : None.
* Return         : address of the protocol value.
*******************************************************************************/
uint8_t *JKBD_GetProtocolValue(uint16_t Length) {
    if (Length == 0) {
        pInformation->Ctrl_Info.Usb_wLength = 1;
        return NULL;
    } else {
        return (uint8_t *)(&ProtocolValue);
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
