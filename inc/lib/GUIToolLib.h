#ifndef GUITOOLLIB_H_INCLUDED
#define GUITOOLLIB_H_INCLUDED
#include <stm32f10x.h>


typedef struct {
    uint8_t magic[4];      // 文件头
    uint32_t dwFileSize;   // 文件大小
    uint8_t nSection;      // 段信息数量
    uint8_t nYSize;         // 字体高度
    uint16_t wCpFlag;      // codepageflag:  每一位表示一个codepage
    uint16_t nTotalChars;  // 字符总数
    char reserved[2];      // 保留字节
} GuiFontHeader_t;
typedef struct {
    uint16_t first;       // 段起始码
    uint16_t last;        // 段结束码
    uint32_t offsetAddr;  // GUI_FONT_INDEX首字节的偏移地址
} GuiFontSection_t;

typedef struct {
    uint8_t     width;       // 宽度
    uint32_t    offsetAddr;  // 点阵数据起始偏移地址
} GuiFontIndex_t;

typedef struct {
    GuiFontHeader_t header;
    GuiFontSection_t* sections;
} GuiFontCache_t;
typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t* bitmap;
} GuiFont_t;
void GUITool_GetHeader(const uint32_t fontAddr,GuiFontHeader_t* pHeader);
void GUITool_GetSection(const uint32_t fontAddr,uint32_t unicode,GuiFontSection_t* pSection);
void GUITool_GetIndex(GuiFontSection_t* pSection,uint32_t unicode,GuiFontIndex_t* pIndex);
void GUITool_GetFontData(const uint32_t fontAddr,uint16_t unicode,uint8_t* pBuffer);
#endif /* GUITOOLLIB_H_INCLUDED */
