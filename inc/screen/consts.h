#ifndef CONSTS_H_INCLUDED
#define CONSTS_H_INCLUDED
#include <HMI_Engine.h>
enum ScreenId {
    InitScreen_ID  = 1,
    USBScreen_ID,
    KeyboardScreen_ID,
    MenuScreen_ID,
    SleepScreen_ID
};

enum EventId {
    USB_STATE_EVENT_ID    = 1,
    KEY_EVENT_ID,
    KEYBOARD_STATE_EVENT_ID
};

// Events
typedef struct {
    uint32_t uiDeviceState;
} USB_STATE_EVENT_DATA;

HMI_EVENT_TYPE_DECLARE(USB_STATE_EVENT,USB_STATE_EVENT_DATA)

typedef struct {
    uint8_t   length;
    uint8_t   cursor;
    uint32_t* keyCodes;
} MappedKeyCodes_t;

typedef struct {
    MappedKeyCodes_t stPress;
    MappedKeyCodes_t stRelease;
} KEY_EVENT_DATA;

HMI_EVENT_TYPE_DECLARE(KEY_EVENT,KEY_EVENT_DATA)

HMI_EVENT_TYPE_DECLARE(KEYBOARD_STATE_EVENT,uint32_t)

#endif /* CONSTS_H_INCLUDED */
