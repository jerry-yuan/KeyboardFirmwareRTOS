/*********************************Copyright (c)*********************************
**
**                                 FIVE工作组
**
**---------------------------------File Info------------------------------------
** File Name:               w25q64.c
** Last modified Date:      2013/9/10 9:32:33
** Last Version:            V1.2
** Description:             none
**
**------------------------------------------------------------------------------
** Created By:              wanxuncpx
** Created date:            2013/8/6 21:12:35
** Version:                 V1.2
** Descriptions:            none
**------------------------------------------------------------------------------
** HW_CMU:                  STM32F103ZET6
** Libraries:               STM32F10x_StdPeriph_Driver
** version                  V3.5
*******************************************************************************/


/******************************************************************************
更新说明:
******************************************************************************/




/******************************************************************************
*********************************  应 用 资 料 ********************************
******************************************************************************/




/******************************************************************************
********************************* 文件引用部分 ********************************
******************************************************************************/
#include "w25q64.h"
#include "FreeRTOS.h"

/******************************************************************************
********************************* 数 据 声 明 *********************************
******************************************************************************/
/*---------------------*
*     数据定义(输出)
*----------------------*/
uint8_t* W25X_Buffer;//[W25X_SECTOR_SIZE];
volatile bool sem_W25X_DMA_Busy = true;
volatile bool sem_W25X_DMA_RxRdy= false;


/*---------------------*
*       数据定义(内部用)
*----------------------*/
static uint8_t  W25X_TX_Byte=0xFF;


void FLASH_Initialize(){
    W25X_Buffer=pvPortMalloc(W25X_SECTOR_SIZE);
    W25X_GPIO_Config();
    W25X_Init();
}



/******************************************************************************
********************************* 函 数 声 明 *********************************
******************************************************************************/
/******************************************************************************
/ 函数功能:初始化W25Q64的GPIO口连接
/ 修改日期:2013/9/10 19:04:15
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_GPIO_Config(void) {
    /* Private typedef ---------------------------------------------------------*/
    GPIO_InitTypeDef GPIO_InitStruct;

    /*Enable or disable APB2 peripheral clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
    // 关掉JTAG对SPI3 NSS引脚的占用
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE) ;
    /** SPI3 GPIO Configuration
         PB3     ------> SPI3_SCK    AF_OUT
         PB4     ------> SPI3_MISO   IN
         PB5     ------> SPI3_MOSI   AF_OUT
    */
    /*Configure GPIO pin */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);


    /** SPI3 GPIO Configuration
         PA15     ------> SPI3_NSS   OUT
    */
    /*Configure GPIO pin */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    W25X_CS_H();
    /*Lock of the gpio */
    GPIO_PinLockConfig(GPIOA,GPIO_Pin_15);
}


/******************************************************************************
/ 函数功能:初始化W25Q64
/ 修改日期:2013/9/10 19:04:16
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Init(void) {
    SPI_InitTypeDef  SPI_InitStructure ;
    DMA_InitTypeDef  DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    //配置DMA通道,DMA2_CH1收
    //读取SPI FLASH时多数为空数据故而数据地址无需增加
    //启动DMA2的时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
    DMA_DeInit(DMA2_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&SPI3->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)W25X_Buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel1, &DMA_InitStructure);


    //配置DMA通道,DMA1_CH3发送
    DMA_DeInit(DMA2_Channel2);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&SPI3->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(&W25X_TX_Byte);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel2, &DMA_InitStructure);

    //关闭DMA,清DMA标记,使能DMA1_CH2的传输完成中断
    DMA_Cmd(DMA2_Channel1, DISABLE);            //关闭发送DMA
    DMA_Cmd(DMA2_Channel2, DISABLE);            //关闭接收DMA
    DMA_ClearFlag(DMA2_FLAG_GL1|DMA2_FLAG_TC1|DMA2_FLAG_HT1|DMA2_FLAG_TE1);
    DMA_ClearFlag(DMA2_FLAG_GL2|DMA2_FLAG_TC2|DMA2_FLAG_HT2|DMA2_FLAG_TE2);
    DMA_ITConfig(DMA2_Channel1,DMA_IT_TC,ENABLE);

    //初始化SPI时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);


    // SPI配置
    SPI_Cmd(SPI3,DISABLE);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex ;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master ;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b ;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low ;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge ;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft ;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2 ;    //72MHz分频
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB ; //SPI设置成LSB模式
    SPI_InitStructure.SPI_CRCPolynomial = 7 ;
    SPI_Init( SPI3, &SPI_InitStructure ) ;
    SPI_Cmd(SPI3,ENABLE);           //启动SPI

    //打开SPI1的DMA发送接收请求
    SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Rx, ENABLE);
    SPI_I2S_DMACmd(SPI3, SPI_I2S_DMAReq_Tx, ENABLE);

    //清DMA忙信号
    sem_W25X_DMA_Busy = false;

    //使能NVIC中断
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = W25X_DMA_TC_PRIO;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/******************************************************************************
/ 函数功能:SPI发送一个字节的数据
/ 修改日期:2013/9/10 19:04:16
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
uint8_t W25X_ReadWriteByte(uint8_t dat) {
    while ((SPI3->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
    SPI3->DR = dat;
    while ((SPI3->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET);
    return (SPI3->DR);
}


/******************************************************************************
/ 函数功能:读取SPI_FLASH的状态寄存器
/ 修改日期:2013/9/10 20:38:48
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
/   BIT7  6   5   4   3   2   1   0
/   SPR   RV  TB BP2 BP1 BP0 WEL BUSY
/
/   SPR:默认0,状态寄存器保护位,配合WP使用
/   TB,BP2,BP1,BP0:FLASH区域写保护设置
/   WEL:写使能锁定, 1
/   BUSY:忙标记位(1,忙;0,空闲)
/   默认:0x00
******************************************************************************/
uint8_t W25X_ReadSR(void) {
    uint8_t byte=0;
    W25X_CS_L();                            //使能器件
    W25X_ReadWriteByte(W25X_ReadStatusReg); //发送读取状态寄存器命令
    byte=W25X_ReadWriteByte(0Xff);          //读取一个字节
    W25X_CS_H();                            //使能器件
    return byte;
}


