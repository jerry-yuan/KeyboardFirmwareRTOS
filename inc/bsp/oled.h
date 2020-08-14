
#ifndef OLED_H_INCLUDED
#define OLED_H_INCLUDED
#include <stm32f10x.h>
#include <SGUI_Typedef.h>
/**
 * OLED屏幕接线
 *  OLED       MCU
 *    DC <---> PA2 (GPIO)
 * RESET <---> PA3 (GPIO)
 *    CS <---> PA4 (GPIO)
 *   CLK <---> PA5 (SPI1-CLK)
 *    NC <---> PA6 (SPI1-MISO)
 *  MOSI <---> PA7 (SPI1-MOSI)
 *
 */

#define OLED_DC_PIN GPIO_Pin_2
#define OLED_RESET_PIN GPIO_Pin_3
#define OLED_CS_PIN GPIO_Pin_4
#define OLED_CLK_PIN GPIO_Pin_5
#define OLED_MISO_PIN GPIO_Pin_6
#define OLED_MOSI_PIN GPIO_Pin_7

#define OLED_SCREEN_WIDTH 256
#define OLED_SCREEN_HEIGHT 64
#define OLED_PIXEL_PER_BYTE 2
#define OLED_FRAMEBUFFER_SIZE   sizeof(uint8_t)*OLED_SCREEN_WIDTH*OLED_SCREEN_HEIGHT/OLED_PIXEL_PER_BYTE

#define OLED_DispString         OLED_DispString_Deng12


typedef enum {
    OLED_COMMAND=0,
    OLED_DISPLAY=1
} BufferType;

void OLED_Initialize();
void OLED_SendBuffer(BufferType type,const uint8_t* buff,uint32_t len);
void OLED_DMAInitialize();
void OLED_DMADeInitialize();
void OLED_Clear();
void OLED_SetPixel(SGUI_INT x,SGUI_INT y,SGUI_COLOR color);
SGUI_COLOR OLED_GetPixel(SGUI_INT x,SGUI_INT y);
void OLED_DrawRect(uint8_t x,uint8_t y,uint8_t w,uint8_t h,uint8_t borderColor,uint8_t fillColor);

void OLED_DispString_GuiToolFont(uint32_t fontAddr,uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,const char* pStr) ;
void OLED_DispString_Deng12(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,const char* pStr);
/*
void OLED_DispChar_GB2312(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,uint16_t ch);
void OLED_DispChar_ASCII(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,uint8_t ch);
void OLED_DispString_GBK(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,const char* pStr);
void OLED_DispString_UTF8(uint8_t x,uint8_t y,uint8_t frColor,uint8_t bgColor,const char* pStr);
*/
extern uint8_t* oledFramebuffer;
extern SGUI_SCR_DEV* screen;
#endif /* OLED_H_INCLUDED */

