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

//键盘状态显示任务
TaskHandle_t hKeyboardStatTask=NULL;
//键盘状态更新事件
EventGroupHandle_t hKeyboardStateUpdateEvent=NULL;
//键盘状态
uint32_t keyboardStatus=0x00;

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

    xReturn = xTaskCreate((TaskFunction_t)keyboardStatusTask,"keyboard-state",128,NULL,TASK_KEYBOARD_STATE_PRIORITY,&hKeyboardStatTask);
    if(xReturn == pdPASS) {
        printf("Create keyboard stat task success!\r\n");
    }
}
