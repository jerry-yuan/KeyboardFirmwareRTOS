#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <stm32f10x.h>
#include <FreeRTOS.h>

#include <task.h>
#include <queue.h>
#include <event_groups.h>

#define KEY_Fn_BIT_MASK 0x80000000

// HID键类

#define NORMAL_KEY  0x0400
#define CONTROL_KEY 0x0500

#define CKEY        0x0100

enum KeyboardUsageCode {
    KeyEmpty=0x00,
    KeyErrorRollOver,KeyPOSTFail,KeyErrorUndefined,
    KeyA,KeyB,KeyC,KeyD,KeyE,KeyF,KeyG,KeyH,KeyI,KeyJ,KeyK,KeyL,KeyM,
    KeyN,KeyO,KeyP,KeyQ,KeyR,KeyS,KeyT,KeyU,KeyV,KeyW,KeyX,KeyY,KeyZ,
    Key1,Key2,Key3,Key4,Key5,Key6,Key7,Key8,Key9,Key0,
    KeyEnter,KeyEscape,KeyDelete,KeyTab,KeySpaceBar,KeyMinus,KeyEquals,
    KeyLeftSquareBracket,KeyRightSquareBracket,
    KeyBackSlash,KeyNonUsHash,KeySemicon,KeyQuotationMark,KeyGraveAccentAndTilde,
    KeyComma,KeyDot,KeySlash,KeyCapsKeyLock,
    KeyF1,KeyF2,KeyF3,KeyF4,KeyF5,KeyF6,KeyF7,KeyF8,KeyF9,KeyF10,KeyF11,KeyF12,
    KeyPrintScreen,KeyScrollLock,KeyPause,
    KeyInsert,KeyHome,KeyPageUp,KeyDeleteForward,KeyEnd,KeyPageDown,
    KeyRight,KeyLeft,KeyDown,KeyUp,
    KeyNumLock,KeyNumSlash,KeyNumStar,KeyNumMinus,KeyNumPlus,KeyNumEnter,
    KeyNum1,KeyNum2,KeyNum3,KeyNum4,KeyNum5,KeyNum6,KeyNum7,KeyNum8,KeyNum9,KeyNum0,
    KeyNumDelete,KeyNonUsSlash,KeyApplication,KeyPower,KeyNumEquals,
    KeyF13,KeyF14,KeyF15,KeyF16,KeyF17,KeyF18,KeyF19,KeyF20,KeyF21,KeyF22,KeyF23,KeyF24,
    KeyExecute,KeyHelp,KeyMenu,KeySelect,KeyStop,KeyAgain,
    KeyUndo,KeyCut,KeyCopy,KeyPaste,KeyFind,
    KeyMute,KeyVolumeUp,KeyVolumeDown,
    KeyLockCapsLock,KeyLockNumLock,KeyLockScrollLock,
    KeyNumComma,KeyNumEqualsSign,
    KeyInternational1,KeyInternational2,KeyInternational3,KeyInternational4,KeyInternational5,
    KeyInternational6,KeyInternational7,KeyInternational8,KeyInternational9,
    KeyLANG1,KeyLANG2,KeyLANG3,KeyLANG4,KeyLANG5,KeyLANG6,KeyLANG7,KeyLANG8,KeyLANG9,
    KeyAlternateErase,KeySysReq,KeyCancel,KeyClear,KeyPrior,KeyReturn,KeySeparator,KeyOut,
    KeyOper,KeyClearAgain,KeyCrSelProps,KeyExSel,
    KeyNum00=0xB0,KeyNum000,
    KeyThousandsSeparator,KeyDecimalSeparator,KeyCurrencyUnit,KeyCurrencySubUnit,
    KeyNumLeftBrace,KeyNumRightBrace,KeyNumLeftCurlyBrace,KeyNumRightCurlyBrace,
    KeyNumTab,KeyNumBackspace,KeyNumA,KeyNumB,KeyNumC,KeyNumE,KeyNumD,KeyNumF,
    KeyNumXOR,KeyNumPower,KeyNumPercent,KeyNumLess,KeyNumLarger,KeyNumAnd,KeyNumDbAnd,KeyNumOr,KeyNumDbOr,
    KeyNumColon,KeyNumHash,KeyNumSpace,KeyNumAt,KeyNumExclamationMark,
    KeyNumMemoryStore,KeyNumMemoryReset,KeyNumMemoryAdd,KeyNumMemorySubtract,KeyNumMemoryMultiply,KeyNumMemoryDivide,
    KeyNumNegativeSwitch,KeyNumClear,KeyNumClearEntry,
    KeyNumBinary,KeyNumOctal,KeyNumDecimal,KeyNumHexadecimal
};

#define KEY_STATE_EVENT_UPDATE          (1<<23)

extern TaskHandle_t hKeyboardTask;
extern TaskHandle_t hKeyboardStatTask;

extern EventGroupHandle_t hKeyboardStateUpdateEvent;
extern uint32_t keyboardStatus;
void keyboardTaskInitialize();



#endif /* KEYBOARD_H_INCLUDED */
