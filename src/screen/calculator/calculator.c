#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <SGUI_Basic.h>
#include <SGUI_Notice.h>
#include <SGUI_IconResource.h>
#include <screen/calculator/calculator.h>

#include <lib/keyboard.h>

#include <resources/Font.h>

#define INPUT_BUFFER_SIZE			10
#define DISPLAY_BUFFER_SIZE			20

#define ACTION_MASK_REFRESH 	0x80

typedef struct {
    int64_t iValue;
    uint8_t uiShift;
} AccurateFloatNumber_t;

typedef enum {
	NoOperate   = 0x00,
    Divide 		= 0x01,
    Multiply 	= 0x02,
    Minus 		= 0x03,
    Plus 		= 0x04,
    Equals		= 0x05
} CalculateAction_t ;

typedef enum {
    InitialState = 0x00,
    InputState   = 0x01,
    ResultState  = 0x02,
    ErrorState   = 0x03,
} CalculatorStates_t;

typedef struct {
    char* pcBuffer;
    char* pcBufferCursor;
    CalculateAction_t ePrevAction;
    AccurateFloatNumber_t stPrevNumber;
    CalculatorStates_t eState;
    const char* pcLastError;
    void (*pfActionHandlers[4])(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);
    void (*pfRenders[4])(SGUI_SCR_DEV* pstDeviceIF);
    void (*pfDoCalcAction[6])(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage);
} CalculatorStateMachine_t;

enum {
    NoAction      = 0x00,
    GoBack        = 0x01,
    RefreshScreen = 0x80,
    ResetCalc	  = 0x81
};

static CalculatorStateMachine_t* pstStateMachine	= NULL;

/*static char* pcBuffer 					= NULL;
static char* pcBufferCursor 			= NULL;
static uint8_t uiFlags					= 0x00;
static AccurateFloatNumber_t* pstResult = NULL;*/

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

static void initialStateRender(SGUI_SCR_DEV* pstDeviceIF);
static void resultStateRender(SGUI_SCR_DEV* pstDeviceIF);
static void inputStateRender(SGUI_SCR_DEV* pstDeviceIF);
static void errorStateRender(SGUI_SCR_DEV* pstDeviceIF);

static void initialStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);
static void resultStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);
static void inputStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);
static void errorStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);

static void resetCalculator();

static void strToNumber(char* pBuffer,AccurateFloatNumber_t* pstNumber);
static void numberToStr(char* pBuffer,AccurateFloatNumber_t* pstNumber);
static void reverseStr(char* pBuffer,uint8_t from,uint8_t to);
static void alineNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB);
static void simplifyNumber(AccurateFloatNumber_t* pstNumber);

static void plusCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage);
static void minusCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage);
static void multiplyCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage);
static void divideCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage);
static void equalsCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage);

static HMI_SCREEN_ACTION screenActions= {
    Initialize,
    Prepare,
    Refresh,
    ProcessEvent,
    PostProcess
};

HMI_SCREEN_OBJECT SCREEN_Calculator= {SCREEN_Calculator_ID,&screenActions};

HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    pstStateMachine = pvPortMalloc(sizeof(CalculatorStateMachine_t));
    memset(pstStateMachine,0,sizeof(CalculatorStateMachine_t));

    pstStateMachine->pcBuffer 		= pvPortMalloc(sizeof(char)*INPUT_BUFFER_SIZE);
    pstStateMachine->pcBufferCursor = pstStateMachine->pcBuffer;
    memset(pstStateMachine->pcBuffer,0,sizeof(char)*INPUT_BUFFER_SIZE);

    pstStateMachine->pfRenders[InitialState] = initialStateRender;
    pstStateMachine->pfRenders[ResultState]  = resultStateRender;
    pstStateMachine->pfRenders[InputState]   = inputStateRender;
    pstStateMachine->pfRenders[ErrorState]   = errorStateRender;

    pstStateMachine->pfActionHandlers[InitialState] = initialStateHandler;
    pstStateMachine->pfActionHandlers[ResultState]  = resultStateHandler;
    pstStateMachine->pfActionHandlers[InputState]   = inputStateHandler;
    pstStateMachine->pfActionHandlers[ErrorState]   = errorStateHandler;

	pstStateMachine->pfDoCalcAction[NoOperate] = equalsCalcAction;
    pstStateMachine->pfDoCalcAction[Divide] = divideCalcAction;
    pstStateMachine->pfDoCalcAction[Multiply] = multiplyCalcAction;
    pstStateMachine->pfDoCalcAction[Minus] = minusCalcAction;
    pstStateMachine->pfDoCalcAction[Plus] = plusCalcAction;
    pstStateMachine->pfDoCalcAction[Equals] = equalsCalcAction;


    resetCalculator();

    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_Basic_ClearScreen(pstDeviceIF);

	resetCalculator();

    Refresh(pstDeviceIF,NULL);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_Basic_DrawRectangle(pstDeviceIF,5,5,256-10,64-10,0x0A,0x00);
    pstStateMachine->pfRenders[pstStateMachine->eState](pstDeviceIF);
    return HMI_RET_NORMAL;
