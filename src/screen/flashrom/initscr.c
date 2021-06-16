#include <screen/flashrom/initscr.h>
#include <screen/flashrom/task.h>
#include <screen/consts.h>
#include <lib/keyboard.h>
#include <task/priorities.h>
#include <stdio.h>
#include <SGUI_Text.h>
#include <SGUI_FontResource.h>

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

HMI_SCREEN_OBJECT SCREEN_FlashRom_Init= {SCREEN_FlashRom_Init_ID,&screenActions};

enum {
    NoAction      = 0x00,
    RefreshScreen = 0x01,
    GoBack        = 0x02,
    LockScreen    = 0x80,
};

static TaskHandle_t hFlashRomTask=NULL;


HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    uint32_t uiFrom = (uint32_t)pstParameters;
    BaseType_t xSuccess = pdPASS;
    if(uiFrom==FLASHROM_SWITCH_SOURCE_MENU) {
        if(hFlashRomTask!=NULL) {
            vTaskDelete(hFlashRomTask);
            hFlashRomTask = NULL;
        }

        xSuccess=xTaskCreate((TaskFunction_t)flashRomTask,"flashrom-task",2048,NULL,TASK_FLASHROM_PRIORITY,&hFlashRomTask);
        if(xSuccess==pdFALSE) {
            printf("create flash rom failed!\r\n");
        }
    }
    SGUI_Basic_ClearScreen(pstDeviceIF);
    Refresh(pstDeviceIF,NULL);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_RECT stRect = {0,0,128,8};
    SGUI_POINT stPoint = {0,0};

    SGUI_Text_DrawText(pstDeviceIF,"FlashROM",SGUI_FONT_REF(FONT_8),&stRect,&stPoint,0x1);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    KEY_EVENT* pstKeyEvent;
    MappedKeyCodes_t stRelease;
    *piActionID = NoAction;
    if(pstEvent->iID == FLASHROM_LOCK_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,FLASHROM_LOCK_EVENT)){
		if(((FLASHROM_LOCK_EVENT*)pstEvent)->Data == 1){
			*piActionID = LockScreen;
		}
    }else if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        pstKeyEvent = (KEY_EVENT*)pstEvent;

        stRelease.cursor	= 0;
        stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*pstKeyEvent->Data.uiReleaseCount);
        mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);

        if(containsKey(&stRelease,KeyEscape)) {
            *piActionID = GoBack;
        }

        vPortFree(stRelease.keyCodes);
    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID == RefreshScreen) {
        Refresh(pstDeviceIF,NULL);
    } else if(iActionID == LockScreen){
		HMI_SwitchScreen(SCREEN_FlashRom_Prog_ID,NULL);
    } else if(iActionID == GoBack) {
        vTaskDelete(hFlashRomTask);
        hFlashRomTask = NULL;
        HMI_GoBack(NULL);
    }
    return HMI_RET_NORMAL;
}

