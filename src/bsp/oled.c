#include <oled.h>

#include <string.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>

#include <delay.h>
#include <w25x.h>

#include <lib/GUIToolLib.h>

uint8_t* oledFramebuffer;//[64][128]= {};
const uint8_t oledInit[]= {
    0xAE,       //关闭显示:Set Display Off
    0xB0,0x00,  //设定行地址为0:Row address Mode Setting
    0x10,       //设定列地址高四位为0:Set Higher Column Address of display RAM
    0x00,       //设定列地址第四位为0:Set Lower Column Address of display RAM
    0xD5,0x50,  //设定显示时钟分频为3分频/晶振时钟为+5%/FPS=125:Set Display Clock Divide Ratio/Oscillator Frequency
    0xD9,0x22,  //设定预充电阈值Set Discharge/Precharge Period
    0x40,       //设定显示起始行为0:Set Display Start Line
    0x81,0xFF,  //设定对比度为0xFF:The Contrast Control Mode Set
    0xA0,       //设定段重映射(左右翻转)为不启用:Set Segment Re-map
    0xC0,       //设定扫描方向(上下翻转)为正向:Set Common Output Scan Direction
    0xA4,       //设定全局显示开关为开:Set Entire Display OFF/ON
    0xA6,       //设定为正常显示模式:Set Normal/Reverse Display
    0xA8,0x3F,  //设定扫描行为从0~64:Set Multiplex Ration
    0xAD,0x80,  //设定DC-DC转换器设置为休眠模式:DC-DC Setting
    0xD3,   //Set Display Offset
    0x00,
    0xDB,   //Set VCOM Deselect Level
    0x30,
    0xDC,   //Set VSEGM Level
    0x30,
    0x33,   //Set Discharge VSL Level 1.8V
    0xAF    //Set Display On
};
SGUI_SCR_DEV* screen;
/**
 * OLED接线图
 *  OLED       MCU
 *    DC <---> PA2 (GPIO)
 * RESET <---> PA3 (GPIO)
 *    CS <---> PA4 (GPIO)
 *   CLK <---> PA5 (SPI1-CLK)
 *    NC <---> PA6 (SPI1-MISO)
 *  MOSI <---> PA7 (SPI1-MOSI)
 *
 */
