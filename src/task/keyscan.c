#include <task/keyscan.h>
#include <task/priorities.h>

#include <bsp/keyboard.h>
#include <bsp/oled.h>
#include <delay.h>



TaskHandle_t hKeyScanTask;
QueueHandle_t keyUpdateEventQueue;

void displayKeyboardCol(uint8_t col,uint8_t data) {
    for(int j=0; j<6; j++) {
        if((data>>j) & 0x01) {
            OLED_DrawRect(44+col*8,8+j*8,8,8,0x0F,0x0A);
        } else {
            OLED_DrawRect(44+col*8,8+j*8,8,8,0x0F,0x00);
        }
    }

}

void displayKeyboardPushState() {
    for(int i=0; i<21; i++) {
        displayKeyboardCol(i,keyboardPushStatus[i]);
    }

}

static void keyScanTask(void) {
    uint8_t keyTemp;
    uint8_t* keyboardPushStatus=pvPortMalloc(sizeof(uint8_t)*21);
    KeyUpdateEvent_t keyUpdateEvent;
    KeyUpdateInfo_t* keyUpdateInfo=NULL;
    while(1) {
        keyUpdateEvent.pressed=NULL;
        keyUpdateEvent.pressedCount=0;
        keyUpdateEvent.release=NULL;
        keyUpdateEvent.releaseCount=0;
        KEY_ResetPulse();
        for(uint8_t colIndex=0; colIndex<21; colIndex++) {
            keyTemp=KEY_Read();
            while(keyTemp!=KEY_Read()) {
                Delay_ms(10);
                keyTemp=KEY_Read();
            }
            if(keyboardPushStatus[colIndex]!=keyTemp) {
                // 更新Report状态
                uint8_t oldState=keyboardPushStatus[colIndex];
                uint8_t newState=keyTemp;
                for(uint8_t rowIndex=0; rowIndex<6; rowIndex++) {
                    if((oldState ^ newState) & 0x01) {
                        // 如果第rowIndex行的按键状态不同  ==> 此键状态翻转了
                        keyUpdateInfo=(KeyUpdateInfo_t*)pvPortMalloc(sizeof(KeyUpdateInfo_t));
                        keyUpdateInfo->row=rowIndex;
                        keyUpdateInfo->column=colIndex;
                        if((oldState&0x01) && !(newState&0x01)) {
                            // 旧状态按下 新状态弹起 ==> 进入Release列
                            keyUpdateInfo->next = keyUpdateEvent.release;
                            keyUpdateEvent.release=keyUpdateInfo;
                            keyUpdateEvent.releaseCount++;
                        } else {
                            // 旧状态弹起 新状态按下 ==> 进入Press列
                            keyUpdateInfo->next = keyUpdateEvent.pressed;
                            keyUpdateEvent.pressed=keyUpdateInfo;
                            keyUpdateEvent.pressedCount++;
                        }
                    }
                    oldState = oldState >>1;
                    newState = newState >>1;

                }
                keyboardPushStatus[colIndex]=keyTemp;
            }
            KEY_NextColumn();
        }
        // 扫描完一遍,如果有按键事件则提交按键事件队列
        if((keyUpdateEvent.pressed !=NULL) || (keyUpdateEvent.release!=NULL)){
            xQueueSend(keyUpdateEventQueue,&keyUpdateEvent,0);
        }
        vTaskDelay(10);
    }
}
void keyScanTaskInitialize() {
    BaseType_t xReturn=pdPASS;
    keyUpdateEventQueue=xQueueCreate(64,sizeof(KeyUpdateEvent_t));
    xReturn=xTaskCreate((TaskFunction_t)keyScanTask,"keyScan",64,NULL,TASK_KEYSCAN_PRIORITY,&hKeyScanTask);
    if(xReturn==pdPASS) {
        printf("create keyScan task success!\r\n");
    }
}
