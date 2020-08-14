#ifndef STAT_LED_H
#define STAT_LED_H

#include "stm32f10x_conf.h"

void STAT_Initialize();
void STAT_SetState(uint8_t state);
void STAT_Set();
void STAT_Reset();
void STAT_Revert();

#endif
