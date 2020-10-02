#include <SGUI_Basic.h>
#include <screen/calculator/calculator.h>

#include <lib/keyboard.h>

#include <resources/Font.h>


enum{
    NoAction      = 0x00,
    GoBack        = 0x01
};

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

static HMI_SCREEN_ACTION screenActions={
    Initialize,
    Prepare,
    Refresh,
    ProcessEvent,
    PostProcess
};

HMI_SCREEN_OBJECT SCREEN_Calculator={SCREEN_Calculator_ID,&screenActions};

HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF){
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters){
	SGUI_Basic_ClearScreen(pstDeviceIF);
	Refresh(pstDeviceIF,NULL);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters){
	SGUI_RECT stRect;
	SGUI_POINT stPoint;

	stRect.iHeight=44;
	stRect.iWidth=256;
	stRect.iX=0;
	stRect.iY=10;

	stPoint.iX=stPoint.iY=0;

	SGUI_Text_DrawText(pstDeviceIF,"0 1 2 3 4 5 6 7 8 9 A B C D E F",SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0A);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID){
	KEY_EVENT* pstKeyEvent;
    MappedKeyCodes_t stPressed,stRelease;
    *piActionID = NoAction;
	if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)){
		pstKeyEvent = (KEY_EVENT*)pstEvent;
		stPressed.cursor    = 0;
        stPressed.length	= pstKeyEvent->Data.uiPressedCount;
        stPressed.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stPressed.length);
        mapKeyCodes(pstKeyEvent->Data.pstPressed,stPressed.keyCodes);

        stRelease.cursor    = 0;
        stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stRelease.length);
        mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);

        if(containsKey(&stRelease,KeyEscape)){
			*piActionID = GoBack;
        }

        vPortFree(stPressed.keyCodes);
        vPortFree(stRelease.keyCodes);

	}
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID){
	if(iActionID==GoBack){
		HMI_GoBack(NULL);
	}
    return HMI_RET_NORMAL;
}
