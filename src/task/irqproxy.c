#include <task/irqproxy.h>
#include <task/priorities.h>
#include <task/gui.h>
#include <screen/consts.h>

extern uint32_t bDeviceState;
EventGroupHandle_t hIRQEventGroup;
TaskHandle_t hIRQProxyTask;
static void IRQProxyTask();

void IRQProxyTaskInitialize(){
	BaseType_t xResult=pdFALSE;

	hIRQEventGroup = xEventGroupCreate();

	xResult = xTaskCreate((TaskFunction_t)IRQProxyTask,"IRQ-Proxy",128,NULL,TASK_IRQ_EVENT_PRIORITY,&hIRQProxyTask);
	if(xResult){
		printf("Create IRQ-Proxy Success!\n");
	}
}

void IRQProxyTask(){
	EventBits_t uxEventBits=0;

	// 使能RTC秒中断
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_WaitForLastTask();

	while(true){
		uxEventBits=xEventGroupWaitBits(hIRQEventGroup,IRQ_EVENT_MASK_ALL,true,false,portMAX_DELAY);
		// 处理RTC秒中断
		if(uxEventBits & IRQ_EVENT_MASK_RTC_SECOND){
			RTC_EVENT* pEvent = pvPortMalloc(sizeof(RTC_EVENT));

			pEvent->Head.iID = RTC_EVENT_ID;
            pEvent->Head.iSize=sizeof(RTC_EVENT);

            xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);
		}
		// 处理USB状态变化
		if(uxEventBits & IRQ_EVENT_MASK_USB_STATE_UPDATE){
			USB_STATE_EVENT* pEvent = pvPortMalloc(sizeof(USB_STATE_EVENT));

			pEvent ->Head.iID   = USB_STATE_EVENT_ID;
            pEvent ->Head.iSize = sizeof(USB_STATE_EVENT);
            pEvent ->Data.uiDeviceState=bDeviceState;

            xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);
		}
		// 处理键盘状态变化
		if(uxEventBits & IRQ_EVENT_MASK_KEYBOARD_UPDATE){
			KEYBOARD_STATE_EVENT* pEvent = pvPortMalloc(sizeof(KEYBOARD_STATE_EVENT));

            pEvent->Head.iID    = KEYBOARD_STATE_EVENT_ID;
            pEvent->Head.iSize  = sizeof(KEYBOARD_STATE_EVENT);

            xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);
		}
		// 处理KeyRepeater
		if(uxEventBits & IRQ_EVENT_MASK_KEYREPEAT_TIMEOUT){
			KEY_REPEAT_EVENT* pEvent = pvPortMalloc(sizeof(KEY_REPEAT_EVENT));

			pEvent->Head.iID	= KEY_REPEAT_EVENT_ID;
			pEvent->Head.iSize	= sizeof(KEY_REPEAT_EVENT);

			xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);

		}

	}
}
