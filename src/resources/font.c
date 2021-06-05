#include <resources/Font.h>
#include <resources/fs.h>
#include <SGUI_Basic.h>
#include <SGUI_Text.h>
#include <bsp/w25x.h>
#include <lib/GUIToolLib.h>
#include <stdio.h>
#include <FreeRTOS.h>
static struct lfs_file* pstDeng12File;
static struct lfs_file* pstLCDFile;

void Font_Initialize(){
	int lfsResult;
	// 打开等线字体
	pstDeng12File = pvPortMalloc(sizeof(struct lfs_file));
	lfsResult=lfs_file_open(pstW25Xfs,pstDeng12File,"/fonts/Deng12",LFS_O_RDONLY);
	if(lfsResult){
		printf("Open /fonts/Deng12 failed with %d\n",lfsResult);
		HardFault_Handler();
	}
	// 打开LCD字体
	pstLCDFile = pvPortMalloc(sizeof(struct lfs_file));
	lfsResult=lfs_file_open(pstW25Xfs,pstLCDFile,"/fonts/LCD",LFS_O_RDONLY);
	if(lfsResult){
		printf("Open /fonts/LCD failed with %d\n",lfsResult);
		HardFault_Handler();
	}
}


static void GUITool_Deng12_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun);
static void GUITool_LCD44_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun);

const SGUI_FONT_RES SGUI_FONT(Deng12)={
    12,4,GUITool_Deng12_GetBitmap,SGUI_TEXT_DECODER_UTF8
};

static void GUITool_Deng12_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun){
	if(bDryRun){
		pstBitmap->pData = NULL;
	}
	GUITool_ReadBitmap(pstDeng12File,pstBitmap,uiCode);
}

const SGUI_FONT_RES SGUI_FONT(LCD44)={
	44,4,GUITool_LCD44_GetBitmap,SGUI_TEXT_DECODER_UTF8
};

static void GUITool_LCD44_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun){
	if(bDryRun){
		pstBitmap->pData = NULL;
	}
	GUITool_ReadBitmap(pstLCDFile,pstBitmap,uiCode);
}
