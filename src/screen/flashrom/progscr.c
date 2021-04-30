#include <screen/flashrom/progscr.h>
#include <screen/flashrom/initscr.h>
#include <screen/flashrom/task.h>
#include <screen/consts.h>
#include <SGUI_Text.h>
#include <SGUI_FontResource.h>
#include <SGUI_ProcessBar.h>

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

static HMI_SCREEN_ACTION screenActions= {
    Initialize,
    Prepare,
    Refresh,
    ProcessEvent,
    PostProcess
};

HMI_SCREEN_OBJECT SCREEN_FlashRom_Prog= {SCREEN_FlashRom_Prog_ID,&screenActions};

enum {
    NoAction      = 0x00,
    RefreshScreen = 0x01,
    GoBack        = 0x02
};

static FlashRomProgScrContext_t* pstContext = NULL;

HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    pstContext = pvPortMalloc(sizeof(FlashRomProgScrContext_t));

    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {

    pstContext->stProgressBar.stParameter.eDirection = SGUI_PROCBAR_RIGHT;
    pstContext->stProgressBar.stParameter.sMaxValue  = 100;
    pstContext->stProgressBar.stParameter.stLayout.iX = 25;
    pstContext->stProgressBar.stParameter.stLayout.iY = 44;
    pstContext->stProgressBar.stParameter.stLayout.iWidth = 206;
    pstContext->stProgressBar.stParameter.stLayout.iHeight = 10;
    pstContext->stProgressBar.stParameter.stPalette.uiDepthBits = 4;
    pstContext->stProgressBar.stParameter.stPalette.eEdgeColor = 0x0A;
    pstContext->stProgressBar.stParameter.stPalette.eBackgroundColor = 0x01;
    pstContext->stProgressBar.stParameter.stPalette.eProcessBarColor = 0x0A;

    pstContext->stProgressBar.stData.sValue = 100;

    Refresh(pstDeviceIF,NULL);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_RECT stRect = {0,0,128,8};
    SGUI_POINT stPoint = {0,0};
    SGUI_AREA_SIZE stArea = {0,0};
    char buffer[30]= {0};
    const char* tips[]= {
        "Waiting for instructions from Host...",
        "Host is reading Flash...",
        "Host is writing Flash..."
    };
    SGUI_Basic_ClearScreen(pstDeviceIF);

    SGUI_Text_GetTextExtent(tips[pstContext->eType],SGUI_FONT_REF(FONT_8),&stArea);
    stRect.iX = (256-stArea.iWidth)/2;
    stRect.iY = 16-stArea.iHeight/2;
    stRect.iWidth = stArea.iWidth;
    stRect.iHeight = stArea.iHeight;

    SGUI_Text_DrawText(pstDeviceIF,tips[pstContext->eType],SGUI_FONT_REF(FONT_8),&stRect,&stPoint,0x0A);
    if(pstContext->eType>0) {
        sprintf(buffer,"%02.1f%%(0x%06XB/0x%06lXB)",
                pstContext->stProgressBar.stData.sValue*100.0/pstContext->stProgressBar.stParameter.sMaxValue,
                pstContext->stProgressBar.stData.sValue,
                pstContext->stProgressBar.stParameter.sMaxValue
               );
        SGUI_Text_GetTextExtent(buffer,SGUI_FONT_REF(FONT_8),&stArea);
        stRect.iX = (256-stArea.iWidth)/2;
        stRect.iY = (64-stArea.iHeight)/2;
        stRect.iWidth = stArea.iWidth;
        stRect.iHeight = stArea.iHeight;
        SGUI_Text_DrawText(pstDeviceIF,buffer,SGUI_FONT_REF(FONT_8),&stRect,&stPoint,0x0A);
        SGUI_ProcessBar_Repaint(pstDeviceIF,&pstContext->stProgressBar);
    }


    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    FLASHROM_PBAR_SETUP_EVENT*	pstSetupEvent;
    FLASHROM_PBAR_UPDATE_EVENT*	pstUpdateEvent;
    *piActionID = NoAction;
    if(pstEvent->iID == FLASHROM_LOCK_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,FLASHROM_LOCK_EVENT)) {
        if(((FLASHROM_LOCK_EVENT*)pstEvent)->Data == 0){
			*piActionID = GoBack;
		}
    } else if(pstEvent->iID == FLASHROM_PBAR_SETUP_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,FLASHROM_PBAR_SETUP_EVENT)) {
        pstSetupEvent = (FLASHROM_PBAR_SETUP_EVENT*)pstEvent;
        pstContext->eType = pstSetupEvent->Data.eProgressBarType;
        pstContext->stProgressBar.stParameter.sMaxValue = pstSetupEvent->Data.uiBytesTotal;
        pstContext->stProgressBar.stData.sValue = 0;
        *piActionID = RefreshScreen;
    } else if(pstEvent->iID == FLASHROM_PBAR_UPDATE_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,FLASHROM_PBAR_UPDATE_EVENT)) {
        pstUpdateEvent = (FLASHROM_PBAR_UPDATE_EVENT*)pstEvent;
        pstContext->stProgressBar.stData.sValue = pstUpdateEvent->Data;
        *piActionID = RefreshScreen;
    }
    /*if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        pstKeyEvent = (KEY_EVENT*)pstEvent;

        stRelease.cursor	= 0;
        stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*pstKeyEvent->Data.uiReleaseCount);
        mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);

        if(containsKey(&stRelease,KeyEscape)) {
            *piActionID = GoBack;
        }

        vPortFree(stRelease.keyCodes);
    }*/
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID == RefreshScreen) {
        Refresh(pstDeviceIF,NULL);
    } else if(iActionID == GoBack) {
        HMI_GoBack((const void*)FLASHROM_SWITCH_SOURCE_PROG);
    }
    return HMI_RET_NORMAL;
}

