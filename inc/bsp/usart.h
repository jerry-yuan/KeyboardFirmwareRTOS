#ifndef BSP_USART_H
#define BSP_USART_H

#include "stm32f10x.h"

void USART_Initialize(void);
void USART_SendBuffer(const uint8_t* pBuffer, const uint16_t length);

#endif
