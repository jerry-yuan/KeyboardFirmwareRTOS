#include <task/usbdevicestate.h>

#include <task.h>

#include <task/gui.h>
#include <screen/consts.h>

TaskHandle_t  hUSBDeviceStateTask;
QueueHandle_t hUSBDeviceStateMessageQueue;

static void USBDeviceStateTask(void* parameters);

void USBDeviceStateTaskInitialize(){
    BaseType_t xReturn=pdPASS;

    hUSBDeviceStateMessageQueue = xQueueCreate(8,sizeof(uint32_t));

    xReturn = xTaskCreate((TaskFunction_t)USBDeviceStateTask,"USBDeviceStateTask",512,NULL,TASK_USB_MESSAGE_PRIORITY,&hUSBDeviceStateTask);
    if(xReturn==pdPASS){
        printf("Create USBDeviceStateTask Success!\r\n");
    }
}

void USBDeviceStateTask(void* parameters){
    BaseType_t xReturn;
    uint32_t bDeviceState;
    USB_STATE_EVENT* pEvent;
    while(1){
        xReturn=xQueueReceive(hUSBDeviceStateMessageQueue,&bDeviceState,portMAX_DELAY);
        if(xReturn == pdPASS){
            pEvent = pvPortMalloc(sizeof(USB_STATE_EVENT));
            pEvent ->Head.iID   = USB_STATE_EVENT_ID;
            pEvent ->Head.iSize = sizeof(USB_STATE_EVENT);
            pEvent ->Data.uiDeviceState=bDeviceState;
            xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);
        }
    }
}
