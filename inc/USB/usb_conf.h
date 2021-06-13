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
#define EP_NUM     (4)

/* MAX_PACKET_SIZE */
/* defines how many bytes per package */
#define MAX_PACKET_SIZE		(0x40)
#define EP0_TX_MAX_PACKET	MAX_PACKET_SIZE
#define EP0_RX_MAX_PACKET   MAX_PACKET_SIZE
#define EP1_TX_MAX_PACKET	(0x08)
#define EP1_RX_MAX_PACKET   (0x04)
#define EP2_TX_MAX_PACKET	(0x08)
#define EP2_RX_MAX_PACKET	(0x04)
#define EP3_TX_MAX_PACKET	(MAX_PACKET_SIZE)
#define EP3_RX_MAX_PACKET	(MAX_PACKET_SIZE)
/*-------------------------------------------------------------*/
/* --------------   Buffer Description Table  -----------------*/
/*-------------------------------------------------------------*/
/* buffer table base address */
#define BTABLE_ADDRESS      (0x00)

#define BUFFER_BASEADDR		(BTABLE_ADDRESS+0x08*EP_NUM)	  // 0x020
/* EP0  */
/* rx/tx buffer base address */
#define ENDP0_TXADDR        (BUFFER_BASEADDR)  				  //(0x020)
#define ENDP0_RXADDR        (ENDP0_TXADDR+EP0_TX_MAX_PACKET)  //(0x060)

/* EP1  */
/* rx/tx buffer base address */
#define ENDP1_TXADDR	    (ENDP0_RXADDR+EP0_RX_MAX_PACKET)	//(0x0A0)
#define ENDP1_RXADDR        (ENDP1_TXADDR+EP1_TX_MAX_PACKET)	//(0x0A8)

/* EP2 */
/* rx/tx buffer base address */
#define ENDP2_TXADDR		(ENDP1_RXADDR+EP1_RX_MAX_PACKET)	//(0x0AC)
#define ENDP2_RXADDR		(ENDP2_TXADDR+EP2_TX_MAX_PACKET)	//(0x0B4)

/* EP3 */
/* rx/tx buffer base address */
#define ENDP3_TXADDR		(ENDP2_RXADDR+EP2_RX_MAX_PACKET)	//(0x0B8)
#define ENDP3_RXADDR		(ENDP3_TXADDR+EP3_TX_MAX_PACKET) 	//(0x0F8)
																//(0x138)
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
/* #define  EP3_IN_Callback   NOP_Process*/
#define  EP4_IN_Callback   NOP_Process
#define  EP5_IN_Callback   NOP_Process
#define  EP6_IN_Callback   NOP_Process
#define  EP7_IN_Callback   NOP_Process

/* #define  EP1_OUT_Callback   NOP_Process */
/* #define  EP2_OUT_Callback   NOP_Process */
/* #define  EP3_OUT_Callback   NOP_Process */
#define  EP4_OUT_Callback   NOP_Process
#define  EP5_OUT_Callback   NOP_Process
#define  EP6_OUT_Callback   NOP_Process
#define  EP7_OUT_Callback   NOP_Process

#ifdef __cplusplus
}
#endif

#endif /*__USB_CONF_H*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
