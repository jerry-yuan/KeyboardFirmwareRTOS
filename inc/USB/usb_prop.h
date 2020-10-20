/**
  ******************************************************************************
  * @file    usb_prop.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   All processing related to Joystick Mouse demo
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
#ifndef __USB_PROP_H
#define __USB_PROP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum _HID_REQUESTS
{
  GET_REPORT = 1,
  GET_IDLE,
  GET_PROTOCOL,

  SET_REPORT = 9,
  SET_IDLE,
  SET_PROTOCOL
} HID_REQUESTS;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void JKBD_init(void);
void JKBD_Reset(void);
void JKBD_SetConfiguration(void);
void JKBD_SetDeviceAddress (void);
RESULT JKBD_Data_Setup(uint8_t);
RESULT JKBD_NoData_Setup(uint8_t);
RESULT JKBD_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *JKBD_GetDeviceDescriptor(uint16_t );
uint8_t *JKBD_GetConfigDescriptor(uint16_t);
uint8_t *JKBD_GetStringDescriptor(uint16_t);
RESULT JKBD_SetProtocol(void);
uint8_t *JKBD_GetProtocolValue(uint16_t Length);
RESULT JKBD_SetProtocol(void);
uint8_t *JKBD_GetStdKbdReportDescriptor(uint16_t Length);
uint8_t *JKBD_GetExtKbdReportDescriptor(uint16_t Length);
uint8_t *JKBD_GetStdKbdHIDDescriptor(uint16_t Length);
uint8_t *JKBD_GetExtKbdHIDDescriptor(uint16_t Length);

/* Exported define -----------------------------------------------------------*/
#define JKBD_GetConfiguration          	NOP_Process
//#define JKBD_SetConfiguration          NOP_Process
#define JKBD_GetInterface              	NOP_Process
#define JKBD_SetInterface              	NOP_Process
#define JKBD_GetStatus                 	NOP_Process
#define JKBD_ClearFeature              	NOP_Process
#define JKBD_SetEndPointFeature       	NOP_Process
#define JKBD_SetDeviceFeature          	NOP_Process
//#define JKBD_SetDeviceAddress          NOP_Process
#define JKBD_Status_In 					NOP_Process
#define JKBD_Status_Out 				NOP_Process

#define REPORT_DESCRIPTOR                  0x22

#ifdef __cplusplus
}
#endif

#endif /* __USB_PROP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
