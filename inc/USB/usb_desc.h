/**
  ******************************************************************************
  * @file    usb_desc.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Descriptor Header for JKBD Mouse Demo
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define JKBD_SIZ_STDKBD_HID_DESC                0x09
#define JKBD_OFF_STDKBD_HID_DESC                0x12
#define JKBD_SIZ_EXTKBD_HID_DESC                0x09
#define JKBD_OFF_EXTKBD_HID_DESC                0x28

#define JKBD_SIZ_DEVICE_DESC                18
#define JKBD_SIZ_CONFIG_DESC                73
#define JKBD_SIZ_STDKBD_REPORT_DESC         60
#define JKBD_SIZ_EXTKBD_REPORT_DESC			42
#define JKBD_SIZ_STRING_LANGID              4
#define JKBD_SIZ_STRING_VENDOR              38
#define JKBD_SIZ_STRING_PRODUCT             8
#define JKBD_SIZ_STRING_SERIAL              26

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

/* Exported functions ------------------------------------------------------- */
extern const uint8_t JKBD_DeviceDescriptor[JKBD_SIZ_DEVICE_DESC];
extern const uint8_t JKBD_ConfigDescriptor[JKBD_SIZ_CONFIG_DESC];
extern const uint8_t JKBD_StdKbdReportDescriptor[JKBD_SIZ_STDKBD_REPORT_DESC];
extern const uint8_t JKBD_ExtKbdReportDescriptor[JKBD_SIZ_EXTKBD_REPORT_DESC];
extern const uint8_t JKBD_StringLangID[JKBD_SIZ_STRING_LANGID];
extern const uint8_t JKBD_StringVendor[JKBD_SIZ_STRING_VENDOR];
extern const uint8_t JKBD_StringProduct[JKBD_SIZ_STRING_PRODUCT];
extern const uint8_t JKBD_StringSerial[JKBD_SIZ_STRING_SERIAL];

#ifdef __cplusplus
}
#endif

#endif /* __USB_DESC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
