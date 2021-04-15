#include "bsp/bsp.h"

#include <delay.h>
#include <bsp/usart.h>
#include <bsp/statled.h>
#include <bsp/w25x.h>
#include <bsp/oled.h>
#include <bsp/keyboard.h>
#include <bsp/rtc.h>
#include <bsp/tim.h>
#include <bsp/bglight.h>

void BSP_Initialize(){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    USART_Initialize();
    STAT_Initialize();
    Delay_TimerInitialize();
    TIM_Initialize();
    RTC_Initialize();
    OLED_Initialize();
    LED_Initialize();
    W25X_Initialize();
    KEY_Initialize();
}
