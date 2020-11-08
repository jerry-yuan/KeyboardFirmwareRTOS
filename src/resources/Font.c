#include <resources/Font.h>
#include <SGUI_Basic.h>
#include <SGUI_Text.h>
#include <bsp/W25Q64.h>
#include <lib/GUIToolLib.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <fatfs/ff.h>
static void GUITool_Deng12_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun);
static void GUITool_LCD44_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun);
static FIL* pstDeng12FontFile;
static FIL* pstLCDFontFile;

const SGUI_FONT_RES SGUI_FONT(Deng12)= {
    12,4,GUITool_Deng12_GetBitmap,SGUI_TEXT_DECODER_UTF8
};

static void GUITool_Deng12_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun) {
    if(bDryRun) {
        pstBitmap->pData = NULL;
    }
    GUITool_ReadBitmap(pstBitmap,uiCode,pstDeng12FontFile);
}

const SGUI_FONT_RES SGUI_FONT(LCD44)= {
    44,4,GUITool_LCD44_GetBitmap,SGUI_TEXT_DECODER_UTF8
};

static void GUITool_LCD44_GetBitmap(SGUI_BMP_RES* pstBitmap,SGUI_UINT32 uiCode,SGUI_BOOL bDryRun) {
    if(bDryRun) {
        pstBitmap->pData = NULL;
    }
    GUITool_ReadBitmap(pstBitmap,uiCode,pstLCDFontFile);
}
static void initFontFile(const char* path,FIL* pstFile) {
    FRESULT eResult;
    eResult = f_open(pstFile,path,FA_READ);
    if(eResult != FR_OK){
		printf("Failed to open font file %s:%d\r\n",path,eResult);
    }
}
void FONT_Initialize() {
    pstDeng12FontFile = pvPortMalloc(sizeof(FIL));
    pstLCDFontFile    = pvPortMalloc(sizeof(FIL));

    initFontFile("fonts/Deng12.font",pstDeng12FontFile);
    initFontFile("fonts/LCD.font",pstLCDFontFile);
}