void OLED_Initialize() {
    oledFramebuffer=pvPortMalloc(OLED_FRAMEBUFFER_SIZE);
    memset(oledFramebuffer,0x00,OLED_FRAMEBUFFER_SIZE);
    /**
     * 初始化接口
     */
    GPIO_InitTypeDef OLED;
    OLED.GPIO_Speed=GPIO_Speed_50MHz;
    // 使能GPIO引脚的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);
    // 初始化 DC RESET CS
    OLED.GPIO_Pin=OLED_DC_PIN | OLED_RESET_PIN |OLED_CS_PIN;
    OLED.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA,&OLED);
    // 初始化 SPI1 的 CLK MOSI MISO
    OLED.GPIO_Pin=OLED_CLK_PIN|OLED_MOSI_PIN|OLED_MISO_PIN;
    OLED.GPIO_Mode=GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA,&OLED);

    // 临时禁用OLED屏幕
    GPIO_SetBits(GPIOA,OLED_CS_PIN);

    // 使能SPI1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    SPI_InitTypeDef SPI;

    SPI.SPI_Direction=SPI_Direction_2Lines_FullDuplex;  // 单线发送
    SPI.SPI_Mode=SPI_Mode_Master;                       // 主机
    SPI.SPI_DataSize=SPI_DataSize_8b;                   // 一次发送8bit
    SPI.SPI_CPOL=SPI_CPOL_High;                         // 空闲时拉高
    SPI.SPI_CPHA=SPI_CPHA_2Edge;                        // 上升沿采样
    SPI.SPI_NSS=SPI_NSS_Soft;                           // 硬件控制CS
    SPI.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;  // 2分频
    SPI.SPI_FirstBit=SPI_FirstBit_MSB;                  // 高位先行
    SPI.SPI_CRCPolynomial=7;                            // CRC校验
    // 初始化SPI1
    SPI_Init(SPI1,&SPI);
    // 使能SPI1
    SPI_Cmd(SPI1,ENABLE);
    /**
     */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);


    /**
     * 初始化OLED屏幕
     */
    GPIO_ResetBits(GPIOA,OLED_RESET_PIN);   // 拉低RESET重启屏幕
    Delay_us(1);                            // 等待1uS
    GPIO_SetBits(GPIOA,OLED_RESET_PIN);     // 拉高RESET屏幕开始运行
    GPIO_ResetBits(GPIOA,OLED_CS_PIN);
    OLED_SendBuffer(OLED_COMMAND,oledInit,sizeof(oledInit));

    SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
    //初始化SimpleGUI设备指针
    screen = pvPortMalloc(sizeof(SGUI_SCR_DEV));
    memset(screen,0x00,sizeof(SGUI_SCR_DEV));
    screen->stSize.iWidth=256;
    screen->stSize.iHeight=64;
    screen->uiDepthBits = 4;
    screen->fnSetPixel  = OLED_SetPixel;
    screen->fnGetPixel  = OLED_GetPixel;
    screen->fnClear     = OLED_Clear;
    screen->fnSyncBuffer= OLED_SyncBuffer;
}
void OLED_SendBuffer(BufferType type,const uint8_t* buff,uint32_t len) {
    DMA_InitTypeDef init;

    init.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
    init.DMA_MemoryBaseAddr     = (uint32_t)buff;
    init.DMA_DIR                = DMA_DIR_PeripheralDST;
    init.DMA_BufferSize         = len;
    init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    init.DMA_Mode               = DMA_Mode_Normal;
    init.DMA_Priority           = DMA_Priority_VeryHigh;
    init.DMA_M2M                = DMA_M2M_Disable;

    //调整数据/命令线电平
    GPIO_WriteBit(GPIOA,OLED_DC_PIN,type);

    DMA_Init(DMA1_Channel3,&init);
    DMA_Cmd(DMA1_Channel3,ENABLE);
    SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
    //等待传输结束
    while(DMA_GetFlagStatus(DMA1_FLAG_TC3)==RESET) {
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            portYIELD();
        }
    }
    DMA_Cmd(DMA1_Channel3,DISABLE);
    DMA_DeInit(DMA1_Channel3);
}
void OLED_Clear() {
    memset(oledFramebuffer,0x00,OLED_FRAMEBUFFER_SIZE);
}
void OLED_SetPixel(SGUI_INT x,SGUI_INT y,SGUI_COLOR color) {
    y=y&0x3F;
    uint8_t* origin=oledFramebuffer+y*OLED_SCREEN_WIDTH/OLED_PIXEL_PER_BYTE+x/OLED_PIXEL_PER_BYTE;
    if(x&0x01) {
        // 奇数列  ==> 取原有数据高四位和新数据低四位作为新数据
        *origin=(*origin & 0xF0) | (color & 0x0F);// 取原有数据的前4位和color的后四位
    } else {
        // 偶数列  ==> 取原有数据后四位和新数据作为高四位
        *origin=(*origin & 0x0F) | ((color<<4)&0xF0) ;
    }

}
SGUI_COLOR OLED_GetPixel(SGUI_INT x,SGUI_INT y) {
	y = y&0x3F;
	uint8_t* origin = oledFramebuffer+y*OLED_SCREEN_WIDTH/OLED_PIXEL_PER_BYTE+x/OLED_PIXEL_PER_BYTE;
	if(x&0x01){
		// 奇数列 ==> 取原有数据低四位
		return *origin & 0x0F;
	}else{
		// 偶数列 ==> 取原有数据高四位
		return (*origin & 0xF0)>>4;
	}
}
void OLED_SyncBuffer() {
    OLED_SendBuffer(OLED_DISPLAY,oledFramebuffer,OLED_FRAMEBUFFER_SIZE);
}
void OLED_SetDisplayState(bool display) {
    uint8_t command=display?0xAF:0xAE;
    OLED_SendBuffer(OLED_COMMAND,&command,sizeof(command));
}
