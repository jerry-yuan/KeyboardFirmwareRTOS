#include <w25x.h>
#include <stm32f10x.h>
void W25X_Initialize() {
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE); //禁止JTAG功能（保留SWD下载口）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_15; //PA15 =1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA,GPIO_Pin_15);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI3, &SPI_InitStructure);
    //使能DMA发送
    DMA_DeInit(DMA2_Channel2);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI3->DR;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 1024;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode =   DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel2, &DMA_InitStructure);
    //使能DMA接收
    DMA_DeInit(DMA2_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI3->DR;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel1, &DMA_InitStructure);
    SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Rx, ENABLE);
    SPI_I2S_DMACmd(SPI3,SPI_I2S_DMAReq_Tx,ENABLE);
    SPI_Cmd(SPI3, ENABLE);
}

void W25X_SendBuffer(const void* buffer,uint32_t length) {
    DMA2->IFCR 			   |= (0xF<<4);
    DMA2_Channel2->CNDTR	= length; 					//设置要传输的数据长度
    DMA2_Channel2->CMAR		= (uint32_t)buffer; 		//设置RAM缓冲区地址
    DMA2_Channel2->CCR	   |= 0x1;
    while(!(DMA2->ISR&(1<<5)));
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
    DMA2_Channel2->CCR 	   &= (uint32_t)~0x1;
}
void W25X_ReceiveBuffer(void* buffer,uint32_t length) {
    SPI3->CR1|=SPI_Direction_2Lines_RxOnly;
    *((char*)buffer) 		= SPI3->DR;			// 读取一次数据
    DMA2->IFCR 			   |= (0xF<<0);
    DMA2_Channel1->CNDTR    = length; 			//设置要传输的数据长度
    DMA2_Channel1->CMAR     = (u32)buffer; 		//设置RAM缓冲区地址
    DMA2_Channel1->CCR     |= 0x1;
    while(!(DMA2->ISR&(1<<1)));
    DMA2_Channel1->CCR     &= (uint32_t)~0x1;
    SPI3->CR1              &= ~SPI_Direction_2Lines_RxOnly;
}
void W25X_Set_WriteState(FunctionalState writeState) {
    const uint8_t enableCmd[] = {W25X_CMD_WRITE_ENABLE};
    const uint8_t disableCmd[]= {W25X_CMD_WRITE_DISABLE};
    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    if(writeState==ENABLE) {
        W25X_SendBuffer(enableCmd,sizeof(enableCmd));
    } else {
        W25X_SendBuffer(disableCmd,sizeof(disableCmd));
    }
    GPIO_SetBits(GPIOA,GPIO_Pin_15);

}
void W25X_Read_Status(W25XStatus_t* puiStatus) {
    const uint8_t command[]= {W25X_CMD_READ_STATUS};

    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    W25X_ReceiveBuffer(puiStatus,sizeof(W25XStatus_t));
    GPIO_SetBits(GPIOA,GPIO_Pin_15);
}
void W25X_Write_Status(W25XStatus_t uiStatus) {
    const uint8_t command[]= {W25X_CMD_WRITE_STATUS};

    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    W25X_SendBuffer(&uiStatus,sizeof(W25XStatus_t));
    GPIO_SetBits(GPIOA,GPIO_Pin_15);
}
void W25X_Read_Data(uint32_t uiAddress,void* pBuffer,uint32_t uiLength) {
    const uint8_t command[]= {W25X_CMD_READ_DATA};
    uint8_t addr[3];
    addr[0] = ((uiAddress & 0xFF0000)>>16)&0xFF;
    addr[1] = ((uiAddress & 0x00FF00)>>8) &0xFF;
    addr[2] = ((uiAddress & 0x0000FF)>>0) &0xFF;
    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    W25X_SendBuffer(addr,sizeof(addr));
    W25X_ReceiveBuffer(pBuffer,uiLength);
    GPIO_SetBits(GPIOA,GPIO_Pin_15);
}
void W25X_Read_Data_Fast(uint32_t uiAddress,void* pBuffer,uint32_t uiLength) {
    const uint8_t command[]= {W25X_CMD_FAST_READ};
    uint8_t addr[4];
    addr[0] = ((uiAddress & 0xFF0000)>>16)&0xFF;
    addr[1] = ((uiAddress & 0x00FF00)>>8) &0xFF;
    addr[2] = ((uiAddress & 0x0000FF)>>0) &0xFF;
    addr[3] = W25X_DUMMY_BYTE;
    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    W25X_SendBuffer(addr,sizeof(addr));
    W25X_ReceiveBuffer(pBuffer,uiLength);
    GPIO_SetBits(GPIOA,GPIO_Pin_15);
}
void W25X_Write_Page(uint32_t uiAddress,const void* pBuffer,uint32_t uiLength) {
    const uint8_t command[]= {W25X_CMD_PAGE_PROGRAM};
    uint8_t addr[3];
    uint16_t byteToWrite = uiLength<W25X_MAX_PAGE?uiLength:W25X_MAX_PAGE;

    W25X_Set_WriteState(ENABLE);

    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    addr[0] = ((uiAddress & 0xFF0000)>>16)&0xFF;
    addr[1] = ((uiAddress & 0x00FF00)>>8) &0xFF;
    addr[2] = ((uiAddress & 0x0000FF)>>0) &0xFF;
    W25X_SendBuffer(command,sizeof(command));
    W25X_SendBuffer(addr,sizeof(addr));
    W25X_SendBuffer(pBuffer,byteToWrite);
    GPIO_SetBits(GPIOA,GPIO_Pin_15);
}
void W25X_Erase_Block(uint32_t uiAddress) {
    const uint8_t command[]= {W25X_CMD_ERASE_BLOCK};
    uint8_t addr[3];

    uiAddress -= uiAddress % 0x10000;			// 对齐64K

    addr[0] = ((uiAddress & 0xFF0000)>>16)&0xFF;
    addr[1] = ((uiAddress & 0x00FF00)>>8) &0xFF;
    addr[2] = ((uiAddress & 0x0000FF)>>0) &0xFF;

    W25X_Set_WriteState(ENABLE);

    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    W25X_SendBuffer(addr,sizeof(addr));
    GPIO_SetBits(GPIOA,GPIO_Pin_15);

}
void W25X_Erase_Sector(uint32_t uiAddress) {
    const uint8_t command[]= {W25X_CMD_ERASE_SECTOR};
    uint8_t addr[3];

    uiAddress -= uiAddress % 0x01000;			// 对齐4K

    addr[0] = ((uiAddress & 0xFF0000)>>16)&0xFF;
    addr[1] = ((uiAddress & 0x00FF00)>>8) &0xFF;
    addr[2] = ((uiAddress & 0x0000FF)>>0) &0xFF;

    W25X_Set_WriteState(ENABLE);

    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    W25X_SendBuffer(addr,sizeof(addr));
    GPIO_SetBits(GPIOA,GPIO_Pin_15);

    W25X_Wait_Busy();
}
void W25X_Erase_Chip() {
    const uint8_t command[]= {W25X_CMD_ERASE_CHIP};

    W25X_Set_WriteState(ENABLE);

    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    GPIO_SetBits(GPIOA,GPIO_Pin_15);
}
void W25X_Read_JEDECId(W25XJEDECId_t* pstJEDECId) {
    const uint8_t command[]= {W25X_CMD_READ_JEDEC_ID};

    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    W25X_ReceiveBuffer(pstJEDECId,sizeof(W25XJEDECId_t));
    GPIO_SetBits(GPIOA,GPIO_Pin_15);

    pstJEDECId->uiFlashId = ((pstJEDECId->uiFlashId>>8)&0x00FF)|((pstJEDECId->uiFlashId<<8)&0xFF00);
}
void W25X_Wait_Busy() {
    W25XStatus_t uiStatus;
    W25X_Read_Status(&uiStatus);
    while(uiStatus & W25X_STATUS_MASK_BUSY) {
        W25X_Read_Status(&uiStatus);
    }
}
void W25X_Write_Buffer(uint32_t uiAddress,const void* pBuffer,uint32_t uiLength) {
    uint16_t uiWriteLength;
    // 先把第一个不完整页给写了
    uiWriteLength = (W25X_MAX_PAGE-(uiAddress % W25X_MAX_PAGE))%W25X_MAX_PAGE;
    uiWriteLength = uiLength < uiWriteLength ? uiLength : uiWriteLength;
    if(uiWriteLength>0) {
        W25X_Write_Page(uiAddress,pBuffer,uiWriteLength);
        uiAddress += uiWriteLength;
        pBuffer   += uiWriteLength;
        uiLength  -= uiWriteLength;
        W25X_Wait_Busy();
    }
    // 把剩余的写了
    while(uiLength>0) {
        uiWriteLength = uiLength<W25X_MAX_PAGE ? uiLength:W25X_MAX_PAGE;
        W25X_Wait_Busy();
        W25X_Write_Page(uiAddress,pBuffer,uiWriteLength);
        uiAddress += uiWriteLength;
        pBuffer   += uiWriteLength;
        uiLength  -= uiWriteLength;
    }
    W25X_Wait_Busy();
}