/******************************************************************************
/ 函数功能:读取芯片ID
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:
/   返回值如下:
/   0XEF13,表示芯片型号为W25Q80
/   0XEF14,表示芯片型号为W25Q16
/   0XEF15,表示芯片型号为W25Q32
/   0XEF16,表示芯片型号为W25Q64
/ 使用说明:none
******************************************************************************/
uint16_t W25X_ReadID(void) {
    uint16_t Temp = 0;

    W25X_CS_L();
    W25X_ReadWriteByte(0x90);       //发送读取ID命令
    W25X_ReadWriteByte(0x00);
    W25X_ReadWriteByte(0x00);
    W25X_ReadWriteByte(0x00);
    Temp|=W25X_ReadWriteByte(0xFF)<<8;
    Temp|=W25X_ReadWriteByte(0xFF);
    W25X_CS_H();
    return Temp;
}


/******************************************************************************
/ 函数功能:等待芯片执行完毕
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Wait_Busy(void) {
    while((W25X_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}


/******************************************************************************
/ 函数功能:读取芯片看是否为忙
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
bool W25X_Read_BusyState(void) {
    if((W25X_ReadSR()&0x01))return true;
    else return false;
}


/******************************************************************************
/ 函数功能:SPI_FLASH写使能
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Write_Enable(void) {
    W25X_CS_L();                            //使能器件
    W25X_ReadWriteByte(W25X_WriteEnable);   //发送写使能
    W25X_CS_H();                            //取消片选
}


/******************************************************************************
/ 函数功能:SPI_FLASH写禁止
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Write_Disable(void) {
    W25X_CS_L();                            //使能器件
    W25X_ReadWriteByte(W25X_WriteDisable);  //发送写禁止指令
    W25X_CS_H();                            //取消片选
}


/******************************************************************************
/ 函数功能:使SPI FLASH掉电
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void SPI_Flash_PowerDown(void) {
    uint16_t i;

    W25X_CS_L();                        //使能器件
    W25X_ReadWriteByte(W25X_PowerDown); //发送掉电命令
    W25X_CS_H();                        //取消片选
    i= (72)*3;
    while(i--);               //等待约3us
}

/******************************************************************************
/ 函数功能:唤醒SPI FLASH
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void SPI_Flash_WakeUp(void) {
    uint16_t i;

    W25X_CS_L();                        //使能器件
    W25X_ReadWriteByte(W25X_ReleasePowerDown); //发送掉电命令
    W25X_CS_H();                        //取消片选
    i= (72)*3;
    while(i--);               //等待约3us
}


/******************************************************************************
/ 函数功能:擦除整个芯片 ,等待时间超长...
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Erase_Chip(bool bwait) {
    W25X_Write_Enable();            //SET WEL
    W25X_Wait_Busy();
    W25X_CS_L();                    //使能器件
    W25X_ReadWriteByte(W25X_ChipErase);   //发送片擦除命令
    W25X_CS_H();                    //取消片选
    if(bwait)W25X_Wait_Busy();      //等待芯片擦除结束
}


/******************************************************************************
/ 函数功能:擦除一个扇区
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:Dst_Addr:扇区地址 根据实际容量设置
/ 输出参数:none
/ 使用说明:none擦除一个扇区的最少时间:65ms
******************************************************************************/
void W25X_Erase_Sector(uint32_t Dst_Addr,bool bwait) {
    W25X_Write_Enable();            //SET WEL
    W25X_Wait_Busy();
    W25X_CS_L();                    //使能器件
    W25X_ReadWriteByte(W25X_SectorErase);           //发送扇区擦除指令
    W25X_ReadWriteByte((uint8_t)((Dst_Addr)>>16));  //发送24bit地址
    W25X_ReadWriteByte((uint8_t)((Dst_Addr)>>8));
    W25X_ReadWriteByte((uint8_t)Dst_Addr);
    W25X_CS_H();                    //取消片选
    if(bwait)W25X_Wait_Busy();      //等待擦除完成
}


