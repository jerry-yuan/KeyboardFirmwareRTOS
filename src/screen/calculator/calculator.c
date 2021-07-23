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
    char  pcBuffer[INPUT_BUFFER_SIZE];
    char* pcBufferCursor;
    CalculateAction_t ePrevAction;
    AccurateFloatNumber_t stPrevNumber;
    CalculatorStates_t eState;
    const char* pcLastError;
    void (*pfActionHandlers[4])(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID);
    void (*pfRenders[4])(SGUI_SCR_DEV* pstDeviceIF);
    void (*pfDoCalcAction[6])(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB,const char** ppcErrorMessage);
} ScreenContext_t;

enum {
    NoAction      = 0x00,
    GoBack        = 0x01,
    PushToHost    = 0x02,
    RefreshScreen = 0x80,
    ResetCalc	  = 0x81
};

static ScreenContext_t* pstContext = NULL;

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


    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_Basic_ClearScreen(pstDeviceIF);

	pstContext = pvPortMalloc(sizeof(ScreenContext_t));
    memset(pstContext,0,sizeof(ScreenContext_t));

    pstContext->pcBufferCursor = pstContext->pcBuffer;

    pstContext->pfRenders[InitialState] = initialStateRender;
    pstContext->pfRenders[ResultState]  = resultStateRender;
    pstContext->pfRenders[InputState]   = inputStateRender;
    pstContext->pfRenders[ErrorState]   = errorStateRender;

    pstContext->pfActionHandlers[InitialState] = initialStateHandler;
    pstContext->pfActionHandlers[ResultState]  = resultStateHandler;
    pstContext->pfActionHandlers[InputState]   = inputStateHandler;
    pstContext->pfActionHandlers[ErrorState]   = errorStateHandler;

	pstContext->pfDoCalcAction[NoOperate] = equalsCalcAction;
    pstContext->pfDoCalcAction[Divide] = divideCalcAction;
    pstContext->pfDoCalcAction[Multiply] = multiplyCalcAction;
    pstContext->pfDoCalcAction[Minus] = minusCalcAction;
    pstContext->pfDoCalcAction[Plus] = plusCalcAction;
    pstContext->pfDoCalcAction[Equals] = equalsCalcAction;

    resetCalculator();

    Refresh(pstDeviceIF,NULL);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_Basic_DrawRectangle(pstDeviceIF,5,5,256-10,64-10,0x0A,0x00);
    pstContext->pfRenders[pstContext->eState](pstDeviceIF);
    return HMI_RET_NORMAL;
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
        } else if(containsKey(&stRelease,KeyInsert)){
            *piActionID = PushToHost;
        } else {
            pstContext -> pfActionHandlers[pstContext->eState](&stPressed,&stRelease,piActionID);
        }

        vPortFree(stPressed.keyCodes);
        vPortFree(stRelease.keyCodes);

    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID==GoBack) {
		vPortFree(pstContext);
		pstContext=NULL;
        HMI_GoBack(NULL);
    } else if(iActionID == ResetCalc) {
        resetCalculator();
    } else if(iActionID == PushToHost) {
        uint8_t i;
        uint8_t pcBuffer[DISPLAY_BUFFER_SIZE] = {0};
        numberToStr((char*)pcBuffer,&pstContext->stPrevNumber);
        for(i=0;i<DISPLAY_BUFFER_SIZE && pcBuffer[i]!='\0';i++){
            if(pcBuffer[i]>='0' && pcBuffer[i]<='9'){
                pcBuffer[i] = (pcBuffer[i]-'0'+9)%10 + Key1;
            }else{
                pcBuffer[i] = KeyDot;
            }
        }
        sendKeysToHost(pcBuffer,i);
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

    numberToStr(pcBuffer,&pstContext->stPrevNumber);

    // draw number
    SGUI_Text_GetTextExtent(pcBuffer,SGUI_FONT_REF(LCD44),&stArea);
    stRect.iX = 236-stArea.iWidth;
    stRect.iY = 10;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Text_DrawText(pstDeviceIF,pcBuffer,SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
    // draw Operator

    SGUI_Text_GetTextExtent(operators[pstContext->ePrevAction],SGUI_FONT_REF(LCD44),&stArea);
    stRect.iX = 10;
    stRect.iY = 10;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Text_DrawText(pstDeviceIF,operators[pstContext->ePrevAction],SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
}
static void inputStateRender(SGUI_SCR_DEV* pstDeviceIF) {
    SGUI_RECT stRect;
    SGUI_POINT stPoint;
    SGUI_AREA_SIZE stArea;
    char cBuffer[DISPLAY_BUFFER_SIZE]= "0.";
    if(pstContext->pcBuffer != pstContext->pcBufferCursor) {
        sprintf(cBuffer,"%s%s",pstContext->pcBuffer,strstr(pstContext->pcBuffer,".")?"":".");
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
    stNoticeBox.cszNoticeText = pstContext->pcLastError;
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
        pstContext -> eState = InputState;
        inputStateHandler(pstPressed,pstRelease,piActionID);
    } else if(pstPressed->keyCodes[0]<=KeyNumEnter && pstPressed->keyCodes[0]>=KeyNumSlash) {
        pstContext -> eState = ResultState;
        pstContext -> ePrevAction = pstPressed->keyCodes[0] - KeyNumSlash+1;
        *piActionID = RefreshScreen;
    }
}
static void resultStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {
    KeyboardUsageCode_t keyCode;
    if(pstPressed->keyCodes[0]<=KeyNumEnter && pstPressed->keyCodes[0]>=KeyNumSlash) {
        pstContext->ePrevAction = pstPressed->keyCodes[0] - KeyNumSlash+1;
        *piActionID = RefreshScreen;
    } else if((pstPressed->keyCodes[0]<=Key0 && pstPressed->keyCodes[0]>=Key1) || (pstPressed->keyCodes[0]<=KeyNum0 && pstPressed->keyCodes[0]>=KeyNum1) || containsKeys(pstPressed,&keyCode,2,KeyDot,KeyNumDelete)) {
        pstContext->eState = InputState;
        inputStateHandler(pstPressed,pstRelease,piActionID);
    }
}
static void inputStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {
    char nextChar = '\0';
    KeyboardUsageCode_t keyCode;
    const char* pcErrorMessage=NULL;
    AccurateFloatNumber_t stNumber;
    if(pstPressed->keyCodes[0]<=KeyNumEnter && pstPressed->keyCodes[0]>=KeyNumSlash) {
        strToNumber(pstContext->pcBuffer,&stNumber);
        *(pstContext->pcBufferCursor = pstContext->pcBuffer) = '\0';

        pstContext->pfDoCalcAction[pstContext->ePrevAction](&pstContext->stPrevNumber,&stNumber,&pcErrorMessage);
        if(pcErrorMessage==NULL) {
            pstContext->eState      = ResultState;
            pstContext->ePrevAction = pstPressed->keyCodes[0]-KeyNumSlash+1;
        } else {
            pstContext->eState      = ErrorState;
            pstContext->pcLastError = pcErrorMessage;
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
                if(pstContext->pcBufferCursor>pstContext->pcBuffer) {
                    pstContext->pcBufferCursor--;
                }
            } else {
                if(pstContext->pcBufferCursor - pstContext->pcBuffer < INPUT_BUFFER_SIZE) {
                    if(nextChar == '.') {
                        if(strchr(pstContext->pcBuffer,'.') == NULL) {
                            if(pstContext->pcBufferCursor == pstContext->pcBuffer) {
                                *pstContext->pcBufferCursor++ = '0';
                            }
                            *pstContext->pcBufferCursor++ = '.';
                        }
                    } else if(nextChar == '0') {
                        if(pstContext->pcBuffer!=pstContext->pcBufferCursor) {
                            *pstContext->pcBufferCursor++ = '0';
                        }
                    } else {
                        *pstContext->pcBufferCursor++ = nextChar;
                    }
                }
            }
            *pstContext->pcBufferCursor='\0';
            *piActionID = RefreshScreen;
        }
    }

}
static void errorStateHandler(MappedKeyCodes_t* pstPressed,MappedKeyCodes_t* pstRelease,SGUI_INT* piActionID) {

}

static void resetCalculator() {
    pstContext->eState       			= InitialState;
    pstContext->ePrevAction				= NoOperate;
    pstContext->stPrevNumber.iValue  	= 0;
    pstContext->stPrevNumber.uiShift 	= 0;
    *(pstContext->pcBufferCursor 		= pstContext->pcBuffer)='\0';
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
    uint64_t uiTempValue;

	uiPrefixLen = 0;
    // 转换64位数字实部为字符串
    uiTempValue = (pstNumber->iValue<0)?(-pstNumber->iValue):pstNumber->iValue;
    uiStrLen = 0;
    while(uiTempValue>0){
		pBuffer[uiStrLen]='0'+uiTempValue%10;
		uiStrLen++;
		uiTempValue/=10;
    }
    if(pstNumber->iValue<0){
		pBuffer[uiStrLen] = '-';
		uiStrLen ++;
		uiPrefixLen = 1;
    }
	reverseStr(pBuffer,0,uiStrLen-1);
    // 找到空闲区域的起始位置
    pChar = pBuffer+uiStrLen;
    // 反向小数部分
    uiReverseStart = uiStrLen-SGUI_MIN_OF(pstNumber->uiShift,uiStrLen-uiPrefixLen);
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
