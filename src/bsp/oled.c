#include <oled.h>

#include <string.h>
#include <stdlib.h>
#include <FreeRTOS.h>

#include <delay.h>
#include <w25q64.h>

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
DMA_InitTypeDef oledDMAProps= {
    (uint32_t)&SPI1->DR,        /*DMA_PeripheralBaseAddr*/
    0,                          /*DMA_MemoryBaseAddr*/
    DMA_DIR_PeripheralDST,      /*DMA_DIR*/
    0,                          /*DMA_BufferSize*/
    DMA_PeripheralInc_Disable,  /*DMA_PeripheralInc*/
    DMA_MemoryInc_Enable,       /*DMA_MemoryInc;*/
    DMA_PeripheralDataSize_Byte,/*DMA_PeripheralDataSize*/
    DMA_MemoryDataSize_Byte,    /*DMA_MemoryDataSize*/
    DMA_Mode_Circular,          /*DMA_Mode*/
    DMA_Priority_VeryHigh,      /*DMA_Priority*/
    DMA_M2M_Disable             /*DMA_M2M*/
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
    //启动FrameBuffer自动刷新机制
    oledDMAProps.DMA_MemoryBaseAddr=(uint32_t)oledFramebuffer;
    oledDMAProps.DMA_BufferSize=OLED_FRAMEBUFFER_SIZE;
    oledDMAProps.DMA_Mode=DMA_Mode_Circular;
    GPIO_SetBits(GPIOA,OLED_DC_PIN);
    OLED_DMAInitialize();
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
    screen->fnSyncBuffer= __NOP;
}
void OLED_SendBuffer(BufferType type,const uint8_t* buff,uint32_t len) {
    DMA_InitTypeDef temp;
    uint16_t dmaCounter;
    uint8_t dcState;
    //备份状态
    memcpy(&temp,&oledDMAProps,sizeof(DMA_InitTypeDef));
    dmaCounter=DMA_GetCurrDataCounter(DMA1_Channel3);
    dcState=GPIO_ReadInputDataBit(GPIOA,OLED_DC_PIN);
    // 停止Framebuffer同步并销毁配置
    if(oledDMAProps.DMA_MemoryBaseAddr!=0) {
        OLED_DMADeInitialize();
    }
    /** 开始传输Buff相关配置 **/
    //调整数据/命令线电平
    GPIO_WriteBit(GPIOA,OLED_DC_PIN,type);

    //设置oledDMAProps
    oledDMAProps.DMA_MemoryBaseAddr=(uint32_t)buff;
    oledDMAProps.DMA_BufferSize=len;
    oledDMAProps.DMA_Mode=DMA_Mode_Normal;
    //重新初始化
    OLED_DMAInitialize();
    //启动传输
    SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE);
    //等待传输结束
    while(DMA_GetFlagStatus(DMA1_FLAG_TC3)==RESET);
    OLED_DMADeInitialize();
    /** 恢复现场 **/
    memcpy(&oledDMAProps,&temp,sizeof(DMA_InitTypeDef));
    if(oledDMAProps.DMA_MemoryBaseAddr!=0) {
        OLED_DMAInitialize();
        if(dmaCounter!=0) {
            DMA_SetCurrDataCounter(DMA1_Channel3,dmaCounter);
        }
        GPIO_WriteBit(GPIOA,OLED_DC_PIN,dcState);
    }
}
void OLED_DMAInitialize() {
    // 初始化DMA
    DMA_Init(DMA1_Channel3,&oledDMAProps);
    // 使能DMA
    DMA_Cmd(DMA1_Channel3,ENABLE);
}
void OLED_DMADeInitialize() {
    //关掉DMA
    DMA_Cmd(DMA1_Channel3,DISABLE);
    // 销毁DMA配置
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
    y=y&0x3F;
    return (*(oledFramebuffer+(y&0x3F)*OLED_SCREEN_WIDTH+x/OLED_PIXEL_PER_BYTE)>>(x%2?0:4))&0x0F;
    //return (oledFramebuffer[y&0x3F][x/2]>>(x%2?0:4))&0x0F;
}
/*
void OLED_DrawRect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t borderColor,uint8_t fillColor) {
    //绘制框子
    // 上边框
    for(int i=1; i<w; i++) {
        OLED_SetPixel(x+i,y,borderColor);
    }
    //右边框
    for(int i=1; i<h; i++) {
        OLED_SetPixel(x+w-1,y+i,borderColor);
    }
    //下边框
    for(int i=0; i<w-1; i++) {
        OLED_SetPixel(x+i,y+h-1,borderColor);
    }
    //左边框
    for(int i=0; i<h-1; i++) {
        OLED_SetPixel(x,y+i,borderColor);
    }
    //绘制填充
    for(int i=0; i<w-2; i++) {
        for(int j=0; j<h-2; j++) {
            OLED_SetPixel(x+i+1,y+1+j,fillColor);
        }
    }
}

void OLED_DispString_Deng12(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,const char* pStr) {
    OLED_DispString_GuiToolFont(FLASH_ADDR_DENGFONT_12,x,y,frColor,bgColor,pStr);
}


void OLED_DispString_GuiToolFont(uint32_t fontAddr,uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,const char* pStr) {
    GuiFont_t font;
    uint32_t unicode=0x0;
    uint8_t* pFont=NULL;
    uint8_t  byteMask;
    while((*pStr)!='\0') {
        pStr=SGUI_Text_StepNext_UTF8(pStr,&unicode);
        GUITool_GetFont(fontAddr,unicode,&font);
        pFont=font.bitmap;
        for(uint8_t i=0; i<font.height; i++) {
            byteMask=0x80;
            for(uint8_t j=0; j<font.width; j++) {
                if (byteMask == 0x00) {
                    byteMask = 0x80;
                    pFont++;
                }
                if(*pFont & byteMask) {
                    OLED_SetPixel(x+j,y+i,frColor);
                } else {
                    OLED_SetPixel(x+j,y+i,bgColor);
                }
                byteMask >>= 1;
            }
            pFont++;
        }
        x+=font.width;
        vPortFree(font.bitmap);
    }
}

void OLED_DispChar_GB2312(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,uint16_t ch) {
    uint8_t ucBuffer[32];
    uint16_t usTemp;
    // 读取字符数据
    uint8_t h8Bit=ch>>8;
    uint8_t l8Bit=ch&0xFF;
    uint32_t GBKAddrOffset=((h8Bit-0xA1)*94+l8Bit-0xA1)*32;
    W25X_Read_Data(ucBuffer,FLASH_ADDR_GBKFONT+GBKAddrOffset,32);
    // 输出至屏幕
    for(int i=0; i<16; i++) {
        usTemp = ((uint16_t)ucBuffer[i*2]<<8)|ucBuffer[i*2+1];
        for(int j=0; j<16; j++) {
            if(usTemp & (0x8000>>j)) {
                OLED_SetPixel(x+j,y+i,frColor);
            } else {
                OLED_SetPixel(x+j,y+i,bgColor);
            }
        }
    }

}
void OLED_DispChar_ASCII(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,uint8_t ch) {
    uint8_t cBuffer[16];
    // 读取字符数据
    W25X_Read_Data(cBuffer,FLASH_ADDR_ASCIIFONT+ch*16,16);
    // 输出至屏幕
    for(int i=0; i<16; i++) {
        for(int j=0; j<8; j++) {
            if(cBuffer[i] & (0x80>>j)) {
                OLED_SetPixel(x+j,y+i,frColor);
            } else {
                OLED_SetPixel(x+j,y+i,bgColor);
            }
        }
    }
}
void OLED_DispString_UTF8(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,const char* pStr) {
    uint32_t unicode;
    uint16_t gbk;
    uint8_t temp,bytesPerChar;
    while(*pStr!='\0') {
        if((*pStr & 0x80) == 0x00) {
            // 单字节字符:英文字符
            OLED_DispChar_ASCII(x,y,frColor,bgColor,*pStr++);
            x+=8;
        } else {
            // 多字节字符
            // 取出第一个字节
            temp=*pStr;
            bytesPerChar=0;
            unicode=0x00000000;
            while(temp&0x80) {
                bytesPerChar++;
                temp<<=1;
            }
            unicode=(*pStr++) & ((1<<(8-bytesPerChar))-1);
            for(uint8_t i=1; i<bytesPerChar; i++) {
                unicode = unicode<<6 | ((*pStr++)&0x3F);
            }
            if(unicode>0x4E00 && unicode < 0x9FA5) {
                W25X_Read_Data((uint8_t*)&gbk,FLASH_ADDR_UTF8_GBK_TABLE+2*(unicode-0x4e00),2);
                OLED_DispChar_GB2312(x,y,frColor,bgColor,gbk);
            } else {
                OLED_DispChar_ASCII(x,y,frColor,bgColor,'?');
                OLED_DispChar_ASCII(x+8,y,frColor,bgColor,'?');
            }
            x+=16;
        }
    }
}
void OLED_DispString_GBK(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,const char* pStr) {
    while(*pStr!= '\0') {
        if(*pStr<=126) {
            // 英文字符
            OLED_DispChar_ASCII(x,y,frColor,bgColor,*pStr);
            x+=8;
            pStr++;
        } else {
            // 汉字字符
            uint16_t usCh=*(uint16_t*)pStr;
            usCh=(usCh<<8)+(usCh>>8);
            OLED_DispChar_GB2312(x,y,frColor,bgColor,usCh);
            x+=16;
            pStr+=2;
        }
    }
}

*/
