#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"

void USART_Initialize(void);
void USART_SendByte(uint8_t ch);

#endif
