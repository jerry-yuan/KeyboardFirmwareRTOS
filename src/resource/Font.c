#include <resources/Font.h>
#include <SGUI_Basic.h>
#include <SGUI_Text.h>
#include <bsp/W25Q64.h>
#include <lib/GUIToolLib.h>
static void GUITool_Deng12_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun);

const SGUI_FONT_RES SGUI_FONT(Deng12)={
    12,1,GUITool_Deng12_GetBitmap,SGUI_TEXT_DECODER_UTF8
};

static void GUITool_Deng12_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun){
    GuiFontHeader_t header;
    GuiFontSection_t section;
    GuiFontIndex_t index;
    uint32_t offsetAddr,rawFontIndex,fontAddr=FLASH_ADDR_DENGFONT_12;
    uint8_t i;
    // 读取FontHeader
    W25X_Read_Data((uint8_t*)&header,fontAddr,sizeof(header));
    // 查找section
    offsetAddr=sizeof(header);
    for(i=0; i<header.nSection; i++) {
        W25X_Read_Data((uint8_t*)&section,fontAddr+offsetAddr,sizeof(section));
        if(section.first<=uiCode && section.last>=uiCode) {
            break;
        }
        offsetAddr+=sizeof(section);
    }
    if(i>=header.nSection) {
        GUITool_Deng12_GetBitmap(pstBitmap,'?',bDryRun);
        return;
    }
    // 读取FontIndex
    offsetAddr=section.offsetAddr+sizeof(uint32_t)*(uiCode-section.first);
    W25X_Read_Data((uint8_t*)&rawFontIndex,fontAddr+offsetAddr,sizeof(uint32_t));
    index.width         = (rawFontIndex & 0xFC000000) >> 26;
    index.offsetAddr    = (rawFontIndex & 0x03FFFFFF);
    // 读取Bitmap数据
    pstBitmap->iWidth=index.width;
    pstBitmap->iHeight=header.nYSize;
    pstBitmap->iDepthBits=1;
        pstBitmap->fnGetPixel=SGUI_BMP_SCAN_MODE_DHPV;
    if(!bDryRun){
        uint8_t fontBitmapSize=header.nYSize * ((index.width + 8 - 1) / 8);
        W25X_Read_Data((uint8_t*)pstBitmap->pData,fontAddr+index.offsetAddr,fontBitmapSize);
    }

}
