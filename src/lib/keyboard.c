#include <lib/keyboard.h>
#include <string.h>
#include <stdarg.h>
#include <delay.h>
#include <usb/hw_config.h>
#include <usb_lib.h>


const uint32_t keyboardMap[6][21]= {
    //1          2          3          4          5          6          7          8          9          10         11         12         13         14         15         16         17         18         19         20         21
    //None-ESC   None-None  None-F1    None-F2    None-F3    None-F4    None-F5    None-F6    None-F7    None-F8    None-F9    None-F10   None-F11   None-F12   None-PtSr  None-ScLK  None-PAUS  None-None  None-None  None-None  None-None
    {0x00000029,0x00000000,0x0000003A,0x0000003B,0x0000003C,0x0000003D,0x0000003E,0x0000003F,0x00000040,0x00000041,0x00000042,0x00000043,0x00000044,0x00000045,0x00040046,0x00000047,0x00080048,0x00000000,0x00000000,0x00000000,0x00000000},
    //None-`     None-1!    None-2@    None-3#    None-4$    None-5%    None-6^    None-7&    None-8*    None-9(    None-0)    None--_    None-=+    None-BS    None-Inst  None-Home  None-PgUp  None-NuLk  None-/     None-*     None--
    {0x00000035,0x0000001E,0x0000001F,0x00000020,0x00000021,0x00000022,0x00000023,0x00000024,0x00000025,0x00000026,0x00000027,0x0000002D,0x0000002E,0x0000002A,0x00000049,0x0000004A,0x0000004B,0x00000053,0x00000054,0x00000055,0x00000056},
    //None-Tab   None-Q     None-W     None-E     None-R     None-T     None-Y     None-U     None-I     None-O     None-P     None-[{    None-]}    None-\|    None-DELE  None-End   None-PgDn  None-Num7  None-Num8  None-Num9  None-None
    {0x0000002B,0x00000014,0x0000001A,0x00000008,0x00000015,0x00000017,0x0000001C,0x00000018,0x0000000C,0x00000012,0x00000013,0x0000002F,0x00000030,0x00000031,0x0000004C,0x0000004D,0x0000004E,0x0000005F,0x00000060,0x00000061,0x00000027},
    //None-Cap   None-A     None-S     None-D     None-F     None-G     None-H     None-J     None-K     None-L     None-;:    None-'"    None-None  None-ENTR  None-None  None-None  None-None  None-Num4  None-NUM5  None-Num6  None-NPls
    {0x00000039,0x00000004,0x00000016,0x00000007,0x00000009,0x0000000A,0x0000000B,0x0000000D,0x0000000E,0x0000000F,0x00000033,0x00000034,0x00000000,0x00000028,0x00000000,0x00000000,0x00000000,0x0000005C,0x0000005D,0x0000005E,0x00000057},
    //None-LSft  None-Z     None-X     None-C     None-V     None-B     None-N     None-M     None-,<    None-.>    None-/?    None-None  None-RSft  None-None  None-None  None-Up    None-None  None-Num1  None-Num2  None-Num3  None-None
    {0x000000E1,0x0000001D,0x0000001B,0x00000006,0x00000019,0x00000005,0x00000011,0x00000010,0x00000036,0x00000037,0x00000038,0x00000000,0x000000E5,0x00000000,0x00000000,0x00100052,0x00000000,0x00000059,0x0000005A,0x0000005B,0x00000027},
    //None-LCtl  None-LWin  None-LAlt  None-None  None-None  None-Spac  None-None  None-None  None-None  None-LAlt  Fn         None-Cont  None-RCtl  None-None  None-Left  None-Down  None-Rigt  None-Num0  None-None  None-.Del  None-NEnt
    {0x000000E0,0x000000E3,0x000000E2,0x00000000,0x00000000,0x0000002C,0x00000000,0x00000000,0x00000000,0x000000E6,0x80000000,0x00000065,0x000000E4,0x00000000,0x00020050,0x00200051,0x0001004F,0x00000062,0x00000000,0x00000063,0x00000058}
};

static HidContext_t* pstContext=NULL;

