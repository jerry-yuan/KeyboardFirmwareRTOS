#include <screen/menuscr.h>
#include <screen/consts.h>
#include <string.h>
#include <FreeRTOS.h>
#include <SGUI_Menu.h>

#include <resources/Font.h>
#include <resources/fontawesome.h>

#include <bsp/tim.h>
#include <lib/keyboard.h>


enum MenuActions {
    NoAction=0x00,
    GoBack=0x01,
    GoDown=0xC0,
    GoUp=0xC1,
    Enter=0x02,
};

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

HMI_SCREEN_OBJECT SCREEN_Menu = {SCREEN_Menu_ID,&screenActions};

static SGUI_MENU* menuObject=NULL;

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    SGUI_RECT menuRect;
    SGUI_ITEMS_ITEM* menuItems=NULL;
	SGUI_MENU_PALETTE_INIT_PARAM menuPalette;
    menuItems  = pvPortMalloc(sizeof(SGUI_ITEMS_ITEM)*2);
    memset(menuItems,0,sizeof(SGUI_ITEMS_ITEM)*2);
    menuItems[0].cszLabelText = FA_CLOCK " 时钟";
    menuItems[1].cszLabelText = FA_CALCULATOR " 计算器";

    menuObject = pvPortMalloc(sizeof(SGUI_MENU));
    memset(menuObject,0,sizeof(SGUI_MENU));


    menuRect.iX=0;
    menuRect.iY=0;
    menuRect.iWidth=256;
    menuRect.iHeight=64;

	menuPalette.uiDepthBits = 4;
	menuPalette.stMenu.eBorder = 0x0F;
	menuPalette.stMenu.eDirectionIconColor = 0x0F;
	menuPalette.stItemBase.uiDepthBits = 4;
	menuPalette.stItemBase.eBackgroundColor = 0x01;
	menuPalette.stItemBase.eTextColor = 0x0F;
	menuPalette.stItemBase.eFocusColor = 0x05;
	menuPalette.stItemBase.eFocusTextColor = 0x0F;

    SGUI_Menu_Initialize(menuObject,&menuRect,&menuPalette,SGUI_FONT_REF(Deng12),menuItems,2);
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
	if((uint32_t)pstParameters != 0xFFFFFFFF){
		HMI_GoBack(NULL);
		return HMI_RET_NORMAL;
	}
	SGUI_Menu_Selecte(menuObject,0);
    SGUI_Basic_ClearScreen(pstDeviceIF);
    SGUI_Menu_Repaint(pstDeviceIF,menuObject);
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_Menu_Repaint(pstDeviceIF,menuObject);
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    KEY_EVENT* keyEvent;
    MappedKeyCodes_t stPressed,stRelease;
    *piActionID = NoAction;
    if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        keyEvent = ((KEY_EVENT*)pstEvent);
		iLastAction = NoAction;
		TIM_KeyRepeater_Reset();

        stPressed.cursor	= 0;
        stPressed.length	= keyEvent->Data.uiPressedCount;
        stPressed.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stPressed.length);
		mapKeyCodes(keyEvent->Data.pstPressed,stPressed.keyCodes);

        stRelease.cursor	= 0;
        stRelease.length	= keyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stRelease.length);
		mapKeyCodes(keyEvent->Data.pstRelease,stRelease.keyCodes);

        if(containsKey(&stPressed,KeyDown)) {
            iLastAction=*piActionID = GoDown;
        } else if(containsKey(&stPressed,KeyUp)) {
            iLastAction=*piActionID = GoUp;
        } else if(containsKey(&stRelease,KeyEscape)) {
            *piActionID = GoBack;
        } else if(containsKey(&stRelease,KeyEnter)) {
            *piActionID = Enter;
        }

        vPortFree(stPressed.keyCodes);
        vPortFree(stRelease.keyCodes);

        if(iLastAction & 0x40) {
            TIM_KeyRepeater_Set();
        }
    }else if(pstEvent->iID == KEY_REPEAT_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_REPEAT_EVENT)){
		if(iLastAction!=NoAction){
			*piActionID = iLastAction;
		}
    }
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID == GoUp) {
		SGUI_Menu_Selecte(menuObject,(SGUI_Menu_GetSelection(menuObject)->iIndex + menuObject->stItems.iCount-1) % menuObject->stItems.iCount);
    } else if(iActionID == GoDown) {
        SGUI_Menu_Selecte(menuObject,(SGUI_Menu_GetSelection(menuObject)->iIndex + 1) % menuObject->stItems.iCount);
    } else if(iActionID == GoBack){
        HMI_GoBack(NULL);
    } else if(iActionID == Enter){
        if(SGUI_Menu_GetSelection(menuObject)->iIndex == 0){
            HMI_SwitchScreen(SCREEN_Clock_Show_ID,NULL);
        }else if(SGUI_Menu_GetSelection(menuObject)->iIndex == 1){
			HMI_SwitchScreen(SCREEN_Calculator_ID,NULL);
        }
    }

    if(iActionID & 0x80) {
        Refresh(pstDeviceIF,NULL);
    }

    return HMI_RET_NORMAL;
}