//    char cToDisplay[DISPLAY_BUFFER_SIZE]= {0};
//
//    const char* cOperators[5]= {"=","÷","×","-","+"};
//    const char* cOperator = "";
//    SGUI_RECT stRect;
//    SGUI_POINT stPoint;
//    SGUI_AREA_SIZE stArea;
//
//    SGUI_Basic_DrawRectangle(pstDeviceIF,5,5,256-10,64-10,0x0A,0x00);
//
//    stPoint.iX=stPoint.iY=0;
//    if(uiFlags & FLAG_MASK_HAS_ERROR) {
//        stRect.iX = 10;
//        stRect.iY = 10;
//        stRect.iHeight = 44;
//        stRect.iWidth  = 246;
//        SGUI_Text_DrawMultipleLinesText(pstDeviceIF,"产生错误:\n请检查是不是他喵的除了个0?",SGUI_FONT_REF(Deng12),&stRect,0,0x0F);
//        return HMI_RET_NORMAL;
//    } else if(pcBuffer!=pcBufferCursor) {
//        sprintf(cToDisplay,"%s%c",pcBuffer,uiFlags&FLAG_MASK_HAS_DOT?'\0':'.');
//    } else if(uiFlags & FLAG_MASK_HAS_RESULT) {
//        numberToStr(cToDisplay,pstResult);
//        cOperator=cOperators[((uiFlags & FLAG_MASK_PREV_CALC)>>4) & 0x0f];
//    } else {
//        sprintf(cToDisplay,"0.");
//    }
//    SGUI_Text_GetTextExtent(cToDisplay,SGUI_FONT_REF(LCD44),&stArea);
//    stRect.iX = 236 - stArea.iWidth;
//    stRect.iY = 10;
//    stRect.iHeight = stArea.iHeight;
//    stRect.iWidth  = stArea.iWidth;
//    SGUI_Text_DrawText(pstDeviceIF,cToDisplay,SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
//    SGUI_Text_GetTextExtent(cOperator,SGUI_FONT_REF(LCD44),&stArea);
//    stRect.iX = stRect.iY = 10;
//    stRect.iHeight = stArea.iHeight;
//    stRect.iWidth  = stArea.iWidth;
//    SGUI_Text_DrawText(pstDeviceIF,cOperator,SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
//
//    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    KEY_EVENT* pstKeyEvent;
    MappedKeyCodes_t stPressed,stRelease;
    *piActionID = NoAction;
    if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        pstKeyEvent = (KEY_EVENT*)pstEvent;
        stPressed.cursor    = 0;
        stPressed.length	= pstKeyEvent->Data.uiPressedCount;
        stPressed.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stPressed.length);
        mapKeyCodes(pstKeyEvent->Data.pstPressed,stPressed.keyCodes);

        stRelease.cursor    = 0;
        stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stRelease.length);
        mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);
        if(containsKey(&stPressed,KeyNumLock)) {
            *piActionID = ResetCalc;
        } else if(containsKey(&stRelease,KeyEscape)) {
            *piActionID = GoBack;
        } else {
            pstStateMachine -> pfActionHandlers[pstStateMachine->eState](&stPressed,&stRelease,piActionID);
        }