/******************************************************************************
/ 函数功能:读出一页数据,无限制
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Read_Page(uint8_t * pBuffer,uint32_t PageAddr) {
    uint32_t    ReadAddr;
    uint16_t    i;

    if(PageAddr < W25X_PAGE_NUM) {
        ReadAddr = PageAddr *W25X_PAGE_SIZE;
        W25X_CS_L();
        W25X_ReadWriteByte(W25X_ReadData);         //发送读取命令
        W25X_ReadWriteByte((uint8_t)((ReadAddr)>>16));  //发送24bit地址
        W25X_ReadWriteByte((uint8_t)((ReadAddr)>>8));
        W25X_ReadWriteByte((uint8_t)ReadAddr);
        for(i=0; i<W25X_PAGE_SIZE; i++) {
            pBuffer[i]=W25X_ReadWriteByte(0XFF);   //循环读数
        }
        W25X_CS_H();
    }
}


/******************************************************************************
/ 函数功能:写一页数据到指定的页,必须该页已被擦除!!!
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Write_Page(uint8_t * pBuffer,uint32_t PageAddr) {
    uint16_t    i;
    uint32_t    WriteAddr;

    //打开写状态,并等待上次写操作完成
    W25X_Write_Enable();
    W25X_Wait_Busy();      //等待擦除完成

    //将数据写入FLASH
    WriteAddr =PageAddr* W25X_PAGE_SIZE;
    W25X_CS_L();
    W25X_ReadWriteByte(W25X_PageProgram);      //发送写页命令
    W25X_ReadWriteByte((uint8_t)((WriteAddr)>>16)); //发送24bit地址
    W25X_ReadWriteByte((uint8_t)((WriteAddr)>>8));
    W25X_ReadWriteByte((uint8_t)WriteAddr);
    for(i=0; i<W25X_PAGE_SIZE; i++)W25X_ReadWriteByte(pBuffer[i]); //循环写数
    W25X_CS_H();
    //W25X_Wait_Busy();      //等待擦除完成
}


/******************************************************************************
/ 函数功能:在指定地址开始读取指定长度的数据(最大64KB字节),连续读取模式
/ 修改日期:2013/9/10 20:43:20
/ 输入参数:
/   pBuffer:数据存储区
/   ReadAddr:开始读取的地址(24bit)
/   NumByteToRead:要读取的字节数(最大65535)
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Read_Data(uint8_t * pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead) {
    uint16_t i;
    W25X_CS_L();
    W25X_ReadWriteByte(W25X_ReadData);         //发送读取命令
    W25X_ReadWriteByte((uint8_t)((ReadAddr)>>16));  //发送24bit地址
    W25X_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    W25X_ReadWriteByte((uint8_t)ReadAddr);
    for(i=0; i<NumByteToRead; i++) {
        pBuffer[i]=W25X_ReadWriteByte(0XFF);   //循环读数
    }
    W25X_CS_H();
}


/******************************************************************************
/ 函数功能:DMA方式高效读取一批数据,小余64KB即可
/ 修改日期:2013/9/11 9:06:27
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_DMARead_Data(uint8_t * pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead) {
    //判断DMA是否仍处于工作状态,若是则等待传输完成,
    if( (DMA2_Channel1->CCR & DMA_CCR1_EN) || (DMA2_Channel1->CCR & DMA_CCR1_EN) ) {
        //while( !DMA_GetFlagStatus(DMA1_FLAG_TC2));//高效写法如下
        while( !(DMA2->ISR & DMA2_FLAG_TC1));       //等待传送完成
        sem_W25X_DMA_RxRdy  = true;                 //标记DMA接收数据信号
        W25X_CS_H();                                //结束片选
        __NOP();
        __NOP();
        __NOP();
        __NOP();            //短延时,使CS有足够的拉高时间
    }

    //设置DMA数据载荷,并清DMA标记
    sem_W25X_DMA_Busy = true;               //标记为DMA忙
    DMA2_Channel2->CMAR = (uint32_t)(&W25X_TX_Byte);    //设置发送数据的源SRAM地址
    DMA2_Channel2->CNDTR= NumByteToRead;    //设置发送字节长度,发送SRAM地址不增加
    DMA2_Channel1->CMAR =(uint32_t)pBuffer; //设置接收数据个数
    DMA2_Channel1->CNDTR= NumByteToRead;    //设置接收数据的目标SRAM地址

    //发送前导字节
    W25X_CS_L();
    W25X_ReadWriteByte(W25X_ReadData);         //发送读取命令
    W25X_ReadWriteByte((uint8_t)((ReadAddr)>>16));  //发送24bit地址
    W25X_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    W25X_ReadWriteByte((uint8_t)ReadAddr);
    SPI3->DR ;                                //接送前读一次SPI1->DR，保证接收缓冲区为空

    //清DMA标记
    DMA_ClearFlag(DMA2_FLAG_GL2|DMA2_FLAG_TC2|DMA2_FLAG_HT2|DMA2_FLAG_TE2);
    DMA_ClearFlag(DMA2_FLAG_GL1|DMA2_FLAG_TC1|DMA2_FLAG_HT1|DMA2_FLAG_TE1);


    //启动DMA发送数据
    while ((SPI3->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
    DMA_Cmd(DMA2_Channel2, ENABLE);
    DMA_Cmd(DMA2_Channel1, ENABLE);


    //等待DMA传送数据完毕
/*
    while( !DMA_GetFlagStatus(DMA1_FLAG_TC2));  //等待接收DMA的传输完成
    DMA_Cmd(DMA1_Channel3, DISABLE);            //关闭发送DMA
    DMA_Cmd(DMA1_Channel2, DISABLE);            //关闭接收DMA
    sem_W25X_DMA_RxRdy = true;                  //标记DMA接收数据信号
    W25X_CS_H();
*/

}


