/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
  ******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "usb.h"
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include <stdio.h>
#include <task/irqproxy.h>
#include <lib/utils.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern uint32_t keyboardStatus;
/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : 端点1输出处理流程(EP1 OUT Callback Routine.)
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void) {
    /* Set the transfer complete token to inform upper layer that the current
    transfer has been complete */
}
void EP1_OUT_Callback(void) {
    BaseType_t xReturn = pdPASS;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    USB_SIL_Read(EP1_OUT,(uint8_t*)&keyboardStatus);
    SetEPRxStatus(ENDP1, EP_RX_VALID);
    xReturn=xEventGroupSetBitsFromISR(hIRQEventGroup,IRQ_EVENT_MASK_KEYBOARD_UPDATE,&xHigherPriorityTaskWoken);
    if(xReturn == pdFAIL) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
void EP2_IN_Callback(void) {

}
void EP2_OUT_Callback(void) {
    SetEPRxStatus(ENDP2, EP_RX_VALID);
}
void EP3_IN_Callback(void){

}
void EP3_OUT_Callback(void){
	uint16_t receivedCount=GetEPRxCount(ENDP3);
	uint16_t part1Length = MIN(receivedCount,pUSBFIFO->uiSize-pUSBFIFO->uiIn);
	uint16_t part2Length = receivedCount-part1Length;

	PMAToUserBufferCopy(pUSBFIFO->pBuffer+pUSBFIFO->uiIn,ENDP3_RXADDR,part1Length);
	PMAToUserBufferCopy(pUSBFIFO->pBuffer,ENDP3_RXADDR+part1Length,part2Length);

	pUSBFIFO->uiIn = (pUSBFIFO->uiIn+receivedCount)%pUSBFIFO->uiSize;

	SetEPRxStatus(ENDP3, EP_RX_VALID);
	FIFO_Notify(pUSBFIFO);

}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