//        if(stPressed.keyCodes[0]<=Key0 && stPressed.keyCodes[0]>=Key1) {
//            *piActionID = ACTION_MASK_INPUT | ACTION_MASK_REFRESH | (((stPressed.keyCodes[0]-Key1 + 1)%10)&0x0F);
//        } else if(stPressed.keyCodes[0]<=KeyNum0 && stPressed.keyCodes[0]>=KeyNum1) {
//            *piActionID = ACTION_MASK_INPUT | ACTION_MASK_REFRESH | (((stPressed.keyCodes[0]-KeyNum1 + 1)%10)&0x0F);
//        } else if(containsKeys(&stPressed,&keyCode,2,KeyDot,KeyNumDelete)) {
//            *piActionID = InputDot;
//        } else if(containsKeys(&stPressed,&keyCode,2,KeyDelete,KeyDeleteForward)) {
//            *piActionID = DoBackSpace;
//        } else if(stPressed.keyCodes[0]<=KeyNumEnter && stPressed.keyCodes[0]>=KeyNumSlash) {
//            *piActionID = ACTION_MASK_CALCULATE | ACTION_MASK_REFRESH | ((stPressed.keyCodes[0]-KeyNumSlash)&0x0F);
//        } else if(containsKey(&stPressed,KeyEnter)) {
//            *piActionID = DoEquals;
//        } else if(containsKey(&stPressed,KeyNumLock)) {
//            *piActionID = DoClear;
//        } else if(containsKey(&stRelease,KeyEscape)) {
//            *piActionID = GoBack;
//        }

        vPortFree(stPressed.keyCodes);
        vPortFree(stRelease.keyCodes);

    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID==GoBack) {
        HMI_GoBack(NULL);
    } else if(iActionID == ResetCalc) {
        resetCalculator();
    }
    if(iActionID & ACTION_MASK_REFRESH) {
        Refresh(pstDeviceIF,NULL);
    }
    return HMI_RET_NORMAL;
}

