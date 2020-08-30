#include <task/bglight.h>
#include <task/priorities.h>

#include <bsp/bglight.h>

TaskHandle_t hBgLightTask;

static void BgLightTask(void* parameters);

void BgLightTaskTaskInitialize(){
    BaseType_t xReturn=pdPASS;

    xReturn = xTaskCreate((TaskFunction_t)BgLightTask,"bgLightTask",64,NULL,TASK_BGLIGHT_PRIORITY,&hBgLightTask);
    if(xReturn == pdPASS){
        printf("Create bgLightTask success!\r\n");
    }
}

void BgLightTask(void* parameters){
    int col = 0;
    //LED_PrepareRowData(0xFF);
    while(1){
        LED_PrepareRowData(0xFF);
        LED_SetColData(~(0x111111<<col));
        col=(col+1)%4;
        vTaskDelay(1);
    }
}
