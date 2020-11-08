#include <task/rtc.h>
#include <task/priorities.h>

#include <task/gui.h>
#include <screen/consts.h>
// 任务指针
TaskHandle_t hRTCTask;
// RTC事件
EventGroupHandle_t hRTCEvent;

static void RTCTask(void* parameter);

void RTCTaskInitialize() {
    BaseType_t xReturn=pdPASS;

    hRTCEvent = xEventGroupCreate();

    xReturn = xTaskCreate((TaskFunction_t)RTCTask,"rtcTask",128,NULL,TASK_RTC_PRIORITY,&hRTCTask);
    if(xReturn == pdPASS) {
        printf("Create rtcTask success!\r\n");
    }
    // 使能RTC秒中断
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    /* 确保上一次 RTC 的操作完成 */
    RTC_WaitForLastTask();
}

void RTCTask(void* parameter) {
    RTC_EVENT* pEvent=NULL;
    EventBits_t xEventBits;
    while(1) {
        xEventBits=xEventGroupWaitBits(hRTCEvent,RTC_EVENT_BIT,true,false,portMAX_DELAY);
        if(xEventBits & RTC_EVENT_BIT){
            pEvent = pvPortMalloc(sizeof(RTC_EVENT));

            pEvent->Head.iID = RTC_EVENT_ID;
            pEvent->Head.iSize=sizeof(RTC_EVENT);

            //xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);
        }
    }
}
