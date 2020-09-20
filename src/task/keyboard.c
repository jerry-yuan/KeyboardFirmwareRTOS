#include <task/keyboard.h>
#include <task/keyscan.h>
#include <task/priorities.h>
#include <task/gui.h>

#include <bsp/oled.h>
#include <bsp/W25Q64.h>

#include <usb/hw_config.h>
#include <usb_lib.h>

#include <screen/consts.h>

//#define STAMP() printf("STAMP:%s:%d\r\n",__FILE__,__LINE__)

//键盘事件发送任务
TaskHandle_t hKeyboardTask=NULL;
//键盘状态显示任务
TaskHandle_t hKeyboardStatTask=NULL;
//键盘状态更新事件
EventGroupHandle_t hKeyboardStateUpdateEvent=NULL;
//键盘状态
uint32_t keyboardStatus=0x00;

/*
static void keyboardTask(void) {
    BaseType_t xReturn=pdPASS;

    KeyUpdateEvent_t keyUpdateEvent;
    KEY_EVENT* pEvent=NULL;

    while(1) {
        xReturn=xQueueReceive(keyUpdateEventQueue,&keyUpdateEvent,portMAX_DELAY);
        if(xReturn==pdFALSE) {
            // 读取失败就跳过
            continue;
        }
        pEvent                  = pvPortMalloc(sizeof(KEY_EVENT));
        pEvent->Head.iID        = KEY_EVENT_ID;
        pEvent->Head.iSize      = sizeof(KEY_EVENT);

        pEvent->Data.stPress.keyCodes       = (uint32_t*)pvPortMalloc(sizeof(uint32_t)*keyUpdateEvent.pressedCount);
        pEvent->Data.stPress.cursor         = 0;
        pEvent->Data.stPress.length         = keyUpdateEvent.pressedCount;

        pEvent->Data.stRelease.keyCodes     = (uint32_t*)pvPortMalloc(sizeof(uint32_t)*keyUpdateEvent.releaseCount);
        pEvent->Data.stRelease.cursor       = 0;
        pEvent->Data.stRelease.length       = keyUpdateEvent.releaseCount;

        mapKeyCodes(keyUpdateEvent.pressed,pEvent->Data.stPress.keyCodes);
        mapKeyCodes(keyUpdateEvent.release,pEvent->Data.stRelease.keyCodes);

        clearList(keyUpdateEvent.pressed);
        clearList(keyUpdateEvent.release);

        xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);

    }
}*/
static void keyboardStatusTask() {
    EventBits_t xEvent;
    KEYBOARD_STATE_EVENT* pEvent;
    while(1) {
        xEvent=xEventGroupWaitBits(hKeyboardStateUpdateEvent,
                                   KEY_STATE_EVENT_UPDATE,
                                   pdTRUE,
                                   pdFALSE,
                                   portMAX_DELAY);

        if(xEvent & KEY_STATE_EVENT_UPDATE) {
            pEvent = pvPortMalloc(sizeof(KEYBOARD_STATE_EVENT));

            pEvent->Head.iID    = KEYBOARD_STATE_EVENT_ID;
            pEvent->Head.iSize  = sizeof(KEYBOARD_STATE_EVENT);
            pEvent->Data        = keyboardStatus;

            xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);

        }
        xEventGroupClearBits(hKeyboardStateUpdateEvent,0xff);

    }
}

void keyboardTaskInitialize() {
    BaseType_t xReturn = pdPASS;

    hKeyboardStateUpdateEvent=xEventGroupCreate();
    //xReturn = xTaskCreate((TaskFunction_t)keyboardTask,"keyboard",128,NULL,TASK_KEYBOARD_PRIORITY,&hKeyboardTask);
    //if(xReturn == pdPASS) {
    //    printf("Create keyboard task success!\r\n");
    //}
    xReturn = xTaskCreate((TaskFunction_t)keyboardStatusTask,"keyboard-state",128,NULL,TASK_KEYBOARD_STATE_PRIORITY,&hKeyboardStatTask);
    if(xReturn == pdPASS) {
        printf("Create keyboard stat task success!\r\n");
    }
}
