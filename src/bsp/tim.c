#include <bsp/tim.h>

void TIM_Initialize(){
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;
    // 初始化TIM2作为Screen Saver

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
}

void TIM_ScreenSaver_Reset(){
    TIM_Cmd(TIM2,DISABLE);
    TIM_SetCounter(TIM2,TIM_SCREEN_SAVER_COUNTER_RESET-1);
    TIM_Cmd(TIM2,ENABLE);
}

void TIM_ScreenSaver_Disable(){
    TIM_Cmd(TIM2,DISABLE);
    TIM_SetCounter(TIM2,0);
}

bool TIM_ScreenSaver_IsEnabled(){
    return (TIM2->CR1 & TIM_CR1_CEN)?true:false;
}
