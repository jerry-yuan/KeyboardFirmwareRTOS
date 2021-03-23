#include <resources/Font.h>
#include <SGUI_Basic.h>
#include <SGUI_Text.h>
#include <bsp/w25x.h>
#include <lib/GUIToolLib.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <flashMap.h>
static void GUITool_Deng12_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun);
static void GUITool_LCD44_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun);

const SGUI_FONT_RES SGUI_FONT(Deng12)={
    12,4,GUITool_Deng12_GetBitmap,SGUI_TEXT_DECODER_UTF8
};

static void GUITool_Deng12_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun){
	if(bDryRun){
		pstBitmap->pData = NULL;
	}
	GUITool_ReadBitmap(pstBitmap,uiCode,FLASH_ADDR_DENGFONT_12);
}

const SGUI_FONT_RES SGUI_FONT(LCD44)={
	44,4,GUITool_LCD44_GetBitmap,SGUI_TEXT_DECODER_UTF8
};

static void GUITool_LCD44_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun){
	if(bDryRun){
		pstBitmap->pData = NULL;
	}
	GUITool_ReadBitmap(pstBitmap,uiCode,FLASH_ADDR_LCDFONT_44);
}
