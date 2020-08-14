#ifndef KEYBOARDSCR_H_INCLUDED
#define KEYBOARDSCR_H_INCLUDED

#include <HMI_Engine.h>
#include <consts.h>

/** 键盘状态 **/
#define KEYBOARD_STATE_NUMLOCK_MASK     0x01
#define KEYBOARD_STATE_CAPSLOCK_MASK    0x02
#define KEYBOARD_STATE_SCROLLLOCK_MASK  0x04

typedef struct {
    uint8_t controlKeys;
    uint8_t reserved;
    uint8_t keys[6];
} StandardKeyboardReport_t;

typedef uint8_t ConsumerKeyboardReport_t;

typedef enum{
    StandardKeyboardWorking=0x00,
    ConsumerKeyboardStandby,
    ConsumerKeyboardWorking
} KeyboardState_t;

typedef struct {
    KeyboardState_t currentState;
    void (*transferHandler[3])(KEY_EVENT* event,SGUI_INT* piActionID);
} KeyboardStateMachine_t;

extern HMI_SCREEN_OBJECT SCREEN_Keyboard;

#endif /* KEYBOARDSCR_H_INCLUDED */
