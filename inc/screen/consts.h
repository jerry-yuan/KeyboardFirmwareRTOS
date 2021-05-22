#ifndef CONSTS_H_INCLUDED
#define CONSTS_H_INCLUDED

#include <task/keyscan.h>

#include <HMI_Engine.h>

enum ScreenId {
    SCREEN_Init_ID  = 1,
    SCREEN_USB_State_ID,
    SCREEN_Keyboard_State_ID,
    SCREEN_Menu_ID,
    SCREEN_Clock_Show_ID,
    SCREEN_Clock_Edit_ID,
    SCREEN_Calculator_ID,
    SCREEN_TOTPAuth_Show_ID
};

enum EventId {
    USB_STATE_EVENT_ID    = 1,
    KEY_EVENT_ID,
    KEYBOARD_STATE_EVENT_ID,
    RTC_EVENT_ID,
    KEY_REPEAT_EVENT_ID
};

// Events
typedef struct {
    uint32_t uiDeviceState;
} USB_STATE_EVENT_DATA;

HMI_EVENT_TYPE_DECLARE(USB_STATE_EVENT,USB_STATE_EVENT_DATA)

typedef struct {
    KeyUpdateInfo_t* pstPressed;
    uint8_t uiPressedCount;
    KeyUpdateInfo_t* pstRelease;
    uint8_t uiReleaseCount;
} KEY_EVENT_DATA;

HMI_EVENT_TYPE_DECLARE(KEY_EVENT,KEY_EVENT_DATA)
HMI_EVENT_TYPE_DECLARE(KEY_REPEAT_EVENT,char)

HMI_EVENT_TYPE_DECLARE(KEYBOARD_STATE_EVENT,uint32_t)
HMI_EVENT_TYPE_DECLARE(RTC_EVENT,uint8_t)

#endif /* CONSTS_H_INCLUDED */
