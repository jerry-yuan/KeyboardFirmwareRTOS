#include<stdio.h>
#include<GUIToolLib.h>
#include<bsp/w25q64.h>
#include <FreeRTOS.h>

static void dumpMemory(const uint8_t* src,uint32_t length){
	printf("  ADDR   |");
    for(uint8_t i=0;i<16;i++){
        printf(" %02X",i);
    }
    printf("\r\n----------------------------------------------------------");
    for (uint32_t i = 0; i < length; i++) {
        if (i % 16 == 0) {
            printf("\r\n");
            printf("%08X |",(unsigned int)src);
        }
        printf(" %02X", *(src++));
    }
    printf("\r\n");
}

static uint8_t getPixel(uint8_t* src,int x,int y,int xSize){
    uint8_t bytesPerLine = (xSize+1)/2;
    if(x%2){
        return *(src+bytesPerLine*y+x/2)>>4 & 0x0F;
    }else{
        return *(src+bytesPerLine*y+x/2) & 0x0F;
    }

}
static void setPixel(uint8_t* dst,int x,int y,int xSize,uint8_t value){
    uint8_t bytesPerLine = (xSize+1)/2;
    if(x%2){
        *(dst+bytesPerLine*y+x/2) |= (value& 0x0F)<<4;
    }else{
        *(dst+bytesPerLine*y+x/2) |= (value& 0x0F);
    }
}
void GUITool_ReadBitmap(SGUI_BMP_RES* pstBitmap,uint16_t uiCode,const uint32_t uiFontOffset){
	GUI_FONT_HEADER stHeader;
	GUI_FONT_SECTION stSection;
	GUI_FONT_CHARINFO stCharInfo;
	uint8_t* puiCharData;
	SGUI_COLOR eColor;
	// 读取字体头
	W25X_Read_Data((uint8_t*)&stHeader,uiFontOffset,sizeof(stHeader));
	// 读取searchTree根节点
	W25X_Read_Data((uint8_t*)&stSection,uiFontOffset+stHeader.pSearchTreeArea,sizeof(stSection));
	// 查找这个字所在段
	while(stSection.pLeftChild!=0 ||stSection.pRightChild!=0){
		if(stSection.uiFirst>uiCode || stSection.uiLast<uiCode){
			break;
		}
		if(stSection.uiMiddle<uiCode){
			W25X_Read_Data((uint8_t*)&stSection,uiFontOffset+stSection.pRightChild,sizeof(stSection));
		}else{
			W25X_Read_Data((uint8_t*)&stSection,uiFontOffset+stSection.pLeftChild,sizeof(stSection));
		}
	}
	if(stSection.uiFirst>uiCode || stSection.uiLast<uiCode){
		printf("%04X cannot found in the font base.\r\n",uiCode);
		return;
	}
	// 查找这个字的属性
	W25X_Read_Data((uint8_t*)&stCharInfo,uiFontOffset+stSection.pCharInfoAddr+sizeof(GUI_FONT_CHARINFO)*(uiCode-stSection.uiFirst),sizeof(stSection));

	pstBitmap->fnGetPixel = SGUI_BMP_SCAN_MODE_DHPH;
	pstBitmap->iDepthBits = stHeader.uiDepthBits;
	pstBitmap->iHeight = stHeader.uiYSize;
	pstBitmap->iWidth = SGUI_MAX_OF(stCharInfo.uiOffsetAddr >> 24,stCharInfo.uiXPos+stCharInfo.uiXSize);

	if(pstBitmap->pData!=NULL){
		// 读取数据
		SGUI_SystemIF_MemorySet((uint8_t*)pstBitmap->pData,0,(pstBitmap->iWidth+1)/2*pstBitmap->iHeight);
		puiCharData = pvPortMalloc(sizeof(uint8_t)*(stCharInfo.uiXSize+1)/2*stCharInfo.uiYSize);
		W25X_Read_Data((uint8_t*)puiCharData,uiFontOffset+(stCharInfo.uiOffsetAddr & 0x00FFFFFF),(stCharInfo.uiXSize+1)/2*stCharInfo.uiYSize);
		for(uint8_t x=0;x<stCharInfo.uiXSize;x++){
			for(uint8_t y=0;y<stCharInfo.uiYSize;y++){
				eColor = getPixel(puiCharData,x,y,stCharInfo.uiXSize);
				setPixel((uint8_t*)pstBitmap->pData,x+stCharInfo.uiXPos,y+stCharInfo.uiYPos,pstBitmap->iWidth,eColor);
			}
		}
		vPortFree(puiCharData);
	}

}
