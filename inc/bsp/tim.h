#ifndef TIM_H_INCLUDED
#define TIM_H_INCLUDED

#include <stm32f10x.h>
#include <stdbool.h>

#define TIM_SCREEN_SAVER_PRESCALER      36000
#define TIM_SCREEN_SAVER_PERIOD         10
#define TIM_SCREEN_SAVER_COUNTER_RESET  72000000/TIM_SCREEN_SAVER_PRESCALER*TIM_SCREEN_SAVER_PERIOD

void TIM_Initialize();

void TIM_ScreenSaver_Reset();
void TIM_ScreenSaver_Disable();
bool TIM_ScreenSaver_IsEnabled();

#endif /* TIM_H_INCLUDED */