void KBDLib_Init(){
    pstContext = pvPortMalloc(sizeof(HidContext_t));
    memset(pstContext,0,sizeof(HidContext_t));
}
// 模拟标准键盘
void KBDLib_PressStdKey(KeyboardUsageCode_t keyCode){
    if(keyCode>=KeyLeftCtrl){
        // 控制键
        pstContext->standardKeyboardReport.controlKeys |= (0x01<<(keyCode-KeyLeftCtrl));
        pstContext->syncFlags |= SYNC_FLAG_STDKBD;
    }else{
        // 普通键
        uint8_t uiIndex=0;
        while(uiIndex<6 && pstContext->standardKeyboardReport.keys[uiIndex]){
            uiIndex++;
        }
        if(uiIndex<6){
            pstContext->standardKeyboardReport.keys[uiIndex] = keyCode;
            pstContext->syncFlags |= SYNC_FLAG_STDKBD;
        }
    }
}
void KBDLib_ReleaseStdKey(KeyboardUsageCode_t keyCode){
    if(keyCode>=KeyLeftCtrl){
        // 控制键
        pstContext->standardKeyboardReport.controlKeys ^= (0x01<<(keyCode-KeyLeftCtrl));
        pstContext->syncFlags |= SYNC_FLAG_STDKBD;
    }else{
        // 普通键
        uint8_t uiIndex = 0;
        // 找到第一个和目标键相等的键
        while(uiIndex<6 && pstContext->standardKeyboardReport.keys[uiIndex]!=keyCode){
            uiIndex++;
        }
        if(uiIndex<6){
            // 找到一个相同的
            uint8_t uiLastIndex = 5;
            // 找到最后一个非空的键
            while(uiLastIndex>uiIndex && pstContext->standardKeyboardReport.keys[uiLastIndex]==0x00){
                uiLastIndex--;
            }
            if(uiLastIndex!=uiIndex) {
                // 二者不等则交换位置
                pstContext->standardKeyboardReport.keys[uiIndex]     ^= pstContext->standardKeyboardReport.keys[uiLastIndex];
                pstContext->standardKeyboardReport.keys[uiLastIndex] ^= pstContext->standardKeyboardReport.keys[uiIndex];
                pstContext->standardKeyboardReport.keys[uiIndex]     ^= pstContext->standardKeyboardReport.keys[uiLastIndex];
                // 更新指向将被删除元素的游标
                uiIndex=uiLastIndex;
            }
            // 删除弹起的按键
            pstContext->standardKeyboardReport.keys[uiIndex]=0x00;
            pstContext->syncFlags |= SYNC_FLAG_STDKBD;
        }
    }
}
void KBDLib_PressStdKeys(MappedKeyCodes_t* pstKeyCodes){
    while(pstKeyCodes->cursor<pstKeyCodes->length){
        KBDLib_PressStdKey(pstKeyCodes->keyCodes[pstKeyCodes->cursor++]&0xFFFF);
    }
}
void KBDLib_ReleaseStdKeys(MappedKeyCodes_t* pstKeyCodes){
    while(pstKeyCodes->cursor<pstKeyCodes->length){
        KBDLib_ReleaseStdKey(pstKeyCodes->keyCodes[pstKeyCodes->cursor++]&0xFFFF);
    }
}
void KBDLib_ReleaseAllStdKeys(){
    uint8_t* pCharOfReport = (uint8_t*)&pstContext->standardKeyboardReport;
    for(uint8_t i=0;i<sizeof(StandardKeyboardReport_t);i++){
        if(pCharOfReport[i]){
            memset(&pstContext->standardKeyboardReport,0,sizeof(StandardKeyboardReport_t));
            pstContext->syncFlags |= SYNC_FLAG_STDKBD;
            break;
        }
    }
}
// 虚拟扩展键盘
void KBDLib_PressExtKey(uint8_t keyCode){
    pstContext->consumerKeyboardReport |= keyCode;
    pstContext->syncFlags |= SYNC_FLAG_EXTKBD;
}
void KBDLib_ReleaseExtKey(uint8_t keyCode){
    pstContext->consumerKeyboardReport ^= keyCode;
    pstContext->syncFlags |= SYNC_FLAG_EXTKBD;
}
void KBDLib_PressExtKeys(MappedKeyCodes_t* pstKeyCodes){
    while(pstKeyCodes->cursor<pstKeyCodes->length){
        KBDLib_PressExtKey((pstKeyCodes->keyCodes[pstKeyCodes->cursor++]>>16)&0xFF);
    }
}
void KBDLib_ReleaseExtKeys(MappedKeyCodes_t* pstKeyCodes){
    while(pstKeyCodes->cursor<pstKeyCodes->length){
        KBDLib_ReleaseExtKey((pstKeyCodes->keyCodes[pstKeyCodes->cursor++]>>16)&0xFF);
    }
}
void KBDLib_ReleaseAllExtKeys(){
    if(pstContext->consumerKeyboardReport){
        memset(&pstContext->consumerKeyboardReport,0,sizeof(ConsumerKeyboardReport_t));
        pstContext->syncFlags |= SYNC_FLAG_EXTKBD;
    }
}
void KBDLib_SyncState(){
    // 标准键盘更新了
    if(pstContext->syncFlags & SYNC_FLAG_STDKBD){
        while(GetEPTxStatus(ENDP1) != EP_TX_NAK);
        UserToPMABufferCopy((uint8_t*)&pstContext->standardKeyboardReport,GetEPTxAddr(ENDP1),sizeof(StandardKeyboardReport_t));
        SetEPTxValid(ENDP1);
        pstContext->syncFlags ^= SYNC_FLAG_STDKBD;
    }
    // 消费键盘更新了
    if(pstContext->syncFlags & SYNC_FLAG_EXTKBD){
        while(GetEPTxStatus(ENDP2) != EP_TX_NAK);
        UserToPMABufferCopy((uint8_t*)&pstContext->consumerKeyboardReport,GetEPTxAddr(ENDP2),sizeof(ConsumerKeyboardReport_t));
        SetEPTxValid(ENDP2);
        pstContext->syncFlags ^= SYNC_FLAG_EXTKBD;
    }
}

