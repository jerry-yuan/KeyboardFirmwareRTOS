/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Endpoint routines
  ******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include <stdio.h>
#include <task/keyboard.h>
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
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
    xReturn=xEventGroupSetBitsFromISR(hKeyboardStateUpdateEvent,KEY_STATE_EVENT_UPDATE,&xHigherPriorityTaskWoken);
    if(xReturn == pdFAIL) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
void EP2_IN_Callback(void) {

}
void EP2_OUT_Callback(void) {
    SetEPRxStatus(ENDP2, EP_RX_VALID);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

