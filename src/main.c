#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include <usb/usb.h>
#include <task/priorities.h>
#include <task/usbdevicestate.h>
#include <task/keyscan.h>
#include <task/keyboard.h>
#include <task/gui.h>
#include <task/rtc.h>
#include <task/bglight.h>
#include <stdlib.h>
#include <math.h>
static TaskHandle_t hBootstrap;

static void bootstrap(void) {
    taskENTER_CRITICAL();
    USB_Initialize();

    USBDeviceStateTaskInitialize();
    keyScanTaskInitialize();
    guiTaskInitialize();
    BgLightTaskTaskInitialize();
    keyboardTaskInitialize();

    RTCTaskInitialize();

    vTaskDelete(hBootstrap);
    taskEXIT_CRITICAL();
}
int main(void) {
    BaseType_t xReturn=pdPASS;
    BSP_Initialize();
    // 引导任务
    xReturn=xTaskCreate((TaskFunction_t)bootstrap,"bootstrap",512,NULL,TASK_BOOTSTRAP_PRIORITY,&hBootstrap);
    if(xReturn==pdPASS){
        vTaskStartScheduler();
    }else{
        printf("bootstrap task create failed!\r\n");
    }
}
