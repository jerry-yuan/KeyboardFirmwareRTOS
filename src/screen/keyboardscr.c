#include <screen/keyboardscr.h>

#include <FreeRTOS.h>
#include <SGUI_Basic.h>

#include <bsp/oled.h>
#include <bsp/W25Q64.h>
#include <bsp/tim.h>

#include <task/keyboard.h>

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

static void standardKeyboardTransferHandler        (KEY_EVENT* event,SGUI_INT* piActionId);
static void consumerKeyboardStandbyTransferHandler (KEY_EVENT* event,SGUI_INT* piActionId);
static void consumerKeyboardWorkingTransferHandler (KEY_EVENT* event,SGUI_INT* piActionId);

static HMI_SCREEN_ACTION screenActions = {
    Initialize,
    Prepare,
    Refresh,
    ProcessEvent,
    PostProcess
};

HMI_SCREEN_OBJECT SCREEN_Keyboard = {SCREEN_Keyboard_State_ID,&screenActions};

static KeyboardStateMachine_t* stateMachine=NULL;

static StandardKeyboardReport_t*    standardKeyboardReport;
static ConsumerKeyboardReport_t*    consumerKeyboardReport;

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    standardKeyboardReport = (StandardKeyboardReport_t*)pvPortMalloc(sizeof(StandardKeyboardReport_t));
    consumerKeyboardReport = (ConsumerKeyboardReport_t*)pvPortMalloc(sizeof(ConsumerKeyboardReport_t));
    memset(standardKeyboardReport,0,sizeof(StandardKeyboardReport_t));
    memset(consumerKeyboardReport,0,sizeof(ConsumerKeyboardReport_t));

    stateMachine = (KeyboardStateMachine_t*)pvPortMalloc(sizeof(KeyboardStateMachine_t));
    stateMachine->currentState = StandardKeyboardWorking;
    stateMachine->transferHandler[StandardKeyboardWorking] = standardKeyboardTransferHandler;
    stateMachine->transferHandler[ConsumerKeyboardStandby] = consumerKeyboardStandbyTransferHandler;
    stateMachine->transferHandler[ConsumerKeyboardWorking] = consumerKeyboardWorkingTransferHandler;
    return HMI_RET_NORMAL;
}
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    TIM_ScreenSaver_Reset();
    W25X_Read_Data(oledFramebuffer,FLASH_ADDR_RUNBG,OLED_FRAMEBUFFER_SIZE);
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
    *piActionID = NoAction;
    if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        stateMachine->transferHandler[stateMachine->currentState]((KEY_EVENT*)pstEvent,piActionID);
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
        HMI_SwitchScreen(SCREEN_Menu_ID,NULL);
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

    return HMI_RET_NORMAL;
}


static void insertToStandardKeyboardReport(uint16_t keyCode,StandardKeyboardReport_t* keyboardReport) {
    uint8_t keyValue=keyCode&0xFF;
    if(keyCode&0x0100) {
        // 控制键
        keyboardReport->controlKeys |= keyValue;
    } else {
        // 普通键
        uint8_t keyIndex=0;
        // 找到第一个空位
        while(keyIndex<6 && keyboardReport->keys[keyIndex]!=0x00 )
            keyIndex++;
        if(keyIndex<6) {
            // 找到了第一个0位 ==> 写入位置
            keyboardReport->keys[keyIndex]=keyCode;
        } else {
            // 6键全满         ==> 跳过响应此键
        }
    }
}
static void removeFromStandardKeyboardReport(uint16_t keyCode,StandardKeyboardReport_t* keyboardReport) {
    uint8_t keyValue=keyCode&0xFF;
    if(keyCode&0x0100) {
        // 控制键
        keyboardReport->controlKeys ^= keyValue;
    } else {
        // 普通键
        uint8_t keyIndex=0;
        // 找到那个与弹起的键相同的键
        while(keyIndex<6&& keyboardReport->keys[keyIndex]!=keyCode) keyIndex++;
        if(keyIndex<6) {
            // 找到一个相同
            // 寻找到最后一个有效键
            uint8_t lastKeyIndex=5;
            while(lastKeyIndex>keyIndex && keyboardReport->keys[lastKeyIndex]==0x00) lastKeyIndex--;
            if(lastKeyIndex!=keyIndex) {
                // 二者不等则交换位置
                keyboardReport->keys[keyIndex]     ^= keyboardReport->keys[lastKeyIndex];
                keyboardReport->keys[lastKeyIndex] ^= keyboardReport->keys[keyIndex];
                keyboardReport->keys[keyIndex]     ^= keyboardReport->keys[lastKeyIndex];
                // 更新指向将被删除元素的游标
                keyIndex=lastKeyIndex;
            }
            // 删除弹起的按键
            keyboardReport->keys[keyIndex]=0x00;
        } else {
            // 没找到意味着当时是直接忽略掉的键
        }
    }
}


