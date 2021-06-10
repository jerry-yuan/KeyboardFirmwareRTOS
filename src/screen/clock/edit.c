#include <screen/clock/edit.h>
#include <lib/keyboard.h>

#include <resources/Font.h>
#include <resources/fontawesome.h>
#include <bsp/tim.h>
#include <SGUI_Basic.h>
#include <SGUI_Text.h>
#include <SGUI_FontResource.h>
#include <SGUI_IconResource.h>
#include <SGUI_Notice.h>
#include <SGUI_VariableBox.h>

#include <time.h>

#define MASK_REFRESH_SCREEN 0x80
#define MASK_GO_BACK        0x40
#define MASK_START_REPEATER 0x20

enum {
    NoAction        = 0x00,
    TurnUp          = 0xA1,
    TurnDown        = 0xA2,
    SwitchFiledR    = 0xA3,
    SwitchFiledL    = 0xA4,
    SetTime         = 0x41,
    GoBack          = 0x40
};

typedef struct{
	SGUI_NUM_VARBOX_STRUCT   pstBoxArr[6];
	SGUI_NUM_VARBOX_STRUCT*  pstYearBox;
	SGUI_NUM_VARBOX_STRUCT*  pstMonthBox;
	SGUI_NUM_VARBOX_STRUCT*  pstDayBox;
	SGUI_NUM_VARBOX_STRUCT*  pstHourBox;
	SGUI_NUM_VARBOX_STRUCT*  pstMinBox;
	SGUI_NUM_VARBOX_STRUCT*  pstSecBox;
	uint8_t                  uiBoxFocus;
} ScreenContext_t;

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

static HMI_SCREEN_ACTION screenActions= {
    Initialize,
    Prepare,
    Refresh,
    ProcessEvent,
    PostProcess
};

HMI_SCREEN_OBJECT SCREEN_Clock_Edit= {SCREEN_Clock_Edit_ID,&screenActions};

static const uint8_t daysOfMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

static ScreenContext_t* pstContext=NULL;

HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_NOTICE_BOX			stNoticeBox;
    SGUI_RECT				stRect;
    SGUI_POINT				stPoint;
    SGUI_NUM_VARBOX_PARAM	stNumBoxInitParam;
	time_t time=RTC_GetCounter();
    struct tm* pstTime=gmtime(&time);
	// 为上下文申请空间
	pstContext  = pvPortMalloc(sizeof(ScreenContext_t));
	memset(pstContext,0,sizeof(ScreenContext_t));
	// 初始化指针
    pstContext->pstYearBox  = pstContext->pstBoxArr+0;
    pstContext->pstMonthBox = pstContext->pstBoxArr+1;
    pstContext->pstDayBox   = pstContext->pstBoxArr+2;
    pstContext->pstHourBox  = pstContext->pstBoxArr+3;
    pstContext->pstMinBox   = pstContext->pstBoxArr+4;
    pstContext->pstSecBox   = pstContext->pstBoxArr+5;
	// 初始化输入框公共参数
    stNumBoxInitParam.eAlignment        = SGUI_CENTER;
    stNumBoxInitParam.pstFontRes        = SGUI_FONT_REF(Deng12);
    stNumBoxInitParam.stLayout.iHeight  = 12;
    stNumBoxInitParam.stLayout.iY       = 26;

    stNumBoxInitParam.stPalette.uiDepthBits = 4;
    stNumBoxInitParam.stPalette.stFocus.eBackgroundColor = 0x05;
    stNumBoxInitParam.stPalette.stFocus.eTextColor = 0x0F;
    stNumBoxInitParam.stPalette.stNormal.eBackgroundColor= 0x00;
    stNumBoxInitParam.stPalette.stNormal.eTextColor=0x0F;

    // 初始化年份输入框
    stNumBoxInitParam.iMin              = 1970;
    stNumBoxInitParam.iMax              = 2038;
    stNumBoxInitParam.stLayout.iWidth   = 28;
    stNumBoxInitParam.stLayout.iX       = 37;

    SGUI_NumberVariableBox_Initialize(pstContext->pstYearBox,&stNumBoxInitParam);

    //初始化月份输入框
    stNumBoxInitParam.iMin              = 1;
    stNumBoxInitParam.iMax              = 12;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 81;

    SGUI_NumberVariableBox_Initialize(pstContext->pstMonthBox,&stNumBoxInitParam);

    //初始化日期输入框
    stNumBoxInitParam.iMin              = 1;
    stNumBoxInitParam.iMax              = 31;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 113;

    SGUI_NumberVariableBox_Initialize(pstContext->pstDayBox,&stNumBoxInitParam);

    //初始化小时输入框
    stNumBoxInitParam.iMin              = 0;
    stNumBoxInitParam.iMax              = 23;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 151;

    SGUI_NumberVariableBox_Initialize(pstContext->pstHourBox,&stNumBoxInitParam);

    //初始化分钟输入框
    stNumBoxInitParam.iMin              = 0;
    stNumBoxInitParam.iMax              = 59;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 176;

    SGUI_NumberVariableBox_Initialize(pstContext->pstMinBox,&stNumBoxInitParam);

    //初始化秒数输入框
    stNumBoxInitParam.iMin              = 0;
    stNumBoxInitParam.iMax              = 59;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 201;

    SGUI_NumberVariableBox_Initialize(pstContext->pstSecBox,&stNumBoxInitParam);

	// 初始化默认选中的项目
    pstContext->uiBoxFocus = 0;
    SGUI_Basic_ClearScreen(pstDeviceIF);

    SGUI_Notice_FitArea(pstDeviceIF,&stNoticeBox.stLayout);
	//绘制对话框和标题
    stNoticeBox.cszNoticeText           = FA_CLOCK " 设置时间:";
    stNoticeBox.pstIcon                 = NULL;
    stNoticeBox.stPalette.eEdgeColor    = 0x0A;
    stNoticeBox.stPalette.eFillColor    = 0x00;
    stNoticeBox.stPalette.eTextColor	= 0x0A;
    stNoticeBox.stPalette.uiDepthBits   = 4;

    SGUI_Notice_Repaint(pstDeviceIF,&stNoticeBox,SGUI_FONT_REF(Deng12),0);

	//设置当前时间
    pstContext->pstYearBox->stData.iValue   = pstTime->tm_year + 1900;
    pstContext->pstMonthBox->stData.iValue  = pstTime->tm_mon+1;
    pstContext->pstDayBox->stData.iValue    = pstTime->tm_mday;
    pstContext->pstHourBox->stData.iValue   = pstTime->tm_hour;
    pstContext->pstMinBox->stData.iValue    = pstTime->tm_min;
    pstContext->pstSecBox->stData.iValue    = pstTime->tm_sec;
	// 给输入框加一个框线
    SGUI_Basic_DrawRectangle(pstDeviceIF,35,24,32,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,79,24,20,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,111,24,20,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,149,24,20,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,174,24,20,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,199,24,20,16,0x0A,0x00);
	// 绘制年月日提示
    stPoint.iX = stPoint.iY =0;

    stRect.iWidth   = 12;
    stRect.iHeight  = 12;
    stRect.iX       = 67;
    stRect.iY       = 26;
    SGUI_Text_DrawText(pstDeviceIF,"年",SGUI_FONT_REF(Deng12),&stRect,&stPoint,SGUI_DRAW_NORMAL);
    stRect.iX       = 99;
    SGUI_Text_DrawText(pstDeviceIF,"月",SGUI_FONT_REF(Deng12),&stRect,&stPoint,SGUI_DRAW_NORMAL);
    stRect.iX       = 131;
    SGUI_Text_DrawText(pstDeviceIF,"日",SGUI_FONT_REF(Deng12),&stRect,&stPoint,SGUI_DRAW_NORMAL);
    stRect.iX       = 170;
    stRect.iWidth   = 3;
    SGUI_Text_DrawText(pstDeviceIF,":",SGUI_FONT_REF(Deng12),&stRect,&stPoint,SGUI_DRAW_NORMAL);
    stRect.iX       = 195;
    SGUI_Text_DrawText(pstDeviceIF,":",SGUI_FONT_REF(Deng12),&stRect,&stPoint,SGUI_DRAW_NORMAL);
    Refresh(pstDeviceIF,pstParameters);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    // 判断月份日期越界问题
    uint16_t uiYear=pstContext->pstYearBox->stData.iValue;
    uint8_t uiMonth=pstContext->pstMonthBox->stData.iValue;
    uint8_t uiDaysOffset;
    uint8_t uiBoxIndex;
    if((uiYear%4 == 0 && uiYear%100 !=0 ) || uiYear%400 == 0) {
        uiDaysOffset = 1;
    } else {
        uiDaysOffset = 0;
    }
    pstContext->pstDayBox->stParam.iMax  = daysOfMonth[uiMonth-1]+(uiMonth==2?uiDaysOffset:0);
    pstContext->pstDayBox->stData.iValue = SGUI_MIN_OF(pstContext->pstDayBox->stData.iValue,pstContext->pstDayBox->stParam.iMax);

	for(uiBoxIndex=0;uiBoxIndex<6;uiBoxIndex++){
		pstContext->pstBoxArr[uiBoxIndex].stData.iFocused = (pstContext->uiBoxFocus == uiBoxIndex)?SGUI_TRUE:SGUI_FALSE;
		SGUI_NumberVariableBox_Repaint(pstDeviceIF,pstContext->pstBoxArr+uiBoxIndex);
	}
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    KEY_EVENT* pstKeyEvent;
    MappedKeyCodes_t stPressed,stRelease;

    *piActionID = NoAction;
    if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        pstKeyEvent = (KEY_EVENT*)pstEvent;
        iLastAction = NoAction;

        TIM_KeyRepeater_Reset();

        stPressed.cursor    = 0;
        stPressed.length	= pstKeyEvent->Data.uiPressedCount;
        stPressed.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stPressed.length);
        mapKeyCodes(pstKeyEvent->Data.pstPressed,stPressed.keyCodes);

        stRelease.cursor    = 0;
        stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*stRelease.length);
        mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);

        if(containsKey(&stPressed,KeyUp)) {
            iLastAction=*piActionID = TurnUp;
        } else if(containsKey(&stPressed,KeyDown)) {
            iLastAction=*piActionID = TurnDown;
        } else if(containsKey(&stPressed,KeyTab)||containsKey(&stPressed,KeyRight)) {
            iLastAction=*piActionID = SwitchFiledR;
        } else if(containsKey(&stPressed,KeyLeft)) {
            iLastAction=*piActionID = SwitchFiledL;
        } else if(containsKey(&stRelease,KeyEnter)) {
            *piActionID = SetTime;
        } else if(containsKey(&stRelease,KeyEscape)) {
            *piActionID = GoBack;
        }

        vPortFree(stPressed.keyCodes);
        vPortFree(stRelease.keyCodes);

        if(iLastAction & MASK_START_REPEATER) {
            TIM_KeyRepeater_Set();
        }
    } else if(pstEvent->iID == KEY_REPEAT_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_REPEAT_EVENT)) {
        if(iLastAction!=NoAction) {
            *piActionID = iLastAction;
        }
    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    struct tm* pstTime;
    if(iActionID == SwitchFiledR) {
        pstContext->uiBoxFocus = (pstContext->uiBoxFocus+1)%6;
    } else if(iActionID == SwitchFiledL) {
        pstContext->uiBoxFocus = (pstContext->uiBoxFocus+5)%6;
    } else if(iActionID == TurnUp) {
        SGUI_NumberVariableBox_Increase(pstContext->pstBoxArr+pstContext->uiBoxFocus);
    } else if(iActionID == TurnDown) {
        SGUI_NumberVariableBox_Decrease(pstContext->pstBoxArr+pstContext->uiBoxFocus);
    } else if(iActionID == SetTime) {
        pstTime = pvPortMalloc(sizeof(struct tm));
        pstTime->tm_year    = pstContext->pstYearBox->stData.iValue - 1900;
        pstTime->tm_mon     = pstContext->pstMonthBox->stData.iValue - 1;
        pstTime->tm_mday    = pstContext->pstDayBox->stData.iValue;
        pstTime->tm_hour    = pstContext->pstHourBox->stData.iValue;
        pstTime->tm_min     = pstContext->pstMinBox->stData.iValue;
        pstTime->tm_sec     = pstContext->pstSecBox->stData.iValue;
        RTC_SetCounter(mktime(pstTime));
        RTC_WaitForLastTask();
        vPortFree(pstTime);
    }
    if(iActionID & MASK_GO_BACK) {
		vPortFree(pstContext);
        HMI_GoBack(NULL);
    }
    if(iActionID & MASK_REFRESH_SCREEN) {
        Refresh(pstDeviceIF,NULL);
    }
    return HMI_RET_NORMAL;
}
