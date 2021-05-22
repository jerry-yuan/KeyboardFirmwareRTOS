#include <w25x.h>
#include <stm32f10x.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <lib/utils.h>
#define W25X_DMA_Channel_Rx DMA2_Channel1
#define W25X_DMA_Channel_Tx DMA2_Channel2

void W25X_Wait_Busy();
static uint8_t* pSectorBuffer = NULL;
void W25X_Initialize() {
	pSectorBuffer = (uint8_t*)pvPortMalloc(W25X_SECTOR_SIZE);
    // 使能GPIO引脚时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE); //禁止JTAG功能（保留SWD下载口）

    /**
     * 初始化GPIO
     */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_15; //PA15 =1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    // 临时禁用Flash
    GPIO_SetBits(GPIOA,GPIO_Pin_15);

    /**
     * 初始化SPI3
     */

    // 使能SPI3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
    SPI_InitTypeDef SPI_InitStructure;

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    // 初始化SPI3
    SPI_Init(SPI3, &SPI_InitStructure);
    // 使能SPI3
    SPI_Cmd(SPI3, ENABLE);

    /**
     * 初始化DMA2
     * DMA2->Channel1 ---- 接收
     * DMA2->Channel2 ---- 发送
     */

    // 启用DMA2时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    DMA_InitTypeDef DMA_InitStructure;
    // 初始化公用DMA结构体
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI3->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t)0;
    DMA_InitStructure.DMA_BufferSize		 = (uint32_t)0;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;

    // 初始化DMA接收通道
    DMA_DeInit(W25X_DMA_Channel_Rx);
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_Init(W25X_DMA_Channel_Rx, &DMA_InitStructure);

	// 初始化DMA发送通道
	DMA_DeInit(W25X_DMA_Channel_Tx);
	DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA_Init(W25X_DMA_Channel_Tx, &DMA_InitStructure);

	// 使能SPI3通过DMA收发数据
    SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Rx, ENABLE);
    SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);

}

