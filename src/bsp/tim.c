#include <bsp/tim.h>
#include <bsp/oled.h>

#include <screen/consts.h>

#include <task/irqproxy.h>
void (*TIM_KeyRepeater_IRQHandler)()=NULL;
void TIM_KeyRepeater_DelayTimeout();
void TIM_KeyRepeater_RepeatTimeout();

SGUI_INT					iLastAction=0;
void TIM_Initialize() {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;
    /**
     * 初始化TIM2作为Screen Saver
     */

    // 初始化中断优先级
    NVIC_InitStructure.NVIC_IRQChannel                      = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0;

    NVIC_Init(&NVIC_InitStructure);

    // 开启TIM2时钟源
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    TIM_TimeBaseStructure.TIM_Period        = TIM_SCREEN_SAVER_COUNTER_RESET-1;             // 自动重装载寄存器的值,用处不大,基本全是手动设定
    TIM_TimeBaseStructure.TIM_Prescaler     = TIM_SCREEN_SAVER_PRESCALER-1 ;                // 时钟预分频数为 1/36000,计数频率为72MHz / 36000 2kHz
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Down;                         // 计数器计数模式，基本定时器只能向上计数，没有计数模式的设置
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    // 初始化定时器
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    // 清除定时器标志
    TIM_ClearFlag(TIM2,TIM_FLAG_Update);
    // 使能定时器中断
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);

    /**
     * 初始化 TIM3作为KeyRepeater
     */

    // 初始化中断优先级
    NVIC_InitStructure.NVIC_IRQChannel                      = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0;

    NVIC_Init(&NVIC_InitStructure);

    // 开启TIM3时钟源
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period        = TIM_KEY_REPEATER_REPEAT_COUNTER_RESET-1;             // 自动重装载寄存器的值,用处不大,基本全是手动设定
    TIM_TimeBaseStructure.TIM_Prescaler     = TIM_KEY_REPEATER_PRESCALER-1 ;                // 时钟预分频数为 1/36000,计数频率为72MHz / 36000 2kHz
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Down;                         // 计数器计数模式，基本定时器只能向上计数，没有计数模式的设置
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

    // 初始化定时器
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    // 清除定时器标志
    TIM_ClearFlag(TIM3,TIM_FLAG_Update);
    // 使能定时器中断
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
}

void TIM_ScreenSaver_Reset() {
    TIM_Cmd(TIM2,DISABLE);
    TIM_SetCounter(TIM2,TIM_SCREEN_SAVER_COUNTER_RESET-1);
    TIM_Cmd(TIM2,ENABLE);
}

void TIM_ScreenSaver_Disable() {
    TIM_Cmd(TIM2,DISABLE);
    TIM_SetCounter(TIM2,0);
}

bool TIM_ScreenSaver_IsEnabled() {
    return (TIM2->CR1 & TIM_CR1_CEN)?true:false;
}

void TIM_ScreenSaver_IRQHandler() {
    TIM_Cmd(TIM2,DISABLE);
    TIM_SetCounter(TIM2,0);
    OLED_SetDisplayState(false);
}

void TIM_KeyRepeater_DelayTimeout() {
    BaseType_t xHigherPriorityTaskWoken=pdFALSE;
    BaseType_t xReturn=pdFAIL;
    TIM_Cmd(TIM3,DISABLE);
    TIM_SetCounter(TIM3,TIM_KEY_REPEATER_REPEAT_COUNTER_RESET-1);
    TIM_KeyRepeater_IRQHandler=TIM_KeyRepeater_RepeatTimeout;
    TIM_Cmd(TIM3,ENABLE);
    xReturn=xEventGroupSetBitsFromISR(hIRQEventGroup,IRQ_EVENT_MASK_KEYREPEAT_TIMEOUT,&xHigherPriorityTaskWoken);
    if(xReturn == pdFAIL) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
void TIM_KeyRepeater_RepeatTimeout() {
	BaseType_t xHigherPriorityTaskWoken=pdFALSE;
    BaseType_t xReturn=pdFAIL;
    xReturn=xEventGroupSetBitsFromISR(hIRQEventGroup,IRQ_EVENT_MASK_KEYREPEAT_TIMEOUT,&xHigherPriorityTaskWoken);
    if(xReturn == pdFAIL) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
void TIM_KeyRepeater_Set() {
    TIM_KeyRepeater_IRQHandler=TIM_KeyRepeater_DelayTimeout;
    TIM_Cmd(TIM3,DISABLE);
    TIM_SetCounter(TIM3,TIM_KEY_REPEATER_DELAY_COUNTER_RESET-1);
    TIM_Cmd(TIM3,ENABLE);
}
void TIM_KeyRepeater_Reset() {
    TIM_Cmd(TIM3,DISABLE);
}

