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
    OLED.GPIO_Pin=OLED_PIN_DC | OLED_PIN_RESET;
    OLED.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA,&OLED);
    // 初始化 SPI1 的 CLK MOSI MISO
    OLED.GPIO_Pin=OLED_PIN_CLK|OLED_PIN_MOSI|OLED_PIN_MISO|OLED_PIN_CS;
    OLED.GPIO_Mode=GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA,&OLED);

    // 临时禁用OLED屏幕
    GPIO_SetBits(GPIOA,OLED_PIN_CS);

    // 使能SPI1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    SPI_InitTypeDef SPI;

    SPI.SPI_Direction				= SPI_Direction_1Line_Tx;	// 单线发送
    SPI.SPI_Mode					= SPI_Mode_Master;			// 主机
    SPI.SPI_DataSize				= SPI_DataSize_8b;			// 一次发送8bit
    SPI.SPI_CPOL					= SPI_CPOL_High;			// 空闲时拉高
    SPI.SPI_CPHA					= SPI_CPHA_2Edge;			// 上升沿采样
    SPI.SPI_NSS						= SPI_NSS_Hard;				// 硬件控制CS
    SPI.SPI_BaudRatePrescaler		= SPI_BaudRatePrescaler_2;	// 2分频
    SPI.SPI_FirstBit				= SPI_FirstBit_MSB;			// 高位先行
    SPI.SPI_CRCPolynomial			= 7;						// CRC校验
    // 启用硬件CS
    SPI_SSOutputCmd(SPI1,ENABLE);
    // 初始化SPI1
    SPI_Init(SPI1,&SPI);
    // 使能SPI1
    SPI_Cmd(SPI1,ENABLE);
    /**
     * 初始化DMA
     */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
    DMA_InitTypeDef DMA;

    DMA.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
    DMA.DMA_MemoryBaseAddr     = (uint32_t)0;
    DMA.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA.DMA_BufferSize         = 0;
    DMA.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA.DMA_Mode               = DMA_Mode_Normal;
    DMA.DMA_Priority           = DMA_Priority_VeryHigh;
    DMA.DMA_M2M                = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel3,&DMA);

    // 启用SPI1的DMA请求
    SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);

    /**
     * 初始化OLED屏幕
     */
    // 重置OLED屏幕
    GPIO_ResetBits(GPIOA,OLED_PIN_RESET);   // 拉低RESET重启屏幕
    Delay_us(1);                            // 等待1uS
    GPIO_SetBits(GPIOA,OLED_PIN_RESET);     // 拉高RESET屏幕开始运行
    // 执行初始化指令
    OLED_SendBuffer(OLED_COMMAND,oledInit,sizeof(oledInit));

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

    // 清屏
    OLED_Clear();
    OLED_SyncBuffer();
}
void OLED_SendBuffer(BufferType type,const uint8_t* buffer,uint32_t length) {
    // 调整DC线电平
    GPIO_WriteBit(GPIOA,OLED_PIN_DC,type);

    // 清除中断
    DMA_ClearITPendingBit(DMA1_IT_GL3);
    DMA_ClearFlag(DMA1_FLAG_GL3);

    // 设置DMA参数
    DMA1_Channel3->CNDTR	= (uint32_t)length;
    DMA1_Channel3->CMAR		= (uint32_t)buffer;

    //启动DMA
    DMA_Cmd(DMA1_Channel3,ENABLE);

    //等待结束
    while(DMA_GetFlagStatus(DMA1_FLAG_TC3)==RESET) {
        if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
            portYIELD();
        }
    }

    // 关掉DMA
    DMA_Cmd(DMA1_Channel3,DISABLE);
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
    if(x&0x01) {
        // 奇数列 ==> 取原有数据低四位
        return *origin & 0x0F;
    } else {
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
