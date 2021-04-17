#ifndef FIFO_BUFFER_H
#define FIFO_BUFFER_H

#include <stm32f10x.h>
#include <FreeRTOS.h>
#include <semphr.h>

typedef struct {
    uint8_t* pBuffer;
    uint16_t uiIn;
    uint16_t uiOut;
    uint16_t uiSize;

	SemaphoreHandle_t xMutex;
	SemaphoreHandle_t xAvailable;
} FIFO_t;

void FIFO_Inititalize(FIFO_t* pFifo,uint8_t* pBuffer,uint16_t uiSize);
uint16_t FIFO_Read(FIFO_t* pFifo,uint8_t* pBuffer,uint16_t length);
void FIFO_Clear(FIFO_t* pFifo);
void FIFO_TakeMutex(FIFO_t* pFifo);
void FIFO_GiveMutex(FIFO_t* pFifo);
void FIFO_Wait(FIFO_t* pFifo);
void FIFO_Notify(FIFO_t* pFifo);
#endif
