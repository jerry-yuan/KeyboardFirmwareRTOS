#ifndef DELAY_H_INCLUDED
#define DELAY_H_INCLUDED
#include <stm32f10x.h>

void Delay_TimerInitialize(void);
void Delay_us(__IO uint32_t us);
void Delay_ms(__IO uint32_t ms);

#define DELAY_USING_TIM6

#endif /* DELAY_H_INCLUDED */
