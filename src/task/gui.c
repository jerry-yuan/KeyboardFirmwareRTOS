#include <gui.h>
#include <string.h>
#include <SGUI_FontResource.h>
#include <screen/hmi.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

QueueHandle_t hEventQueue=NULL;
TaskHandle_t hGuiTaskHandle;

static void graphicsUserInterfaceTask(void*);

void guiTaskInitialize(){
    BaseType_t xReturn=pdPASS;

    hmiEngineInitialize();
    if(hEventQueue == NULL){
        hEventQueue=xQueueCreate(1024,sizeof(USB_STATE_EVENT*));
    }
    xReturn = xTaskCreate((TaskFunction_t)graphicsUserInterfaceTask,"GUI Task",512,NULL,TASK_GUI_PRIORITY,&hGuiTaskHandle);
    if(xReturn == pdPASS){
        printf("Create GUI Task Success!\r\n");
    }
}

static void graphicsUserInterfaceTask(void* parameters){
    HMI_ENGINE_RESULT           eProcessResult;
    HMI_EVENT_BASE*             pEvent;
    BaseType_t                  xReturn;
    eProcessResult=HMI_ActiveEngine(hmiEngine, SCREEN_Init_ID);
    if(HMI_PROCESS_FAILED(eProcessResult)){
        printf("ActiveEngine failed:%d\r\n",eProcessResult);
    }
    HMI_StartEngine(NULL);
    if(HMI_PROCESS_FAILED(eProcessResult)){
        printf("StartEngine failed:%d\r\n",eProcessResult);
    }
    while(1){
        xReturn=xQueueReceive(hEventQueue,&pEvent,portMAX_DELAY);
        if(xReturn==pdPASS){
            //printf("event received! id=%d size=%d type=%d\r\n",pEvent->iID,pEvent->iSize,pEvent->iType);
            HMI_ProcessEvent(pEvent);
            if(pEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pEvent,KEY_EVENT)){
                vPortFree(((KEY_EVENT*)pEvent)->Data.stPress.keyCodes);
                vPortFree(((KEY_EVENT*)pEvent)->Data.stRelease.keyCodes);
            }
            vPortFree(pEvent);
        }
    }
}
