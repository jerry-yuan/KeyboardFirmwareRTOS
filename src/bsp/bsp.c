#include "bsp/bsp.h"

#include<delay.h>
#include <bsp/usart.h>
#include <bsp/statled.h>
#include <bsp/W25Q64.h>
#include <bsp/oled.h>
#include <bsp/keyboard.h>
#include <bsp/rtc.h>
#include <bsp/tim.h>
void BSP_Initialize(){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    Delay_TimerInitialize();
    USART_Config();
    TIM_Initialize();
    RTC_Initialize();
    STAT_Initialize();
    OLED_Initialize();

    FLASH_Initialize();
    KEY_Initialize();
}
