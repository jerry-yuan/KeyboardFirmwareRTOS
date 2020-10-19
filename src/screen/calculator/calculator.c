#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <SGUI_Basic.h>
#include <screen/calculator/calculator.h>

#include <lib/keyboard.h>

#include <resources/Font.h>

#define INPUT_BUFFER_SIZE			10
#define DISPLAY_BUFFER_SIZE			20

#define ACTION_MASK_REFRESH 	0x80
#define ACTION_MASK_INPUT		0x40
#define ACTION_MASK_CALCULATE 	0x20

#define FLAG_MASK_HAS_DOT		0x01
#define FLAG_MASK_HAS_RESULT 	0x02
#define FLAG_MASK_HAS_ERROR		0x04
#define FLAG_MASK_PREV_CALC		0xF0

#define FLAG_CALC_EQUALS		0x00
#define FLAG_CALC_DIVIDE		0x10
#define FLAG_CALC_MULTIPLY		0x20
#define FLAG_CALC_MINUS			0x30
#define FLAG_CALC_PLUS			0x40


enum {
    NoAction      = 0x00,
    GoBack        = 0x01,
    RefreshScreen = 0x80,

    DoBackSpace   = 0x81,
    DoClear		  = 0x82,

    DoDivide	  = 0xA0,
    DoMultipy     = 0xA1,
    DoMinus		  = 0xA2,
    DoPlus	      = 0xA3,
    DoEquals      = 0xA4,

    Input0		  = 0xC0,
    Input1		  = 0xC1,
    Input2		  = 0xC2,
    Input3		  = 0xC3,
    Input4		  = 0xC4,
    Input5		  = 0xC5,
    Input6		  = 0xC6,
    Input7		  = 0xC7,
    Input8		  = 0xC8,
    Input9		  = 0xC9,
    InputDot	  = 0xCA
};

static char* pcBuffer 					= NULL;
static char* pcBufferCursor 			= NULL;
static uint8_t uiFlags					= 0x00;
static AccurateFloatNumber_t* pstResult = NULL;

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

static void strToNumber(char* pBuffer,AccurateFloatNumber_t* pstNumber);
static void numberToStr(char* pBuffer,AccurateFloatNumber_t* pstNumber);
static void reverseStr(char* pBuffer,uint8_t from,uint8_t to);
static void alineNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB);
static void simplifyNumber(AccurateFloatNumber_t* pstNumber);
static void addNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB);
static void minusNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB);
static void multiplyNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB);
static void divideNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB);

static HMI_SCREEN_ACTION screenActions= {
    Initialize,
    Prepare,
    Refresh,
    ProcessEvent,
    PostProcess
};

HMI_SCREEN_OBJECT SCREEN_Calculator= {SCREEN_Calculator_ID,&screenActions};

HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    pstResult = pvPortMalloc(sizeof(AccurateFloatNumber_t));
    pcBuffer = pvPortMalloc(sizeof(char)*INPUT_BUFFER_SIZE);
    pcBufferCursor = pcBuffer;
    memset(pcBuffer,0,sizeof(char)*INPUT_BUFFER_SIZE);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_Basic_ClearScreen(pstDeviceIF);
    Refresh(pstDeviceIF,NULL);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    char cToDisplay[DISPLAY_BUFFER_SIZE]= {0};

    const char* cOperators[5]= {"=","÷","×","-","+"};
    const char* cOperator = "";
    SGUI_RECT stRect;
    SGUI_POINT stPoint;
    SGUI_AREA_SIZE stArea;

    SGUI_Basic_DrawRectangle(pstDeviceIF,5,5,256-10,64-10,0x0A,0x00);

    stPoint.iX=stPoint.iY=0;
	if(uiFlags & FLAG_MASK_HAS_ERROR){
		stRect.iX = 10;
		stRect.iY = 10;
		stRect.iHeight = 44;
		stRect.iWidth  = 246;
		SGUI_Text_DrawMultipleLinesText(pstDeviceIF,"产生错误:\n请检查是不是他喵的除了个0?",SGUI_FONT_REF(Deng12),&stRect,0,0x0F);
		return HMI_RET_NORMAL;
	} else if(pcBuffer!=pcBufferCursor) {
        sprintf(cToDisplay,"%s%c",pcBuffer,uiFlags&FLAG_MASK_HAS_DOT?'\0':'.');
    } else if(uiFlags & FLAG_MASK_HAS_RESULT) {
        numberToStr(cToDisplay,pstResult);
        cOperator=cOperators[((uiFlags & FLAG_MASK_PREV_CALC)>>4) & 0x0f];
    } else {
        sprintf(cToDisplay,"0.");
    }
    SGUI_Text_GetTextExtent(cToDisplay,SGUI_FONT_REF(LCD44),&stArea);
    stRect.iX = 236 - stArea.iWidth;
    stRect.iY = 10;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Text_DrawText(pstDeviceIF,cToDisplay,SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);
    SGUI_Text_GetTextExtent(cOperator,SGUI_FONT_REF(LCD44),&stArea);
    stRect.iX = stRect.iY = 10;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Text_DrawText(pstDeviceIF,cOperator,SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0F);

    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    KEY_EVENT* pstKeyEvent;
    MappedKeyCodes_t stPressed,stRelease;
    uint8_t keyCode;
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

        if(stPressed.keyCodes[0]<=Key0 && stPressed.keyCodes[0]>=Key1) {
            *piActionID = ACTION_MASK_INPUT | ACTION_MASK_REFRESH | (((stPressed.keyCodes[0]-Key1 + 1)%10)&0x0F);
        } else if(stPressed.keyCodes[0]<=KeyNum0 && stPressed.keyCodes[0]>=KeyNum1) {
            *piActionID = ACTION_MASK_INPUT | ACTION_MASK_REFRESH | (((stPressed.keyCodes[0]-KeyNum1 + 1)%10)&0x0F);
        } else if(containsKeys(&stPressed,&keyCode,2,KeyDot,KeyNumDelete)) {
            *piActionID = InputDot;
        } else if(containsKeys(&stPressed,&keyCode,2,KeyDelete,KeyDeleteForward)) {
            *piActionID = DoBackSpace;
        } else if(stPressed.keyCodes[0]<=KeyNumEnter && stPressed.keyCodes[0]>=KeyNumSlash) {
            *piActionID = ACTION_MASK_CALCULATE | ACTION_MASK_REFRESH | ((stPressed.keyCodes[0]-KeyNumSlash)&0x0F);
        } else if(containsKey(&stPressed,KeyEnter)) {
            *piActionID = DoEquals;
        } else if(containsKey(&stPressed,KeyNumLock)) {
            *piActionID = DoClear;
        } else if(containsKey(&stRelease,KeyEscape)) {
            *piActionID = GoBack;
        }

        vPortFree(stPressed.keyCodes);
        vPortFree(stRelease.keyCodes);

    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    AccurateFloatNumber_t stNumber= {0,0};
    if(iActionID & ACTION_MASK_INPUT) {
        if(pcBufferCursor-pcBuffer < INPUT_BUFFER_SIZE - ((uiFlags & FLAG_MASK_HAS_DOT)?0:1)) {
            if(iActionID == InputDot) {
                if(!(uiFlags & FLAG_MASK_HAS_DOT)) {
                    uiFlags |= FLAG_MASK_HAS_DOT;
                    if(pcBufferCursor == pcBuffer){
						*pcBufferCursor++ = '0';
                    }
                    *pcBufferCursor++ = '.';
                }
            } else if(iActionID == Input0) {
				if(pcBuffer!=pcBufferCursor) {
                    *pcBufferCursor++ = '0';
                }
            }else{
                *pcBufferCursor++ = '0'+(iActionID & 0x0F);
            }
            *pcBufferCursor = '\0';
        }
    } else if(iActionID & ACTION_MASK_CALCULATE) {
        if(pcBufferCursor>pcBuffer) {
			// 输入缓冲有数据  ==> 参与计算
            strToNumber(pcBuffer,&stNumber);
            *(pcBufferCursor = pcBuffer) = '\0';
            uiFlags   &= ~FLAG_MASK_HAS_DOT;

            if(!(uiFlags & FLAG_MASK_HAS_RESULT) || (uiFlags & FLAG_MASK_PREV_CALC) == FLAG_CALC_EQUALS) {
                // 没有临时数据  ==> 将数据放到临时区域
                *pstResult = stNumber;
                uiFlags   |= FLAG_MASK_HAS_RESULT;
            } else {
                // 有临时数据    ==> 执行上一个计算并放到临时区域
                uiFlags   |= FLAG_MASK_HAS_RESULT;
                if((uiFlags & FLAG_MASK_PREV_CALC) == FLAG_CALC_PLUS) {
                    // 加法
                    addNumber(pstResult,&stNumber);
                    //dbTempResult += dbNumber;
                } else if((uiFlags & FLAG_MASK_PREV_CALC) == FLAG_CALC_MINUS) {
                    // 减法
                    minusNumber(pstResult,&stNumber);
                    //dbTempResult -= dbNumber;
                } else if((uiFlags & FLAG_MASK_PREV_CALC) == FLAG_CALC_MULTIPLY) {
                    // 乘法
                    multiplyNumber(pstResult,&stNumber);
                    //dbTempResult *= dbNumber;
                } else if((uiFlags & FLAG_MASK_PREV_CALC) == FLAG_CALC_DIVIDE) {
                    // 除法
                    if(stNumber.iValue != 0) {
                        //dbTempResult /= dbNumber;
                        divideNumber(pstResult,&stNumber);
                    } else {
                        uiFlags |= FLAG_MASK_HAS_ERROR;
                    }
                }
            }
        }

        // 计算结束
        if(uiFlags & FLAG_MASK_HAS_ERROR) {
            // 计算异常
            uiFlags &= ~FLAG_MASK_HAS_RESULT;
        } else {
            // 计算正常 ==> 保存下一次要进行的操作
            uiFlags = ((((iActionID & 0x0F) +1)%5) << 4) | (uiFlags & 0x0F);
            if(iActionID != DoEquals){
				uiFlags |= FLAG_MASK_HAS_RESULT;
            }
        }
    } else if(iActionID == DoBackSpace) {
        if(pcBufferCursor > pcBuffer) {
            pcBufferCursor --;
            if(*pcBufferCursor == '.') {
                uiFlags &= ~FLAG_MASK_HAS_DOT;
            }
            *pcBufferCursor = '\0';
        }
    } else if(iActionID == DoClear) {
        uiFlags = 0x00;
        pstResult->iValue = 0;
        pstResult->uiShift = 0;
        pcBufferCursor = pcBuffer;
        *pcBuffer = '\0';
    }
    if(iActionID & ACTION_MASK_REFRESH) {
        Refresh(pstDeviceIF,NULL);
    }
    if(iActionID==GoBack) {
        HMI_GoBack(NULL);
    }

    return HMI_RET_NORMAL;
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
static void addNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB) {
    alineNumber(pstNumberA,pstNumberB);
    pstNumberA->iValue += pstNumberB->iValue;
    simplifyNumber(pstNumberA);
    simplifyNumber(pstNumberB);
}
static void minusNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB) {
    alineNumber(pstNumberA,pstNumberB);
    pstNumberA -> iValue -= pstNumberB->iValue;
    simplifyNumber(pstNumberA);
    simplifyNumber(pstNumberB);
}
static void multiplyNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB) {
	pstNumberA -> iValue  *= pstNumberB->iValue;
	pstNumberA -> uiShift += pstNumberB->uiShift;
	simplifyNumber(pstNumberA);
}
static void divideNumber(AccurateFloatNumber_t* pstNumberA,AccurateFloatNumber_t* pstNumberB) {
	double dbNumberA = pstNumberA->iValue;
	double dbNumberB = pstNumberB->iValue;
	uint8_t isNegative=0;
	uint8_t uiIntegerLength = 0;
	// 转换为double
	for(uint8_t i=0;i<pstNumberA->uiShift;i++){
		dbNumberA /= 10.0;
	}
	for(uint8_t i=0;i<pstNumberB->uiShift;i++){
		dbNumberB /= 10.0;
	}
	// 运算
	dbNumberA /= dbNumberB;
	// 转换回Number
	isNegative = dbNumberA <0 ? 1: 0;
	pstNumberA->uiShift =0;
	pstNumberA->iValue = (int64_t)dbNumberA;
	while(pstNumberA->iValue>0){
		pstNumberA->iValue /= 10;
		uiIntegerLength++;
	}
	pstNumberA->iValue = (int64_t)dbNumberA;
	dbNumberA -= pstNumberA->iValue;
	while(uiIntegerLength<9){
		dbNumberA *= 10;
		pstNumberA->iValue = pstNumberA->iValue*10 + ((uint8_t)dbNumberA) * (isNegative?-1:1);
		dbNumberA -= (uint8_t)dbNumberA;
		pstNumberA->uiShift ++;
		uiIntegerLength ++;
	}
	simplifyNumber(pstNumberA);
}
