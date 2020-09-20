#include <task/keyscan.h>
#include <task/priorities.h>
#include <task/gui.h>
#include <bsp/keyboard.h>
#include <bsp/oled.h>
#include <screen/consts.h>

#include <delay.h>



TaskHandle_t hKeyScanTask;
//QueueHandle_t keyUpdateEventQueue;
void clearKeyUpdateInfoList(KeyUpdateInfo_t* head) {
    KeyUpdateInfo_t* temp;
    while(head!=NULL) {
        temp=head;
        head=head->next;
        vPortFree(temp);
    }
}
static void keyScanTask(void) {
    uint8_t keyTemp;
    uint8_t* keyboardPushStatus=pvPortMalloc(sizeof(uint8_t)*21);
    KEY_EVENT* keyEvent=pvPortMalloc(sizeof(KEY_EVENT));
    KeyUpdateInfo_t* keyUpdateInfo=NULL;

    keyEvent				= pvPortMalloc(sizeof(KEY_EVENT));
    keyEvent->Head.iID 		= KEY_EVENT_ID;
    keyEvent->Head.iSize	= sizeof(KEY_EVENT);

    keyEvent->Data.pstPressed		= NULL;
    keyEvent->Data.uiPressedCount	= 0;
    keyEvent->Data.pstRelease		= NULL;
    keyEvent->Data.uiReleaseCount	= 0;

    while(1) {
        KEY_ResetPulse();
        for(uint8_t colIndex=0; colIndex<21; colIndex++) {
            keyTemp=KEY_Read();
            while(keyTemp!=KEY_Read()) {
                vTaskDelay(10/portTICK_PERIOD_MS);
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
                            keyUpdateInfo->next 		= keyEvent->Data.pstRelease;
                            keyEvent->Data.pstRelease	= keyUpdateInfo;
                            keyEvent->Data.uiReleaseCount++;
                        } else {
                            // 旧状态弹起 新状态按下 ==> 进入Press列
                            keyUpdateInfo->next 		= keyEvent->Data.pstPressed;
                            keyEvent->Data.pstPressed   = keyUpdateInfo;
                            keyEvent->Data.uiPressedCount++;
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
        if(keyEvent->Data.uiPressedCount>0 || keyEvent->Data.uiReleaseCount>0) {
            xQueueSend(hEventQueue,&keyEvent,0);
            keyEvent				= pvPortMalloc(sizeof(KEY_EVENT));
            keyEvent->Head.iID 		= KEY_EVENT_ID;
            keyEvent->Head.iSize	= sizeof(KEY_EVENT);

            keyEvent->Data.pstPressed		= NULL;
            keyEvent->Data.uiPressedCount	= 0;
            keyEvent->Data.pstRelease		= NULL;
            keyEvent->Data.uiReleaseCount	= 0;
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}
void keyScanTaskInitialize() {
    BaseType_t xReturn=pdPASS;
    //keyUpdateEventQueue=xQueueCreate(64,sizeof(KeyUpdateEvent_t));
    xReturn=xTaskCreate((TaskFunction_t)keyScanTask,"keyScan",64,NULL,TASK_KEYSCAN_PRIORITY,&hKeyScanTask);
    if(xReturn==pdPASS) {
        printf("Create keyScan task success!\r\n");
    }
}