/******************************************************************************
/ 函数功能:读唯一ID号,8个字节
/ 修改日期:2013/9/11 9:38:49
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void W25X_Read_UID(uint8_t * pBuffer) {
    uint8_t i;

    W25X_CS_L();
    W25X_ReadWriteByte(W25X_ReadUniqueID);
    W25X_ReadWriteByte(0x00);
    W25X_ReadWriteByte(0x00);
    W25X_ReadWriteByte(0x00);
    W25X_ReadWriteByte(0x00);
    for(i=0; i<8; i++)
        pBuffer[i]=W25X_ReadWriteByte(0XFF);   //循环读数
    W25X_CS_H();
}


/******************************************************************************
/ 函数功能:DMA数据接收完毕中断
/ 修改日期:2013/9/11 9:38:47
/ 输入参数:none
/ 输出参数:none
/ 使用说明:none
******************************************************************************/
void DMA2_Channel1_IRQHandler(void) {
    //空读ISR状态
    DMA2->ISR;

    //关闭DMA通道
    //DMA_Cmd(DMA1_Channel2, DISABLE);//以下为等效写法
    //DMA_Cmd(DMA1_Channel3, DISABLE);//以下为等效写法
    DMA2_Channel1->CCR &= ~DMA_CCR1_EN;     //关闭DMA1_CH2
    DMA2_Channel2->CCR &= ~DMA_CCR1_EN;     //关闭DMA1_CH2

    //清DMA中断标记
    //DMA_ClearITPendingBit(DMA1_IT_GL2|DMA1_IT_TC2|DMA1_IT_HT2|DMA1_IT_TE2);//以下为等待模式
    DMA2->IFCR = DMA2_IT_GL1|DMA2_IT_TC1|DMA2_IT_HT1|DMA2_IT_TE1;


    //置信号量
    DMA_Cmd(DMA2_Channel2, DISABLE);            //关闭发送DMA
    DMA_Cmd(DMA2_Channel1, DISABLE);            //关闭接收DMA
    sem_W25X_DMA_Busy   = false;                //标记为DMA空闲
    sem_W25X_DMA_RxRdy  = true;                 //标记DMA接收数据信号
    W25X_CS_H();                                //结束片选
}
