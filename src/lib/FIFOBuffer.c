#include <lib/FIFOBuffer.h>
#include <string.h>
#include <lib/utils.h>
void FIFO_Inititalize(FIFO_t* pFifo,uint8_t* pBuffer,uint16_t uiSize){
	pFifo->pBuffer	= pBuffer;
	pFifo->uiIn		= 0;
	pFifo->uiOut	= 0;
	pFifo->uiSize	= uiSize;

	pFifo->xMutex		= xSemaphoreCreateMutex();
	pFifo->xAvailable	= xSemaphoreCreateBinary();
}
uint16_t FIFO_Read(FIFO_t* pFifo,uint8_t* pBuffer,uint16_t length){
	uint16_t tempIn;
	uint16_t readableLength=0;
	while(readableLength<1){
		tempIn = pFifo->uiIn;
		readableLength = MIN(length,(tempIn+pFifo->uiSize-pFifo->uiOut)%pFifo->uiSize);
		if(readableLength<1){
			FIFO_Wait(pFifo);
		}
	}
	uint16_t part1Length = MIN(pFifo->uiSize-pFifo->uiOut,readableLength);
	uint16_t part2Length = readableLength-part1Length;

	// 拷贝末尾前的一部分字节
	memcpy(pBuffer,pFifo->pBuffer+pFifo->uiOut,part1Length);
	// 拷贝绕过来的一部分字节
	memcpy(pBuffer+part1Length,pFifo->pBuffer,part2Length);

	pFifo->uiOut = (pFifo->uiOut+readableLength)%pFifo->uiSize;

	return readableLength;

}

uint8_t FIFO_ReadByte(FIFO_t* pFifo){
	uint8_t byte;
	FIFO_Read(pFifo,&byte,1);
	return byte;
}

void FIFO_Clear(FIFO_t* pFifo){
	pFifo->uiOut=pFifo->uiIn;
}
void FIFO_TakeMutex(FIFO_t* pFifo){
	BaseType_t xHigherTaskWoken = pdFALSE;
	if(__get_CONTROL()==0){
		xSemaphoreTakeFromISR(pFifo->xMutex,&xHigherTaskWoken);
		if(xHigherTaskWoken){
			portYIELD_FROM_ISR(xHigherTaskWoken);
		}
	}else{
		xSemaphoreTake(pFifo->xMutex,portMAX_DELAY);
	}
}
void FIFO_GiveMutex(FIFO_t* pFifo){
	BaseType_t xHigherTaskWoken = pdFALSE;
	if(__get_CONTROL()==0){
		xSemaphoreGiveFromISR(pFifo->xMutex,&xHigherTaskWoken);
		if(xHigherTaskWoken){
			portYIELD_FROM_ISR(xHigherTaskWoken);
		}
	}else{
		xSemaphoreGive(pFifo->xMutex);
	}
}
void FIFO_Wait(FIFO_t* pFifo){
	BaseType_t xHigherTaskWoken = pdFALSE;
	if(__get_CONTROL()==0){
		xSemaphoreGiveFromISR(pFifo->xAvailable,&xHigherTaskWoken);
		if(xHigherTaskWoken){
			portYIELD_FROM_ISR(xHigherTaskWoken);
		}
	}else{
		xSemaphoreTake(pFifo->xAvailable,portMAX_DELAY);
	}
}
void FIFO_Notify(FIFO_t* pFifo){
	BaseType_t xHigherTaskWoken = pdFALSE;
	if(__get_CONTROL()==0){
		xSemaphoreGiveFromISR(pFifo->xAvailable,&xHigherTaskWoken);
		if(xHigherTaskWoken){
			portYIELD_FROM_ISR(xHigherTaskWoken);
		}
	}else{
		xSemaphoreGive(pFifo->xAvailable);
	}
}
