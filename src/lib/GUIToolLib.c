#include<stdio.h>
#include<GUIToolLib.h>
#include<bsp/w25q64.h>
#include <FreeRTOS.h>

void GUITool_DumpHeader(GuiFontHeader_t* fileHeader) {
    printf("=========文件信息=========\r\n");
    printf("文件类型:%c%c%c\n", fileHeader->magic[0], fileHeader->magic[1], fileHeader->magic[2]);
    printf("文件版本:%d.%d\r\n", (fileHeader->magic[3] & 0xF0) >> 4, fileHeader->magic[3] & 0x0f);
    printf("文件大小:%ld Bytes\r\n", fileHeader->dwFileSize);
    printf("段信息数量:%d\r\n", fileHeader->nSection);
    printf("字体高度:%d\r\n", fileHeader->nYSize);
    printf("CodePage标志位:0x%04x\r\n", fileHeader->wCpFlag);
    printf("字符数量:%d\r\n", fileHeader->nTotalChars);
}
void GUITool_GetHeader(const uint32_t fontAddr,GuiFontHeader_t* pHeader){
    // 读取FontHeader
    W25X_Read_Data((uint8_t*)pHeader,fontAddr,sizeof(GuiFontHeader_t));
}
void GUITool_GetSection(const uint32_t fontAddr,uint32_t unicode,GuiFontSection_t* pSection){
    GuiFontHeader_t     header;
    uint32_t            offsetAddr = fontAddr+sizeof(GuiFontHeader_t);
    uint8_t             i;
    GUITool_GetHeader(fontAddr,&header);
    for(i=0; i<header.nSection; i++) {
        W25X_Read_Data((uint8_t*)pSection,fontAddr+offsetAddr,sizeof(GuiFontSection_t));
        if(pSection->first <= unicode && pSection->last >= unicode) {
            break;
        }
        offsetAddr+=sizeof(GuiFontSection_t);
    }
}
void GUITool_GetIndex(GuiFontSection_t* pSection,uint32_t unicode,GuiFontIndex_t* pIndex){
    uint32_t uiFontIndex;
    uint32_t offsetAddr=pSection->offsetAddr+sizeof(uint32_t)*(unicode-pSection->first);
    W25X_Read_Data((uint8_t*)&uiFontIndex,offsetAddr,sizeof(uint32_t));
    pIndex->width       = (uiFontIndex & 0xFC000000) >> 26;
    pIndex->offsetAddr  = (uiFontIndex & 0x03FFFFFF);
}
void GUITool_GetFontData(const uint32_t fontAddr,uint16_t unicode,uint8_t* pBuffer){

}



void GUITool_GetFont(const uint32_t fontAddr,uint16_t unicode,GuiFont_t* font) {
    GuiFontHeader_t header;
    GuiFontSection_t section;
    GuiFontIndex_t index;
    uint32_t offsetAddr,rawFontIndex;
    uint8_t i;
    uint8_t* pBitmap;
    // 读取FontHeader
    W25X_Read_Data((uint8_t*)&header,fontAddr,sizeof(header));
    // 查找section
    offsetAddr=sizeof(header);
    for(i=0; i<header.nSection; i++) {
        W25X_Read_Data((uint8_t*)&section,fontAddr+offsetAddr,sizeof(section));
        if(section.first<=unicode && section.last>=unicode) {
            break;
        }
        offsetAddr+=sizeof(section);
    }
    if(i>=header.nSection) {
        GUITool_GetFont(fontAddr,'?',font);
        return;
    }
    // 读取FontIndex
    offsetAddr=section.offsetAddr+sizeof(uint32_t)*(unicode-section.first);
    W25X_Read_Data((uint8_t*)&rawFontIndex,fontAddr+offsetAddr,sizeof(uint32_t));
    index.width         = (rawFontIndex & 0xFC000000) >> 26;
    index.offsetAddr    = (rawFontIndex & 0x03FFFFFF);
    // 读取Bitmap数据
    uint8_t fontBitmapSize=header.nYSize * ((index.width + 8 - 1) / 8);
    pBitmap=(uint8_t*)pvPortMalloc(fontBitmapSize);
    font->width=index.width;
    font->height=header.nYSize;
    W25X_Read_Data(pBitmap,fontAddr+index.offsetAddr,fontBitmapSize);
    font->bitmap=pBitmap;
}


