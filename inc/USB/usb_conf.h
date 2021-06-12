/**
  ******************************************************************************
  * @file    usb_conf.h
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Joystick Mouse demo configuration file
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
#ifndef __USB_CONF_H
#define __USB_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* External variables --------------------------------------------------------*/
/*-------------------------------------------------------------*/
/* EP_NUM */
/* defines how many endpoints are used by the device */
/*-------------------------------------------------------------*/
#define EP_NUM     (3)

/* MAX_PACKET_SIZE */
/* defines how many bytes per package */
#define MAX_PACKET_SIZE		(0x40)

/*-------------------------------------------------------------*/
/* --------------   Buffer Description Table  -----------------*/
/*-------------------------------------------------------------*/
/* buffer table base address */
/* buffer table base address */
#define BTABLE_ADDRESS      (0x00)

#define BUFFER_BASEADDR		(BTABLE_ADDRESS+0x08*EP_NUM)
/* EP0  */
/* rx/tx buffer base address */
#define ENDP0_RXADDR        (BUFFER_BASEADDR+MAX_PACKET_SIZE*0)	//(0x18)
#define ENDP0_TXADDR        (BUFFER_BASEADDR+MAX_PACKET_SIZE*1)  //(0x58)

/* EP1  */
/* rx/tx buffer base address */
#define ENDP1_RXADDR        (BUFFER_BASEADDR+MAX_PACKET_SIZE*2)	//(0x98)
#define ENDP1_TXADDR	    (BUFFER_BASEADDR+MAX_PACKET_SIZE*3)	//(0xD8)

/* EP2 */
/* rx/tx buffer base address */
#define ENDP2_RXADDR		(BUFFER_BASEADDR+MAX_PACKET_SIZE*4)	//(0x118)
#define ENDP2_TXADDR		(BUFFER_BASEADDR+MAX_PACKET_SIZE*5)	//(0x158)

/* EP3 */
/* rx/tx buffer base address */
#define ENDP3_TXADDR		(BUFFER_BASEADDR+MAX_PACKET_SIZE*6)
#define ENDP3_RXADDR		(BUFFER_BASEADDR+MAX_PACKET_SIZE*7)

/*-------------------------------------------------------------*/
/* -------------------   ISTR events  -------------------------*/
/*-------------------------------------------------------------*/
/* IMR_MSK */
/* mask defining which events has to be handled */
/* by the device application software */
#define IMR_MSK (CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM \
                 | CNTR_ESOFM | CNTR_RESETM )

/* CTR service routines */
/* associated to defined endpoints */
/* #define  EP1_IN_Callback   NOP_Process*/
/* #define  EP2_IN_Callback   NOP_Process*/
#define  EP3_IN_Callback   NOP_Process
#define  EP4_IN_Callback   NOP_Process
#define  EP5_IN_Callback   NOP_Process
#define  EP6_IN_Callback   NOP_Process
#define  EP7_IN_Callback   NOP_Process

/* #define  EP1_OUT_Callback   NOP_Process */
/* #define  EP2_OUT_Callback   NOP_Process */
#define  EP3_OUT_Callback   NOP_Process
#define  EP4_OUT_Callback   NOP_Process
#define  EP5_OUT_Callback   NOP_Process
#define  EP6_OUT_Callback   NOP_Process
#define  EP7_OUT_Callback   NOP_Process

#ifdef __cplusplus
}
#endif

#endif /*__USB_CONF_H*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
