#include <stdio.h>
#include <lib/GUIToolLib.h>
#include <bsp/w25x.h>
#include <FreeRTOS.h>

void dumpMemory(const uint8_t* src,uint32_t length){
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
void GUITool_ReadBitmap(struct lfs_file* pstFontFile,SGUI_BMP_RES* pstBitmap,uint16_t uiCode){
	GUI_FONT_HEADER stHeader;
	GUI_FONT_SECTION stSection;
	GUI_FONT_CHARINFO stCharInfo;
	uint8_t* puiCharData;
	SGUI_COLOR eColor;
	// 读取字体头
	lfs_file_seek(pstW25Xfs,pstFontFile,0,LFS_SEEK_SET);
	lfs_file_read(pstW25Xfs,pstFontFile,&stHeader,sizeof(stHeader));
	// 读取searchTree根节点
	lfs_file_seek(pstW25Xfs,pstFontFile,stHeader.pSearchTreeArea,LFS_SEEK_SET);
	lfs_file_read(pstW25Xfs,pstFontFile,&stSection,sizeof(stSection));
	// 查找这个字所在段
	while(stSection.pLeftChild!=0 ||stSection.pRightChild!=0){
		if(stSection.uiFirst>uiCode || stSection.uiLast<uiCode){
			break;
		}
		if(stSection.uiMiddle<uiCode){
			lfs_file_seek(pstW25Xfs,pstFontFile,stSection.pRightChild,LFS_SEEK_SET);
		}else{
			lfs_file_seek(pstW25Xfs,pstFontFile,stSection.pLeftChild,LFS_SEEK_SET);
		}
		lfs_file_read(pstW25Xfs,pstFontFile,&stSection,sizeof(stSection));
	}
	if(stSection.uiFirst>uiCode || stSection.uiLast<uiCode){
		printf("%04X cannot found in the font base.\r\n",uiCode);
		return;
	}
	// 查找这个字的属性
	lfs_file_seek(pstW25Xfs,pstFontFile,stSection.pCharInfoAddr+sizeof(GUI_FONT_CHARINFO)*(uiCode-stSection.uiFirst),LFS_SEEK_SET);
	lfs_file_read(pstW25Xfs,pstFontFile,&stCharInfo,sizeof(stSection));

	pstBitmap->fnGetPixel = SGUI_BMP_SCAN_MODE_DHPH;
	pstBitmap->uiDepthBits = stHeader.uiDepthBits;
	pstBitmap->iHeight = stHeader.uiYSize;
	pstBitmap->iWidth = SGUI_MAX_OF(stCharInfo.uiOffsetAddr >> 24,stCharInfo.uiXPos+stCharInfo.uiXSize);

	if(pstBitmap->pData!=NULL){
		// 读取数据
		SGUI_SystemIF_MemorySet((uint8_t*)pstBitmap->pData,0,(pstBitmap->iWidth+1)/2*pstBitmap->iHeight);
		puiCharData = pvPortMalloc(sizeof(uint8_t)*(stCharInfo.uiXSize+1)/2*stCharInfo.uiYSize);
		lfs_file_seek(pstW25Xfs,pstFontFile,(stCharInfo.uiOffsetAddr & 0x00FFFFFF),LFS_SEEK_SET);
		lfs_file_read(pstW25Xfs,pstFontFile,puiCharData,(stCharInfo.uiXSize+1)/2*stCharInfo.uiYSize);
		for(uint8_t x=0;x<stCharInfo.uiXSize;x++){
			for(uint8_t y=0;y<stCharInfo.uiYSize;y++){
				eColor = getPixel(puiCharData,x,y,stCharInfo.uiXSize);
				setPixel((uint8_t*)pstBitmap->pData,x+stCharInfo.uiXPos,y+stCharInfo.uiYPos,pstBitmap->iWidth,eColor);
			}
		}
		vPortFree(puiCharData);
	}

}