void standardKeyboardTransferHandler(KEY_EVENT* event,SGUI_INT* piActionId) {
    MappedKeyCodes_t* pressed   = &event->Data.stPress;
    MappedKeyCodes_t* released  = &event->Data.stRelease;
    if(pressed->length>0 && pressed->keyCodes[0] & KEY_Fn_BIT_MASK) {
        // 发现是Fn键
        stateMachine->currentState=ConsumerKeyboardStandby;
        if(pressed->length>1) {
            pressed->cursor++;
            consumerKeyboardStandbyTransferHandler(event,piActionId);
        }
        memset(standardKeyboardReport,0,sizeof(StandardKeyboardReport_t));
    } else {
        while(pressed->cursor<pressed->length) {
            insertToStandardKeyboardReport(pressed->keyCodes[pressed->cursor],standardKeyboardReport);
            pressed->cursor++;
        }
        while(released->cursor<released->length) {
            removeFromStandardKeyboardReport(released->keyCodes[released->cursor],standardKeyboardReport);
            released->cursor++;
        }
    }
    JKBD_Send((uint8_t*)standardKeyboardReport,sizeof(StandardKeyboardReport_t),ENDP1);
}
void consumerKeyboardStandbyTransferHandler (KEY_EVENT* event,SGUI_INT* piActionId) {
    MappedKeyCodes_t* released  = &event->Data.stRelease;
    if(released->length>0 && (released->keyCodes[0]&KEY_Fn_BIT_MASK)) {
        stateMachine->currentState=StandardKeyboardWorking;
        if(TIM_ScreenSaver_IsEnabled()>0){
            *piActionId = GoMenu;
        }else{
            *piActionId = TurnOnScreen;
        }

    } else {
        stateMachine->currentState=ConsumerKeyboardWorking;
        consumerKeyboardWorkingTransferHandler(event,piActionId);
    }
}
void consumerKeyboardWorkingTransferHandler (KEY_EVENT* event,SGUI_INT* piActionId) {
    MappedKeyCodes_t* pressed   = &event->Data.stPress;
    MappedKeyCodes_t* released  = &event->Data.stRelease;
    if(released->length>0 &&  (released->keyCodes[0]&KEY_Fn_BIT_MASK)) {
        stateMachine->currentState=StandardKeyboardWorking;
        standardKeyboardTransferHandler(event,piActionId);
    } else {
        while(pressed->cursor < pressed->length) {
            *consumerKeyboardReport |= (uint8_t)(((pressed->keyCodes[pressed->cursor])&0xFF0000)>>16);
            pressed->cursor++;
        }

        while(released->cursor < released->length) {
            *consumerKeyboardReport ^= (uint8_t)(((released->keyCodes[released->cursor])&0xFF0000)>>16);
            released->cursor++;
        }
        JKBD_Send((uint8_t*)consumerKeyboardReport,sizeof(ConsumerKeyboardReport_t),ENDP2);
    }
}
