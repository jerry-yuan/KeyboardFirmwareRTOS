#include <screen/keyboardscr.h>

#include <FreeRTOS.h>
#include <SGUI_Basic.h>

#include <bsp/oled.h>
#include <bsp/w25x.h>
#include <bsp/tim.h>
#include <bsp/flashMap.h>

#include <usb/hw_config.h>
#include <usb_lib.h>

#define ACTION_MASK_TURNONSCR   0x80
#define ACTION_MASK_RESET_TIM   0x40
#define ACTION_MASK_DISABLE_TIM 0x20

enum KeyboardScreenAction {
    NoAction        = 0x00,
    TurnOnScreen    = 0xC0,
    GoMenu          = 0xA0,
    RedrawState     = 0xC1
};

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

static void standardKeyboardTransferHandler        (MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);
static void consumerKeyboardStandbyTransferHandler (MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);
static void consumerKeyboardWorkingTransferHandler (MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);

static HMI_SCREEN_ACTION screenActions = {
    Initialize,
    Prepare,
    Refresh,
    ProcessEvent,
    PostProcess
};

HMI_SCREEN_OBJECT SCREEN_Keyboard = {SCREEN_Keyboard_State_ID,&screenActions};

__IO uint32_t keyboardStatus=0x00;

static KeyboardStateMachine_t* stateMachine=NULL;

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {

    stateMachine = (KeyboardStateMachine_t*)pvPortMalloc(sizeof(KeyboardStateMachine_t));
    stateMachine->currentState = StandardKeyboardWorking;
    stateMachine->transferHandler[StandardKeyboardWorking] = standardKeyboardTransferHandler;
    stateMachine->transferHandler[ConsumerKeyboardStandby] = consumerKeyboardStandbyTransferHandler;
    stateMachine->transferHandler[ConsumerKeyboardWorking] = consumerKeyboardWorkingTransferHandler;
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    TIM_ScreenSaver_Reset();
    W25X_Read_Data(FLASH_ADDR_RUNBG,oledFramebuffer,OLED_FRAMEBUFFER_SIZE);
    Refresh(pstDeviceIF,NULL);
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    // 数字锁定
    SGUI_Basic_DrawRectangle(pstDeviceIF, 59,32,10,10,0x0A,0x0A*((keyboardStatus&KEYBOARD_STATE_NUMLOCK_MASK)>>0));
    // 大写锁定
    SGUI_Basic_DrawRectangle(pstDeviceIF,123,32,10,10,0x0A,0x0A*((keyboardStatus&KEYBOARD_STATE_CAPSLOCK_MASK)>>1));
    // 滚动锁定
    SGUI_Basic_DrawRectangle(pstDeviceIF,187,32,10,10,0x0A,0x0A*((keyboardStatus&KEYBOARD_STATE_SCROLLLOCK_MASK)>>1));
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    MappedKeyCodes_t stPressed,stRelease;
	KEY_EVENT* pstKeyEvent;
    *piActionID = NoAction;
    if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        pstKeyEvent = (KEY_EVENT*)pstEvent;

		stPressed.cursor	= 0;
		stPressed.length	= pstKeyEvent->Data.uiPressedCount;
		stPressed.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stPressed.length);
		mapKeyCodes(pstKeyEvent->Data.pstPressed,stPressed.keyCodes);

		stRelease.cursor	= 0;
		stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
		stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stRelease.length);
		mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);

        while(stPressed.cursor<stPressed.length || stRelease.cursor<stRelease.length ){
            stateMachine->transferHandler[stateMachine->currentState](&stPressed,&stRelease,piActionID);
            if(*piActionID!=NoAction){
                break;
            }
        }

        vPortFree(stPressed.keyCodes);
        vPortFree(stRelease.keyCodes);

    } else if(pstEvent->iID == KEYBOARD_STATE_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEYBOARD_STATE_EVENT)) {
        *piActionID = RedrawState;
    } else if(pstEvent->iID == RTC_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,RTC_EVENT)){
        // do nothing
    }else{
        *piActionID = TurnOnScreen;
    }
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID==RedrawState) {
        Refresh(pstDeviceIF,NULL);
    } else if(iActionID==GoMenu) {
        HMI_SwitchScreen(SCREEN_Menu_ID,(void*)0xFFFFFFFF);
    }
    if(iActionID & ACTION_MASK_TURNONSCR){
        OLED_SetDisplayState(true);
    }
    if(iActionID & ACTION_MASK_RESET_TIM){
        TIM_ScreenSaver_Reset();
    }
    if(iActionID & ACTION_MASK_DISABLE_TIM){
        TIM_ScreenSaver_Disable();
    }
    KBDLib_SyncState();
    return HMI_RET_NORMAL;
}


void standardKeyboardTransferHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {
    if(pstPressed->length>0 && pstPressed->keyCodes[0] & KEY_Fn_BIT_MASK) {
        // 发现是Fn键 => 切换至扩展键盘预备状态
        stateMachine->currentState=ConsumerKeyboardStandby;
        pstPressed->cursor++;
        KBDLib_ReleaseAllStdKeys();
    } else {
        KBDLib_PressStdKeys(pstPressed);
        KBDLib_ReleaseStdKeys(pstRelease);
    }
    //JKBD_Send((uint8_t*)standardKeyboardReport,sizeof(StandardKeyboardReport_t),ENDP1);
}
void consumerKeyboardStandbyTransferHandler (MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {
    if(pstRelease->length>0 && (pstRelease->keyCodes[0]&KEY_Fn_BIT_MASK)) {
        stateMachine->currentState=StandardKeyboardWorking;
        if(TIM_ScreenSaver_IsEnabled()>0){
            *piActionID = GoMenu;
        }else{
            *piActionID = TurnOnScreen;
        }
    } else {
        stateMachine->currentState=ConsumerKeyboardWorking;
    }
}
void consumerKeyboardWorkingTransferHandler (MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {
    if(pstRelease->length>0 &&  (pstRelease->keyCodes[0]&KEY_Fn_BIT_MASK)) {
        stateMachine->currentState=StandardKeyboardWorking;
        standardKeyboardTransferHandler(pstPressed,pstRelease,piActionID);
    } else {
        KBDLib_PressExtKeys(pstPressed);
        KBDLib_ReleaseExtKeys(pstRelease);
        //JKBD_Send((uint8_t*)consumerKeyboardReport,sizeof(ConsumerKeyboardReport_t),ENDP2);
    }
}