static void initialStateRender(SGUI_SCR_DEV* pstDeviceIF) {
    SGUI_RECT stRect;
    SGUI_POINT stPoint;
    SGUI_AREA_SIZE stArea;

    SGUI_Text_GetTextExtent("0.",SGUI_FONT_REF(LCD44),&stArea);
    stPoint.iX = stPoint.iY = 0;
    stRect.iX = 236-stArea.iWidth;
    stRect.iY = 10;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Text_DrawText(pstDeviceIF,"0.",SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
}
static void resultStateRender(SGUI_SCR_DEV* pstDeviceIF) {
    SGUI_RECT stRect;
    SGUI_POINT stPoint;
    SGUI_AREA_SIZE stArea;
    char pcBuffer[DISPLAY_BUFFER_SIZE]= {0};
    const char* operators[]= {"","÷","×","-","+","="};

    stPoint.iX = stPoint.iY = 0;

    numberToStr(pcBuffer,&pstStateMachine->stPrevNumber);

    // draw number
    SGUI_Text_GetTextExtent(pcBuffer,SGUI_FONT_REF(LCD44),&stArea);
    stRect.iX = 236-stArea.iWidth;
    stRect.iY = 10;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Text_DrawText(pstDeviceIF,pcBuffer,SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
    // draw Operator

    SGUI_Text_GetTextExtent(operators[pstStateMachine->ePrevAction],SGUI_FONT_REF(LCD44),&stArea);
    stRect.iX = 10;
    stRect.iY = 10;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Text_DrawText(pstDeviceIF,operators[pstStateMachine->ePrevAction],SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
}
static void inputStateRender(SGUI_SCR_DEV* pstDeviceIF) {
    SGUI_RECT stRect;
    SGUI_POINT stPoint;
    SGUI_AREA_SIZE stArea;
    char cBuffer[DISPLAY_BUFFER_SIZE]= "0.";
    if(pstStateMachine->pcBuffer != pstStateMachine->pcBufferCursor) {
        sprintf(cBuffer,"%s%s",pstStateMachine->pcBuffer,strstr(pstStateMachine->pcBuffer,".")?"":".");
    }

    SGUI_Text_GetTextExtent(cBuffer,SGUI_FONT_REF(LCD44),&stArea);
    stPoint.iX = stPoint.iY = 0;
    stRect.iX = 236-stArea.iWidth;
    stRect.iY = 10;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Text_DrawText(pstDeviceIF,cBuffer,SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
}
static void errorStateRender(SGUI_SCR_DEV* pstDeviceIF) {
    SGUI_NOTICE_BOX stNoticeBox;
    stNoticeBox.pstIcon = &SGUI_RES_ICON_ERROR_16;
    stNoticeBox.cszNoticeText = pstStateMachine->pcLastError;
    stNoticeBox.stPalette.eEdgeColor = 0x0F;
    stNoticeBox.stPalette.eFillColor = 0x02;
    stNoticeBox.stPalette.eTextColor = 0x0F;
    stNoticeBox.stPalette.uiDepthBits = 4;
    stNoticeBox.stLayout.iX = 10;
    stNoticeBox.stLayout.iY = 10;
    stNoticeBox.stLayout.iWidth = 256-20;
    stNoticeBox.stLayout.iHeight = 64-20;
    SGUI_Notice_Repaint(pstDeviceIF,&stNoticeBox,SGUI_FONT_REF(Deng12),0);
}

static void initialStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {
    KeyboardUsageCode_t keyCode;
    if((pstPressed->keyCodes[0]<=Key0 && pstPressed->keyCodes[0]>=Key1) || (pstPressed->keyCodes[0]<=KeyNum0 && pstPressed->keyCodes[0]>=KeyNum1) || containsKeys(pstPressed,&keyCode,2,KeyDot,KeyNumDelete)) {
        pstStateMachine -> eState = InputState;
        inputStateHandler(pstPressed,pstRelease,piActionID);
    } else if(pstPressed->keyCodes[0]<=KeyNumEnter && pstPressed->keyCodes[0]>=KeyNumSlash) {
        pstStateMachine -> eState = ResultState;
        pstStateMachine -> ePrevAction = pstPressed->keyCodes[0] - KeyNumSlash+1;
        *piActionID = RefreshScreen;
    }
}
static void resultStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {
    KeyboardUsageCode_t keyCode;
    if(pstPressed->keyCodes[0]<=KeyNumEnter && pstPressed->keyCodes[0]>=KeyNumSlash) {
        pstStateMachine->ePrevAction = pstPressed->keyCodes[0] - KeyNumSlash+1;
        *piActionID = RefreshScreen;
    } else if((pstPressed->keyCodes[0]<=Key0 && pstPressed->keyCodes[0]>=Key1) || (pstPressed->keyCodes[0]<=KeyNum0 && pstPressed->keyCodes[0]>=KeyNum1) || containsKeys(pstPressed,&keyCode,2,KeyDot,KeyNumDelete)) {
        pstStateMachine->eState = InputState;
        inputStateHandler(pstPressed,pstRelease,piActionID);
    }
}
static void inputStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {
    char nextChar = '\0';
    KeyboardUsageCode_t keyCode;
    const char* pcErrorMessage=NULL;
    AccurateFloatNumber_t stNumber;
    if(pstPressed->keyCodes[0]<=KeyNumEnter && pstPressed->keyCodes[0]>=KeyNumSlash) {
        strToNumber(pstStateMachine->pcBuffer,&stNumber);
        *(pstStateMachine->pcBufferCursor = pstStateMachine->pcBuffer) = '\0';

        pstStateMachine->pfDoCalcAction[pstStateMachine->ePrevAction](&pstStateMachine->stPrevNumber,&stNumber,&pcErrorMessage);
        if(pcErrorMessage==NULL) {
            pstStateMachine->eState      = ResultState;
            pstStateMachine->ePrevAction = pstPressed->keyCodes[0]-KeyNumSlash+1;
        } else {
            pstStateMachine->eState      = ErrorState;
            pstStateMachine->pcLastError = pcErrorMessage;
        }

        *piActionID = RefreshScreen;
    } else {
        if(pstPressed->keyCodes[0]<=Key0 && pstPressed->keyCodes[0]>=Key1) {
            nextChar = '0'+(pstPressed->keyCodes[0]-Key1+1)%10;
        } else if(pstPressed->keyCodes[0]<=KeyNum0 && pstPressed->keyCodes[0]>=KeyNum1) {
            nextChar = '0'+(pstPressed->keyCodes[0]-KeyNum1+1)%10;
        } else if(containsKeys(pstPressed,&keyCode,2,KeyDot,KeyNumDelete)) {
            nextChar = '.';
        } else if(containsKeys(pstPressed,&keyCode,2,KeyDelete,KeyDeleteForward)) {
            nextChar = '\b';
        }
        if(nextChar != '\0') {
            if(nextChar == '\b') {
                if(pstStateMachine->pcBufferCursor>pstStateMachine->pcBuffer) {
                    pstStateMachine->pcBufferCursor--;
                }
            } else {
                if(pstStateMachine->pcBufferCursor - pstStateMachine->pcBuffer < INPUT_BUFFER_SIZE) {
                    if(nextChar == '.') {
                        if(strchr(pstStateMachine->pcBuffer,'.') == NULL) {
                            if(pstStateMachine->pcBufferCursor == pstStateMachine->pcBuffer) {
                                *pstStateMachine->pcBufferCursor++ = '0';
                            }
                            *pstStateMachine->pcBufferCursor++ = '.';
                        }
                    } else if(nextChar == '0') {
                        if(pstStateMachine->pcBuffer!=pstStateMachine->pcBufferCursor) {
                            *pstStateMachine->pcBufferCursor++ = '0';
                        }
                    } else {
                        *pstStateMachine->pcBufferCursor++ = nextChar;
                    }
                }
            }
            *pstStateMachine->pcBufferCursor='\0';
            *piActionID = RefreshScreen;
        }
    }

}
static void errorStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {

}

static void resetCalculator() {
    pstStateMachine->eState       			= InitialState;
    pstStateMachine->ePrevAction			= NoOperate;
    pstStateMachine->stPrevNumber.iValue  	= 0;
    pstStateMachine->stPrevNumber.uiShift 	= 0;
    *(pstStateMachine->pcBufferCursor 		= pstStateMachine->pcBuffer)='\0';
}

static void strToNumber(char* pBuffer,AccurateFloatNumber_t* pstNumber) {
    char* pcFraction;
    pcFraction = strchr(pBuffer,'.');

    if(pcFraction!=NULL) {
        *pcFraction++ = '\0';
    }

    pstNumber->iValue	= atol(pBuffer);
    pstNumber->uiShift 	= 0;
    if(pcFraction != NULL) {
        pstNumber->uiShift = strlen(pcFraction);
        for(uint8_t i=0; i<pstNumber->uiShift; i++) {
            pstNumber->iValue *= 10;
        }
        pstNumber->iValue += atol(pcFraction) * (pstNumber->iValue<0?-1:1);
    }
    simplifyNumber(pstNumber);

}
static void numberToStr(char* pBuffer,AccurateFloatNumber_t* pstNumber) {
    char* pChar;
    uint8_t uiStrLen;
    uint8_t uiReverseStart;
    uint8_t uiPrefixLen;
    uiStrLen=sprintf(pBuffer,"%ld",pstNumber->iValue);
    pChar = pBuffer+uiStrLen;
    uiPrefixLen = (*pBuffer=='-')?1:0;

    uiReverseStart = uiStrLen-SGUI_MIN_OF(pstNumber->uiShift,uiStrLen-uiPrefixLen);
    // 反向放在小数点后边的部分
    reverseStr(pBuffer,uiReverseStart,uiStrLen-1);
    // 补小数点前缺少的0
    if(pstNumber->uiShift > uiStrLen-uiPrefixLen) {
        memset(pBuffer+uiStrLen,'0',pstNumber->uiShift-uiStrLen+uiPrefixLen);
        pBuffer[pstNumber->uiShift+uiPrefixLen]='\0';
        pChar = pBuffer+pstNumber->uiShift+uiPrefixLen;
    }
    // 补小数点
    *pChar++ = '.';
    *pChar   = '\0';
    // 补零
    if(pstNumber->uiShift >= uiStrLen-uiPrefixLen) {
        *pChar++ = '0';
        *pChar = '\0';
    }
    reverseStr(pBuffer,uiReverseStart,pChar-pBuffer-1);

}
static void reverseStr(char* pBuffer,uint8_t from,uint8_t to) {
    for(int i=from; i<=(from+to)/2; i++) {
        if(pBuffer[i]!=pBuffer[from+to-i]) {
            pBuffer[i]          = pBuffer[i] ^ pBuffer[from+to-i];
            pBuffer[from+to-i] = pBuffer[i] ^ pBuffer[from+to-i];
            pBuffer[i]          = pBuffer[i] ^ pBuffer[from+to-i];
        }
    }
}
static void simplifyNumber(AccurateFloatNumber_t* pstNumber) {
    while(pstNumber->iValue%10 == 0 && pstNumber->uiShift>0) {
        pstNumber->iValue /=10;
        pstNumber->uiShift --;
    }
}
static void alineNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB) {
    // 对齐
    if(pstNumberA->uiShift<pstNumberB->uiShift) {
        // 数字B 小数更多	==> A对齐到B
        //pstNumberA->iValue *= pow(10,pstNumberB->uiShift-pstNumberA->uiShift);
        for(uint8_t i=0; i<pstNumberB->uiShift-pstNumberA->uiShift; i++) {
            pstNumberA->iValue *= 10;
        }
        pstNumberA->uiShift = pstNumberB->uiShift;
    } else if(pstNumberA->uiShift>pstNumberB->uiShift) {
        // 数字A 小数跟多   ==> B对齐到A
        //pstNumberB->iValue *= pow(10,pstNumberA->uiShift-pstNumberB->uiShift);
        for(uint8_t i=0; i<pstNumberA->uiShift-pstNumberB->uiShift; i++) {
            pstNumberB->iValue *= 10;
        }
        pstNumberB->uiShift = pstNumberA->uiShift;
    }
}

static void plusCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage) {
    alineNumber(pstNumberA,pstNumberB);
    pstNumberA->iValue += pstNumberB->iValue;
    simplifyNumber(pstNumberA);
    simplifyNumber(pstNumberB);
}
static void minusCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage) {
    alineNumber(pstNumberA,pstNumberB);
    pstNumberA -> iValue -= pstNumberB->iValue;
    simplifyNumber(pstNumberA);
    simplifyNumber(pstNumberB);
}
static void multiplyCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage) {
    pstNumberA -> iValue  *= pstNumberB->iValue;
    pstNumberA -> uiShift += pstNumberB->uiShift;
    simplifyNumber(pstNumberA);
}
static void divideCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage) {
    double dbNumberA = pstNumberA->iValue;
    double dbNumberB = pstNumberB->iValue;
    uint8_t isNegative=0;
    uint8_t uiIntegerLength = 0;
    // 转换为double
    for(uint8_t i=0; i<pstNumberA->uiShift; i++) {
        dbNumberA /= 10.0;
    }
    for(uint8_t i=0; i<pstNumberB->uiShift; i++) {
        dbNumberB /= 10.0;
    }
    if(dbNumberB>=(-1e-6)&&dbNumberB<=(1e-6)) {
        *ppcErrorMessage = "你他喵的是不是除了个0?";
        return;
    }
    // 运算
    dbNumberA /= dbNumberB;
    // 转换回Number
    isNegative = dbNumberA <0 ? 1: 0;
    pstNumberA->uiShift =0;
    pstNumberA->iValue = (int64_t)dbNumberA;
    while(pstNumberA->iValue>0) {
        pstNumberA->iValue /= 10;
        uiIntegerLength++;
    }
    pstNumberA->iValue = (int64_t)dbNumberA;
    dbNumberA -= pstNumberA->iValue;
    while(uiIntegerLength<9) {
        dbNumberA *= 10;
        pstNumberA->iValue = pstNumberA->iValue*10 + ((uint8_t)dbNumberA) * (isNegative?-1:1);
        dbNumberA -= (uint8_t)dbNumberA;
        pstNumberA->uiShift ++;
        uiIntegerLength ++;
    }
    simplifyNumber(pstNumberA);
}
static void equalsCalcAction(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage) {
    pstNumberA->iValue = pstNumberB->iValue;
    pstNumberA->uiShift = pstNumberB->uiShift;
}
