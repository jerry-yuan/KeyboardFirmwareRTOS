#include "bsp/usart.h"
#include "lib/FIFOBuffer.h"
#include "lib/utils.h"
#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <string.h>

static SemaphoreHandle_t	xTxWait  = NULL;
static SemaphoreHandle_t	xTxMutex = NULL;
static SemaphoreHandle_t    xRxFlag  = NULL;
FIFO_t*						pRxFIFO	 = NULL;

void USART_Initialize(void) {
    // 初始化信号量
    xTxMutex = xSemaphoreCreateCounting(1,1);
    xTxWait  = xSemaphoreCreateCounting(1,0);
    xRxFlag  = xSemaphoreCreateCounting(1,0);

    pRxFIFO  = pvPortMalloc(sizeof(FIFO_t));

    FIFO_Inititalize(pRxFIFO,pvPortMalloc(1200),1200);
    /**
     * 初始化USART
     */
    // 开启USART1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    // 初始化USART1
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate 				= 460800;
    USART_InitStructure.USART_WordLength			= USART_WordLength_8b;
    USART_InitStructure.USART_StopBits				= USART_StopBits_1;
    USART_InitStructure.USART_Parity				= USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode 					= USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // 使能串口
    USART_Cmd(USART1, ENABLE);

    /**
     * 初始化GPIO
     */
    // 开启GPIOA的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;

    // TxD -> PA9
    GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_9;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // RxD -> PA10
    GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin		= GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /**
     * 初始化USART DMA
     */
    // 开启DMA1时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
    // 初始化DMA初始化结构
    DMA_InitTypeDef DMA_InitStructure;

    DMA_InitStructure.DMA_PeripheralBaseAddr	= (uint32_t)&USART1->DR;
    DMA_InitStructure.DMA_PeripheralInc			= DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc				= DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize		= DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Priority				= DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M					= DMA_M2M_Disable;

    // 初始化Tx的DMA
    DMA_InitStructure.DMA_Mode					= DMA_Mode_Normal;
    DMA_InitStructure.DMA_MemoryBaseAddr		= (uint32_t)0;
    DMA_InitStructure.DMA_BufferSize			= (uint32_t)0;
    DMA_InitStructure.DMA_DIR					= DMA_DIR_PeripheralDST;
    DMA_Init(DMA1_Channel4,&DMA_InitStructure);

    // 初始化Rx的DMA
    DMA_InitStructure.DMA_Mode					= DMA_Mode_Circular;
    DMA_InitStructure.DMA_MemoryBaseAddr		= (uint32_t)pRxFIFO->pBuffer;
    DMA_InitStructure.DMA_BufferSize			= (uint32_t)pRxFIFO->uiSize;
    DMA_InitStructure.DMA_DIR					= DMA_DIR_PeripheralSRC;
    DMA_Init(DMA1_Channel5,&DMA_InitStructure);

    // 使能DMA通道请求
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

    // 使能DMA1_Channel5(Rx)
    DMA_Cmd(DMA1_Channel5, ENABLE);

    // 初始化DMA中断配置
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	= 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority			= 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd					= ENABLE;

    // 初始化Tx DMA中断
    NVIC_InitStructure.NVIC_IRQChannel						= DMA1_Channel4_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    // 初始化Rx DMA中断
    NVIC_InitStructure.NVIC_IRQChannel						= DMA1_Channel5_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    // 初始化USART中断
    NVIC_InitStructure.NVIC_IRQChannel						= USART1_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    // 清除中断标志
    DMA_ClearITPendingBit(DMA1_IT_GL4 | DMA1_IT_GL5);
    USART_ClearITPendingBit(USART1, USART_IT_IDLE);

    // 使能中断
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
}
void USART_SendBuffer(const uint8_t* pBuffer,uint16_t length) {
    if(length < 1) {
        pBuffer = (uint8_t*)"WARNING:TOO LONG TO SEND BUFFER!\r\n";
        length  = 34;
    }
    xSemaphoreTake(xTxMutex, portMAX_DELAY);
    // 清除中断
    DMA_ClearITPendingBit(DMA1_IT_GL4);
    DMA_ClearFlag(DMA1_FLAG_GL4);

    //关闭DMA
    DMA_Cmd(DMA1_Channel4, DISABLE);

    // 设置DMA通道
    DMA1_Channel4->CNDTR	= (uint16_t)length;		//拷贝数量
    DMA1_Channel4->CMAR		= (uint32_t)pBuffer;	//来源地址

    //启动DMA
    DMA_Cmd(DMA1_Channel4, ENABLE);

    // 堵死当前任务等待DMA干活,但CPU切到其他任务该干嘛干嘛去
    // 等干完再释放出来
    xSemaphoreTake(xTxWait,portMAX_DELAY);
    //while(DMA_GetFlagStatus(DMA1_FLAG_TC4)==RESET);

    // 归还互斥信号量
    xSemaphoreGive(xTxMutex);
}

void DMA1_Channel4_IRQHandler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(DMA_GetITStatus(DMA1_IT_TC4) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC4);
        // 关掉DMA
        DMA_Cmd(DMA1_Channel4, DISABLE);
        if(xTxWait != NULL)
            xSemaphoreGiveFromISR(xTxWait, &xHigherPriorityTaskWoken);
    }
}

void USART1_IRQHandler(void) {
	uint32_t temp = 0;
    if(USART_GetITStatus(USART1,USART_IT_IDLE) != RESET) {
        pRxFIFO->uiIn = pRxFIFO->uiSize-DMA_GetCurrDataCounter(DMA1_Channel5);
		FIFO_Notify(pRxFIFO);
        temp = USART1->SR;
        temp = USART1->DR;
        USART_ClearITPendingBit(USART1,USART_IT_IDLE);
        //xSemaphoreGiveFromISR(xTxMutex);
    }
}

int _write(int fd, char *ptr,int len) {
    uint16_t tlen;
    int totalWrite=0;
    if(__get_CONTROL()==0) {
        while((DMA1_Channel4->CCR & DMA_CCR1_EN) && DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET);
        USART_SendData(USART1, 'C');
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        while(*ptr && (totalWrite<len)) {
            USART_SendData(USART1, (uint8_t) (*ptr++));
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
            totalWrite++;
        }
    } else {
        while(len>0) {
            tlen = MIN(0xFFFF,len);
            USART_SendBuffer((uint8_t*)ptr,tlen);
            len -= tlen;
            totalWrite+=tlen;
        }
    }
    return totalWrite;



}
int _read(int fd,char *ptr,int len) {
	return FIFO_Read(pRxFIFO,(uint8_t*)ptr,len);
    /*while(i<len) {
        while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

        *ptr++=(int)USART_ReceiveData(DEBUG_USARTx);
        i++;
    }*/
}