void mapKeyCodes(KeyUpdateInfo_t* pCurrent,uint32_t* pKeyCode) {
    uint32_t* pFirstKeyCode=pKeyCode;
    while(NULL != pCurrent) {
        *pKeyCode=keyboardMap[pCurrent->row][pCurrent->column];
        if((*pKeyCode & KEY_Fn_BIT_MASK ) && (pKeyCode!=pFirstKeyCode)) {
            *pKeyCode       = *pKeyCode ^ *pFirstKeyCode;
            *pFirstKeyCode  = *pKeyCode ^ *pFirstKeyCode;
            *pKeyCode       = *pKeyCode ^ *pFirstKeyCode;
        }
        pKeyCode++;
        pCurrent=pCurrent->next;
    }
}

bool containsKey(MappedKeyCodes_t* mappedKeyCodes,KeyboardUsageCode_t keyCode) {
    for(int i=0; i<mappedKeyCodes->length; i++) {
        if((KeyboardUsageCode_t)(mappedKeyCodes->keyCodes[i] & 0xFF) == keyCode) {
            return true;
        }
    }
    return false;
}
bool containsKeys(MappedKeyCodes_t* mappedKeyCodes,KeyboardUsageCode_t* pKeyCodeFound,uint8_t checkLength,KeyboardUsageCode_t keyCode,...) {
    va_list nextCode;
    va_start(nextCode,keyCode);
    *pKeyCodeFound = 0;
	for(int i=0; i<checkLength; i++) {
		for(int j=0; j<mappedKeyCodes->length; j++) {
			if((KeyboardUsageCode_t)(mappedKeyCodes->keyCodes[j] & 0xFF) == keyCode) {
				*pKeyCodeFound = keyCode;
				i=checkLength;
				break;
			}
        }
        keyCode = va_arg(nextCode,int);
    }
    va_end(nextCode);
    return *pKeyCodeFound!=0;
}

void sendKeysToHost(uint8_t* pKeys,uint8_t uiLength){
    KBDLib_ReleaseAllStdKeys();
    KBDLib_SyncState();
    while(uiLength-->0){
        KBDLib_PressStdKey(*pKeys);
        KBDLib_SyncState();
        KBDLib_ReleaseStdKey(*pKeys);
        KBDLib_SyncState();
        pKeys++;
    }
}
