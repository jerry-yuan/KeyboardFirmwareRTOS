#include <screen/clock/edit.h>
#include <lib/keyboard.h>
#include <task/keyboard.h>

#include <resources/Font.h>

#include <SGUI_Basic.h>
#include <SGUI_Text.h>
#include <SGUI_FontResource.h>
#include <SGUI_IconResource.h>
#include <SGUI_Notice.h>
#include <SGUI_VariableBox.h>

#include <time.h>

#define MASK_REFRESH_SCREEN 0x80
#define MASK_GO_BACK        0x40

enum{
    NoAction        = 0x00,
    TurnUp          = 0x81,
    TurnDown        = 0x82,
    SwitchFiledR    = 0x83,
    SwitchFiledL    = 0x84,
    SetTime         = 0x41,
    GoBack          = 0x40
};

static HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF);
static HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters);
static HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID);
static HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID);

static HMI_SCREEN_ACTION screenActions={
    Initialize,
    Prepare,
    Refresh,
    ProcessEvent,
    PostProcess
};

HMI_SCREEN_OBJECT SCREEN_Clock_Edit={SCREEN_Clock_Edit_ID,&screenActions};

const uint8_t daysOfMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

static SGUI_NUM_VARBOX_STRUCT*  pstYearBox=NULL;
static SGUI_NUM_VARBOX_STRUCT*  pstMonthBox=NULL;
static SGUI_NUM_VARBOX_STRUCT*  pstDayBox=NULL;
static SGUI_NUM_VARBOX_STRUCT*  pstHourBox=NULL;
static SGUI_NUM_VARBOX_STRUCT*  pstMinBox=NULL;
static SGUI_NUM_VARBOX_STRUCT*  pstSecBox=NULL;

static SGUI_NUM_VARBOX_STRUCT*  pstBoxArr=NULL;

static uint8_t                  uiBoxFocus=0;

HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF){
    SGUI_NUM_VARBOX_PARAM	stNumBoxInitParam;

    pstBoxArr   = pvPortMalloc(sizeof(SGUI_NUM_VARBOX_STRUCT)*6);

    pstYearBox  = pstBoxArr;
    pstMonthBox = pstBoxArr+1;
    pstDayBox   = pstBoxArr+2;
    pstHourBox  = pstBoxArr+3;
    pstMinBox   = pstBoxArr+4;
    pstSecBox   = pstBoxArr+5;

    stNumBoxInitParam.eAlignment        = SGUI_CENTER;
    stNumBoxInitParam.pstFontRes        = SGUI_FONT_REF(Deng12);
    stNumBoxInitParam.stLayout.iHeight  = 12;
    stNumBoxInitParam.stLayout.iY       = 26;

    // 初始化年份输入框
    stNumBoxInitParam.iMin              = 1970;
    stNumBoxInitParam.iMax              = 2038;
    stNumBoxInitParam.stLayout.iWidth   = 28;
    stNumBoxInitParam.stLayout.iX       = 37;

    SGUI_NumberVariableBox_Initialize(pstYearBox,&stNumBoxInitParam);

    //初始化月份
    stNumBoxInitParam.iMin              = 1;
    stNumBoxInitParam.iMax              = 12;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 81;

    SGUI_NumberVariableBox_Initialize(pstMonthBox,&stNumBoxInitParam);

    //初始化日期
    stNumBoxInitParam.iMin              = 1;
    stNumBoxInitParam.iMax              = 31;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 113;

    SGUI_NumberVariableBox_Initialize(pstDayBox,&stNumBoxInitParam);

    //初始化小时
    stNumBoxInitParam.iMin              = 0;
    stNumBoxInitParam.iMax              = 23;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 151;

    SGUI_NumberVariableBox_Initialize(pstHourBox,&stNumBoxInitParam);

    //初始化分钟
    stNumBoxInitParam.iMin              = 0;
    stNumBoxInitParam.iMax              = 59;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 176;

    SGUI_NumberVariableBox_Initialize(pstMinBox,&stNumBoxInitParam);

    //初始化秒数
    stNumBoxInitParam.iMin              = 0;
    stNumBoxInitParam.iMax              = 59;
    stNumBoxInitParam.stLayout.iWidth   = 16;
    stNumBoxInitParam.stLayout.iX       = 201;

    SGUI_NumberVariableBox_Initialize(pstSecBox,&stNumBoxInitParam);

    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters){
    SGUI_NOTICE_BOX stNoticeBox;
    SGUI_RECT stRect;
    SGUI_POINT stPoint;
    time_t time=RTC_GetCounter();
    struct tm* pstTime=gmtime(&time);
    uiBoxFocus = 0;
    SGUI_Basic_ClearScreen(pstDeviceIF);

    SGUI_Notice_FitArea(pstDeviceIF,&stNoticeBox.stLayout);

    stNoticeBox.cszNoticeText           = "设置时间:";
    stNoticeBox.pstIcon                 = NULL;
    stNoticeBox.stPalette.eEdgeColor    = 0x0A;
    stNoticeBox.stPalette.eFillColor    = 0x00;
    stNoticeBox.stPalette.uiDepthBits   = 4;

    SGUI_Notice_Repaint(pstDeviceIF,&stNoticeBox,SGUI_FONT_REF(Deng12),0);

    pstYearBox->stData.iValue   = pstTime->tm_year + 1900;
    pstMonthBox->stData.iValue  = pstTime->tm_mon+1;
    pstDayBox->stData.iValue    = pstTime->tm_mday;
    pstHourBox->stData.iValue   = pstTime->tm_hour;
    pstMinBox->stData.iValue    = pstTime->tm_min;
    pstSecBox->stData.iValue    = pstTime->tm_sec;

    SGUI_Basic_DrawRectangle(pstDeviceIF,35,24,32,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,79,24,20,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,111,24,20,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,149,24,20,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,174,24,20,16,0x0A,0x00);
    SGUI_Basic_DrawRectangle(pstDeviceIF,199,24,20,16,0x0A,0x00);

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
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters){
    // 判断月份日期越界问题
    uint16_t uiYear=pstYearBox->stData.iValue;
    uint8_t  uiMonth=pstMonthBox->stData.iValue;
    uint8_t  uiDaysOffset;
    if((uiYear%4 == 0 && uiYear%100 !=0 ) || uiYear%400 == 0){
        uiDaysOffset = 1;
    }else{
        uiDaysOffset = 0;
    }
    pstDayBox->stParam.iMax=daysOfMonth[uiMonth-1]+(uiMonth==2?uiDaysOffset:0);
    pstDayBox->stData.iValue = SGUI_MIN_OF(pstDayBox->stData.iValue,pstDayBox->stParam.iMax);

    SGUI_NumberVariableBox_Repaint(pstDeviceIF,pstYearBox,uiBoxFocus==0?SGUI_DRAW_REVERSE:SGUI_DRAW_NORMAL);
    SGUI_NumberVariableBox_Repaint(pstDeviceIF,pstMonthBox,uiBoxFocus==1?SGUI_DRAW_REVERSE:SGUI_DRAW_NORMAL);
    SGUI_NumberVariableBox_Repaint(pstDeviceIF,pstDayBox,uiBoxFocus==2?SGUI_DRAW_REVERSE:SGUI_DRAW_NORMAL);
    SGUI_NumberVariableBox_Repaint(pstDeviceIF,pstHourBox,uiBoxFocus==3?SGUI_DRAW_REVERSE:SGUI_DRAW_NORMAL);
    SGUI_NumberVariableBox_Repaint(pstDeviceIF,pstMinBox,uiBoxFocus==4?SGUI_DRAW_REVERSE:SGUI_DRAW_NORMAL);
    SGUI_NumberVariableBox_Repaint(pstDeviceIF,pstSecBox,uiBoxFocus==5?SGUI_DRAW_REVERSE:SGUI_DRAW_NORMAL);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID){
    KEY_EVENT* pstKeyEvent;
    MappedKeyCodes_t stRelease;

    *piActionID = NoAction;
    if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)){
        pstKeyEvent = (KEY_EVENT*)pstEvent;
		stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*pstKeyEvent->Data.uiReleaseCount);

        mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);

        if(containsKey(&stRelease,KeyUp)){
            *piActionID = TurnUp;
        }else if(containsKey(&stRelease,KeyDown)){
            *piActionID = TurnDown;
        }else if(containsKey(&stRelease,KeyTab)||containsKey(&stRelease,KeyRight)){
            *piActionID = SwitchFiledR;
        }else if(containsKey(&stRelease,KeyLeft)){
            *piActionID = SwitchFiledL;
        }else if(containsKey(&stRelease,KeyEnter)){
            *piActionID = SetTime;
        }else if(containsKey(&stRelease,KeyEscape)){
            *piActionID = GoBack;
        }

        vPortFree(stRelease.keyCodes);
    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID){
    struct tm* pstTime;
    if(iActionID == SwitchFiledR){
        uiBoxFocus = (uiBoxFocus+1)%6;
    }else if(iActionID == SwitchFiledL){
        uiBoxFocus = (uiBoxFocus+5)%6;
    }else if(iActionID == TurnUp){
        SGUI_NumberVariableBox_Increase(pstBoxArr+uiBoxFocus);
    }else if(iActionID == TurnDown){
        SGUI_NumberVariableBox_Decrease(pstBoxArr+uiBoxFocus);
    }else if(iActionID == SetTime){
        pstTime = pvPortMalloc(sizeof(struct tm));
        pstTime->tm_year    = pstYearBox->stData.iValue - 1900;
        pstTime->tm_mon     = pstMonthBox->stData.iValue - 1;
        pstTime->tm_mday    = pstDayBox->stData.iValue;
        pstTime->tm_hour    = pstHourBox->stData.iValue;
        pstTime->tm_min     = pstMinBox->stData.iValue;
        pstTime->tm_sec     = pstSecBox->stData.iValue;
        RTC_SetCounter(mktime(pstTime));
        RTC_WaitForLastTask();
        vPortFree(pstTime);
    }
    if(iActionID & MASK_GO_BACK){
        HMI_GoBack(NULL);
    }
    if(iActionID & MASK_REFRESH_SCREEN){
        Refresh(pstDeviceIF,NULL);
    }
    return HMI_RET_NORMAL;
}
