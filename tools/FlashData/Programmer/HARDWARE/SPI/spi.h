#ifndef __SPI_H
#define __SPI_H
#include "sys.h"

#include "platform_config.h"

void SPI_IO_Init(void);			 //³õÊ¼»¯SPI¿Ú


#define USE_SPI3

#define SPI_SC_PORT						GPIOA
#define SPI_SC_PIN						GPIO_Pin_15//

#define SPI_BUS_USED         SPI3
//#define SPI_ENGINE_RCC       RCC_APB1Periph_SPI2
#define SPI_DEFAULT_SPEED    9000000              // Default SPI clock = 9MHz to support most chips.
#define SPI_DR_Base          (&(SPI_BUS_USED->DR))
#define SPI_TX_DMA_CH        DMA2_Channel2         // SPI1 TX is only available on DMA1 CH5 
#define SPI_TX_DMA_FLAG      DMA2_FLAG_TC2
#define SPI_RX_DMA_CH        DMA2_Channel1         // SPI1 RX is only available on DMA1 CH4 
#define SPI_RX_DMA_FLAG      DMA2_FLAG_TC1
#define select_chip() GPIO_ResetBits(SPI_SC_PORT,SPI_SC_PIN)
#define unselect_chip() GPIO_SetBits(SPI_SC_PORT,SPI_SC_PIN)
/*
#ifdef USE_STLINK_V2_AS_SERPROG1


/ * SPI * /

//#error "test"




#define USE_SPI2

#define SPI_SC_PORT						GPIOB
#define SPI_SC_PIN						GPIO_Pin_6//

#define SPI_BUS_USED         SPI2
//#define SPI_ENGINE_RCC       RCC_APB1Periph_SPI2
#define SPI_DEFAULT_SPEED    9000000              // Default SPI clock = 9MHz to support most chips.
#define SPI_DR_Base          (&(SPI_BUS_USED->DR))
#define SPI_TX_DMA_CH        DMA1_Channel5         // SPI1 TX is only available on DMA1 CH5 
#define SPI_TX_DMA_FLAG      DMA1_FLAG_TC5
#define SPI_RX_DMA_CH        DMA1_Channel4         // SPI1 RX is only available on DMA1 CH4 
#define SPI_RX_DMA_FLAG      DMA1_FLAG_TC4
#define select_chip() GPIO_ResetBits(SPI_SC_PORT,SPI_SC_PIN)
#define unselect_chip() GPIO_SetBits(SPI_SC_PORT,SPI_SC_PIN)



#else  //USE_STLINK_V2_AS_SERPROG


#define USE_SPI1

#define SPI_SC_PORT						GPIOA
#define SPI_SC_PIN						GPIO_Pin_4

#define SPI_BUS_USED         SPI1
//#define SPI_ENGINE_RCC       RCC_APB2Periph_SPI1
#define SPI_DEFAULT_SPEED    9000000              // Default SPI clock = 9MHz to support most chips.
#define SPI_DR_Base          (&(SPI_BUS_USED->DR))
#define SPI_TX_DMA_CH        DMA1_Channel3         // SPI1 TX is only available on DMA1 CH3 
#define SPI_TX_DMA_FLAG      DMA1_FLAG_TC3
#define SPI_RX_DMA_CH        DMA1_Channel2         // SPI1 RX is only available on DMA1 CH2 
#define SPI_RX_DMA_FLAG      DMA1_FLAG_TC2
#define select_chip() GPIO_ResetBits(SPI_SC_PORT,SPI_SC_PIN)
#define unselect_chip() GPIO_SetBits(SPI_SC_PORT,SPI_SC_PIN)

#endif //end USE_STLINK_V2_AS_SERPROG
*/


uint32_t spi_conf(uint32_t speed_hz);
void spi_bulk_write(uint32_t size);
void spi_bulk_read(uint32_t size);
void DMA_configuration(void);


		 
#endif

