/**
  ******************************************************************************
  * @file    hw_config.c
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


/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include <task/irqproxy.h>
#include <screen/usbscr.h>
#include <stdio.h>
/* Private function prototypes -----------------------------------------------*/

//USB唤醒中断服务函数
void USBWakeUp_IRQHandler(void) {
    EXTI_ClearITPendingBit(EXTI_Line18);//清除USB唤醒中断挂起位
}

//USB中断处理函数
void USB_LP_CAN1_RX0_IRQHandler(void) {
    USB_Istr();
}

/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : 设置USB时钟输入为48MHz(Configures USB Clock input (48MHz).)
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
//USB时钟配置函数,USBclk=48Mhz@HCLK=72Mhz
void Set_USBClock(void) {
#ifdef STM32F10X_HD
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);	//USBclk=PLLclk/1.5=48Mhz
#endif
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);	 //USB时钟使能
}


/*******************************************************************************
* Function Name  : Enter_LowPowerMode.
* Description    : 进入待机模式时关闭系统时钟和电源(Power-off system clocks and power while entering suspend mode.)
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void) {
	BaseType_t xResult;
    BaseType_t xHigherPriorityTaskWoken;
    bDeviceState = SUSPENDED;
    xResult=xEventGroupSetBitsFromISR(hIRQEventGroup,IRQ_EVENT_MASK_USB_STATE_UPDATE,&xHigherPriorityTaskWoken);
    if(xResult == pdFAIL){
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    //printf("usb enter low power mode\r\n");
    //bDeviceState记录USB连接状态，在usb_pwr.c里面定义
}


/*******************************************************************************
* Function Name  : Leave_LowPowerMode.
* Description    : 退出待机模式时恢复系统时钟和电源(Restores system clocks and power while exiting suspend mode.)
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void) {
	BaseType_t	xResult;
    BaseType_t  xHigherPriorityTaskWoken;
    DEVICE_INFO *pInfo=&Device_Info;
    //printf("leave low power mode\r\n");
    if (pInfo->Current_Configuration!=0) {
        bDeviceState = CONFIGURED;
    } else {
        bDeviceState = ATTACHED;
    }

    xResult=xEventGroupSetBitsFromISR(hIRQEventGroup,IRQ_EVENT_MASK_USB_STATE_UPDATE,&xHigherPriorityTaskWoken);
    if(xResult == pdFAIL){
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


/*******************************************************************************
* Function Name  : USB_Interrupts_Config.
* Description    : 配置USB中断(Configures the USB interrupts.)
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_Interrupts_Config(void) {
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    /* Configure the EXTI line 18 connected internally to the USB IP */
    EXTI_ClearITPendingBit(EXTI_Line18);		//  开启线18上的中断
    EXTI_InitStructure.EXTI_Line = EXTI_Line18; 	// USB resume from suspend mode
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;	//line 18上事件上升降沿触发
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable the USB interrupt */
#ifdef STM32F10X_HD
    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;//组2，优先级次之
#endif
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the USB Wake-up interrupt */
#ifdef STM32F10X_HD
    NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;   //组2，优先级最高
#endif
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
}


/*******************************************************************************
* Function Name  : USB_Cable_Config.
* Description    : 软件实现USB线的连接与断开(Software Connection/Disconnection of USB Cable.)
* Input          : NewState: new state.
* Output         : None.
* Return         : None
*******************************************************************************/
//USB接口配置(配置1.5K上拉电阻,ALIENTEK的M3系列开发板，不需要配置,固定加了上拉电阻)
//NewState:DISABLE,不上拉，ENABLE,上拉
void USB_Cable_Config (FunctionalState NewState) {
    /*if (NewState!=DISABLE)
        printf("usb pull up enable\r\n");
    else
        printf("usb pull up disable\r\n");*/
}

//USB使能连接/断线   enable:0,断开;1,允许连接
void USB_Port_Set(u8 enable) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);    //使能PORTA时钟
    if(enable) {
        _SetCNTR(_GetCNTR()&(~(1<<1)));//退出断电模式
    } else {
        _SetCNTR(_GetCNTR()|(1<<1));  // 断电模式
        GPIOA->CRH&=0XFFF00FFF;       //
        GPIOA->CRH|=0X00033000;       // Rx(PA11)和Tx(PA12) 两个线 通用推挽输出+输出模式50MHz
        //PAout(12)=0;
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
