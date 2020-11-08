#ifndef GUITOOLLIB_H_INCLUDED
#define GUITOOLLIB_H_INCLUDED
#include <stm32f10x.h>
#include <SGUI_Text.h>
#include <fatfs/ff.h>
typedef struct {
    uint8_t uiYSize;         // 字体高度
    uint8_t uiDepthBits;    // 深度
    uint8_t ucReserved[2];    // 保留字节
    uint32_t pSearchTreeArea;  // 搜索树根节点位置
    uint32_t pCharInfoArea;    // 字符属性区域位置
    uint32_t pDataArea;        // 字符位图区域位置
} GUI_FONT_HEADER;
typedef struct{
    uint16_t uiFirst;         // 段起始码
    uint16_t uiMiddle;           // 段中心码
    uint16_t uiLast;          // 段结束码
    uint16_t uiReserved;    // 保留字节
    uint32_t pCharInfoAddr; // 字符信息起始偏移
    uint32_t pLeftChild;          // 左子树
    uint32_t pRightChild;         // 右子树
} GUI_FONT_SECTION;
typedef struct {
    uint8_t uiXSize;
    uint8_t uiYSize;
    int8_t uiXPos;
    int8_t uiYPos;
    uint32_t uiOffsetAddr;
} GUI_FONT_CHARINFO;

void GUITool_ReadBitmap(SGUI_BMP_RES* pstBitmap,uint16_t uiCode,FIL* pstFontFile);

#endif /* GUITOOLLIB_H_INCLUDED */