void W25X_ReceiveBuffer(void* buffer,uint16_t length){
	uint8_t dummy = 0xFF;
	// 清除中断
	DMA_ClearITPendingBit(DMA2_IT_GL1|DMA2_IT_GL2);
	DMA_ClearFlag(DMA2_FLAG_GL1|DMA2_FLAG_GL2);

	dummy=SPI3->DR;
	dummy=0xFF;

	// SPI接收通道(DMA2_Channel1)
	W25X_DMA_Channel_Rx->CNDTR  = (uint16_t)length;				// 拷贝数量			length
	W25X_DMA_Channel_Rx->CCR   |= DMA_MemoryInc_Enable;			// 内存增长模式		增长
	W25X_DMA_Channel_Rx->CMAR   = (uint32_t)buffer;				// 内存地址			buffer

	// SPI发送通道(DMA2_Channel2)
	W25X_DMA_Channel_Tx->CNDTR  = (uint16_t)length;				// 拷贝数量			length
	W25X_DMA_Channel_Tx->CCR   &= ~DMA_MemoryInc_Enable;		// 内存增长模式		不增长
	W25X_DMA_Channel_Tx->CMAR   = (uint32_t)&dummy;				// 内存地址			buffer

	// 启动DMA
	DMA_Cmd(W25X_DMA_Channel_Rx,ENABLE);
	DMA_Cmd(W25X_DMA_Channel_Tx,ENABLE);

	while(DMA_GetFlagStatus(DMA2_FLAG_TC1)==RESET);	//TODO: 改成信号量调度的
	// 关掉DMA
	DMA_Cmd(W25X_DMA_Channel_Rx,DISABLE);
	DMA_Cmd(W25X_DMA_Channel_Tx,DISABLE);
}
void W25X_SendBuffer(const void* buffer,uint16_t length) {
    uint8_t dummy = 0xFF;
	// 清除中断
	DMA_ClearITPendingBit(DMA2_IT_GL1|DMA2_IT_GL2);
	DMA_ClearFlag(DMA2_FLAG_GL1|DMA2_FLAG_GL2);

	// 清除SPI中残留的数据
	SPI3->DR;

	// SPI接收通道(DMA2_Channel1)
	W25X_DMA_Channel_Rx->CNDTR  = (uint16_t)length;			// 拷贝数量			length
	W25X_DMA_Channel_Rx->CCR   &= ~DMA_MemoryInc_Enable;	// 内存增长模式		不增长
	W25X_DMA_Channel_Rx->CMAR   = (uint32_t)&dummy;			// 内存地址			dummy

	// SPI发送通道(DMA2_Channel2)
	W25X_DMA_Channel_Tx->CNDTR  = (uint16_t)length;			// 拷贝数量			length
	W25X_DMA_Channel_Tx->CCR   |= DMA_MemoryInc_Enable;		// 内存增长模式		增长
	W25X_DMA_Channel_Tx->CMAR   = (uint32_t)buffer;			// 内存地址			buffer

	// 启动DMA
	DMA_Cmd(W25X_DMA_Channel_Rx,ENABLE);
	DMA_Cmd(W25X_DMA_Channel_Tx,ENABLE);

	while(DMA_GetFlagStatus(DMA2_FLAG_TC2)==RESET){

	}
	// 关掉DMA
	DMA_Cmd(W25X_DMA_Channel_Rx,DISABLE);
	DMA_Cmd(W25X_DMA_Channel_Tx,DISABLE);
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
void W25X_Write_Status(W25XStatus_t puiStatus) {
    const uint8_t command[]= {W25X_CMD_WRITE_STATUS};

    GPIO_ResetBits(GPIOA,GPIO_Pin_15);
    W25X_SendBuffer(command,sizeof(command));
    W25X_SendBuffer(&puiStatus,sizeof(W25XStatus_t));
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
void W25X_Write_Page(uint32_t uiAddress,void* pBuffer,uint32_t uiLength) {
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
void W25X_Write_Buffer(uint32_t uiAddress,uint8_t* pBuffer,uint32_t uiLength) {
	uint32_t uiSectorAddress;		// 扇区的首地址
	uint32_t uiSectorWriteAddress;	// (每扇区)写入位置的地址
	uint8_t* puiSectorBuffer;		// (每扇区)写入数据来源
	uint16_t uiAddressOffset;		// (每扇区)写入数据的位置相对扇区首地址的偏移量
	uint16_t uiWriteLength;			// (每扇区)写入的pBuffer长度
    uint16_t uiSectorWriteLength;	// (每扇区)实际需要写入的长度
    uint16_t  uiPageWriteLength;		// (每页)需要写入的长度
    uint8_t	 uiFlags;				// 擦除必要性标志

    // 计算涉及写入的第一个扇区的首地址
    uiSectorAddress = uiAddress - uiAddress % W25X_SECTOR_SIZE;
    // 每次循环处理一个扇区写入
	while(uiLength>0){
		W25X_Wait_Busy();
		uiFlags = 0;
		// 计算写入位置相对当前扇区起始位置的偏移
		uiAddressOffset = uiAddress-uiSectorAddress;
		// 当前扇区要写入的数据量 = 剩余要写的量 和 扇区剩余可写的量 中较小的一个
		uiWriteLength = MIN(W25X_SECTOR_SIZE-uiAddressOffset,uiLength);
		// 读出当前扇区现有的数据
		W25X_Read_Data(uiSectorAddress,pSectorBuffer,W25X_SECTOR_SIZE);
		//dumpMemory(SectorBuffer,W25X_SECTOR_SIZE);
		// 检查是否需要擦除
		for(uint16_t uiByteIndex=0;uiByteIndex<uiWriteLength;uiByteIndex++){
			// ~原有 & 新的 = 是否有位从0变1了 ==> 需要擦除
			uiFlags |= ((~pSectorBuffer[uiAddressOffset+uiByteIndex])&pBuffer[uiByteIndex])?1:0;
			// 将数据覆盖到缓冲区
			pSectorBuffer[uiAddressOffset+uiByteIndex] = pBuffer[uiByteIndex];
		}
		if(uiFlags){
			// 需要擦除 => 擦除扇区,修改扇区缓冲中的数据,然后完整写回
			// 擦除扇区
			W25X_Erase_Sector(uiSectorAddress);
			// 修改写入位置的信息(从头写,写一个扇区)
			uiSectorWriteAddress = uiSectorAddress;
			uiSectorWriteLength = W25X_SECTOR_SIZE;
			puiSectorBuffer     = pSectorBuffer;
		}else{
			// 无需擦除 => 直接写入对应区域即可
			uiSectorWriteAddress = uiAddress;
			uiSectorWriteLength = uiWriteLength;
			puiSectorBuffer     = pSectorBuffer+uiAddressOffset;
		}
		// 开始页编程
		while(uiSectorWriteLength>0){
			// 计算当前页最多能写入的长度
			uiPageWriteLength = MIN(W25X_MAX_PAGE,uiSectorWriteLength);
			// 写入一页
			W25X_Write_Page(uiSectorWriteAddress,puiSectorBuffer,uiPageWriteLength);
			W25X_Wait_Busy();
			// 更新写入地址与剩余长度
			uiSectorWriteAddress += uiPageWriteLength;
			puiSectorBuffer      += uiPageWriteLength;
			uiSectorWriteLength  -= uiPageWriteLength;
		}
		// 更新地址
		uiSectorAddress += W25X_SECTOR_SIZE;
		uiAddress 		+= uiWriteLength;
		pBuffer 		+= uiWriteLength;
		uiLength        -= uiWriteLength;
	}
	return;
}
