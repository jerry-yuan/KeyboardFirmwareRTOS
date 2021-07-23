#ifndef LIB_KEYBOARD_H_INCLUDED
#define LIB_KEYBOARD_H_INCLUDED

#include <stm32f10x.h>

#include <task/keyscan.h>

#include <screen/consts.h>

#define KEY_Fn_BIT_MASK 0x80000000

// HID键码

#define NORMAL_KEY  0x0400
#define CONTROL_KEY 0x0500

#define CKEY        0x0100

#define SYNC_FLAG_STDKBD    0x01<<0
#define SYNC_FLAG_EXTKBD    0x01<<1

typedef enum {
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
    KeyNumBinary,KeyNumOctal,KeyNumDecimal,KeyNumHexadecimal,
    KeyLeftCtrl=0xE0,KeyLeftShift,KeyLeftAlt,KeyLeftGUI,
    KeyRightCtrl,KeyRightShift,KeyRightAlt,KeyRightGUI
} KeyboardUsageCode_t;

typedef struct {
    uint8_t   length;
    uint8_t   cursor;
    uint32_t* keyCodes;
} MappedKeyCodes_t;
// HID 报文
typedef struct {
    uint8_t controlKeys;
    uint8_t reserved;
    uint8_t keys[6];
} StandardKeyboardReport_t;

typedef uint8_t ConsumerKeyboardReport_t;
// HID虚拟设备状态
typedef struct{
    uint8_t syncFlags;
    // Standard Keyboard
    StandardKeyboardReport_t standardKeyboardReport;
    // Consumer Keyboard
    ConsumerKeyboardReport_t consumerKeyboardReport;
} HidContext_t;

void KBDLib_Init();

void KBDLib_PressStdKey(KeyboardUsageCode_t keyCode);
void KBDLib_ReleaseStdKey(KeyboardUsageCode_t keyCode);
void KBDLib_PressStdKeys(MappedKeyCodes_t* keyCodes);
void KBDLib_ReleaseStdKeys(MappedKeyCodes_t* keyCodes);


void KBDLib_PressExtKey(KeyboardUsageCode_t keyCode);
void KBDLib_ReleaseExtKey(KeyboardUsageCode_t keyCode);
void KBDLib_PressExtKeys(MappedKeyCodes_t* keyCodes);
void KBDLib_ReleaseExtKeys(MappedKeyCodes_t* keyCodes);

void KBDLib_SyncState();

void mapKeyCodes(KeyUpdateInfo_t* pCurrent,uint32_t* pKeyCode);
bool containsKey(MappedKeyCodes_t* mappedKeyCodes,KeyboardUsageCode_t keyCode);
bool containsKeys(MappedKeyCodes_t* mappedKeyCodes,KeyboardUsageCode_t* pKeyCodeFound,uint8_t checkLength,KeyboardUsageCode_t keyCode,...);
void sendKeysToHost(uint8_t* pKeys,uint8_t uiLength);
#endif /* KEYBOARD_H_INCLUDED */
