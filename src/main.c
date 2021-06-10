#include "FreeRTOS.h"
#include "task.h"
#include <bsp/bsp.h>
#include <usb/usb.h>
#include <task/priorities.h>
#include <task/keyscan.h>
#include <task/gui.h>
#include <task/bglight.h>
#include <task/irqproxy.h>
#include <stdlib.h>
#include <math.h>
static TaskHandle_t hBootstrap;

static void bootstrap(void) {
    //taskENTER_CRITICAL();

	guiTaskInitialize();
    BgLightTaskTaskInitialize();

    keyScanTaskInitialize();
    IRQProxyTaskInitialize();

	USB_Initialize();

    vTaskDelete(hBootstrap);
    //taskEXIT_CRITICAL();
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
