#include <screen/menuscr.h>
#include <screen/consts.h>
#include <string.h>
#include <FreeRTOS.h>
#include <SGUI_Menu.h>

#include <resources/Font.h>
#include <task/keyboard.h>

enum MenuActions {
    NoAction=0,
    GoBack=0x01,
    GoDown=0x80,
    GoUp=0x81,
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

HMI_SCREEN_OBJECT SCREEN_Menu = {MenuScreen_ID,&screenActions};

static SGUI_MENU_STRUCT* menuObject=NULL;

static bool ContainsKey(MappedKeyCodes_t* mappedKeyCodes,uint8_t keyCode) {
    for(int i=0; i<mappedKeyCodes->length; i++) {
        if((uint8_t)(mappedKeyCodes->keyCodes[i] & 0xFF) == keyCode) {
            return true;
        }
    }
    return false;
}
static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    SGUI_RECT menuRect;
    SGUI_ITEMS_ITEM* menuItems=NULL;

    menuItems  = pvPortMalloc(sizeof(SGUI_ITEMS_ITEM)*5);
    memset(menuItems,0,sizeof(SGUI_ITEMS_ITEM)*5);
    menuItems[0].cszLabelText = "菜单1";
    menuItems[1].cszLabelText = "菜单2";
    menuItems[2].cszLabelText = "菜单3";
    menuItems[3].cszLabelText = "菜单4";
    menuItems[4].cszLabelText = "菜单5";

    menuObject = pvPortMalloc(sizeof(SGUI_MENU_STRUCT));
    memset(menuObject,0,sizeof(SGUI_MENU_STRUCT));


    menuRect.iX=0;
    menuRect.iY=0;
    menuRect.iWidth=256;
    menuRect.iHeight=64;

    SGUI_Menu_Initialize(menuObject,SGUI_FONT_REF(Deng12),&menuRect,menuItems,5);
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
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
    *piActionID = NoAction;
    if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        keyEvent = ((KEY_EVENT*)pstEvent);
        if(ContainsKey(&keyEvent->Data.stRelease,KeyDown)) {
            printf("GoDown!\r\n");
            *piActionID = GoDown;
        } else if(ContainsKey(&keyEvent->Data.stRelease,KeyUp)) {
            printf("GoUp!\r\n");
            *piActionID = GoUp;
        } else if(ContainsKey(&keyEvent->Data.stRelease,KeyEscape)) {
            printf("GoBack!\r\n");
            *piActionID = GoBack;
        } else if(ContainsKey(&keyEvent->Data.stRelease,KeyEnter)) {
            printf("Enter!\r\n");
            *piActionID = Enter;
        }
    }
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID == GoUp) {
        menuObject->stItems.iSelection=(menuObject->stItems.iSelection+menuObject->stItems.iCount-1) % menuObject->stItems.iCount;
    } else if(iActionID == GoDown) {
        menuObject->stItems.iSelection=(menuObject->stItems.iSelection+1)%menuObject->stItems.iCount;
    } else if(iActionID == GoBack){
        HMI_GoBack(NULL);
    }

    if(iActionID & 0x80) {
        Refresh(pstDeviceIF,NULL);
    }

    return HMI_RET_NORMAL;
}
