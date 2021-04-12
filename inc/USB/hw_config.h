/**
  ******************************************************************************
  * @file    hw_config.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Hardware Configuration & Setup
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
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "usb_type.h"

#ifdef __cplusplus
extern "C" {
#endif

    /* Exported types ------------------------------------------------------------*/
    /* Exported constants --------------------------------------------------------*/
    /* Exported macro ------------------------------------------------------------*/
    /* Exported define -----------------------------------------------------------*/
    /* Exported functions ------------------------------------------------------- */
    void Set_System(void);
    void Set_USBClock(void);
    void Enter_LowPowerMode(void);
    void Leave_LowPowerMode(void);
    void USB_Interrupts_Config(void);
    void USB_Cable_Config (FunctionalState NewState);

    void USB_Port_Set(uint8_t enable);
    void JKBD_StdKbdSend();
    void JKBD_ExtKbdSend();
    void JKBD_Send(uint8_t* buf,uint8_t len,uint8_t endpoint);


    uint8_t JoyState(void);
    void Get_SerialNum(void);

#ifdef __cplusplus
}
#endif

#endif  /*__HW_CONFIG_H*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
