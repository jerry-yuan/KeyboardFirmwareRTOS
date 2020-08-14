#include "bsp/bsp.h"
#include "bsp/usart.h"
#include "USB/usb.h"


void BSP_Initialize(){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    Delay_TimerInitialize();
    USART_Config();

    STAT_Initialize();
    OLED_Initialize();
    FLASH_Initialize();
    KEY_Initialize();
}
