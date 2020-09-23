#include <screen/clock/show.h>
#include <lib/keyboard.h>
#include <task/keyboard.h>

#include <resources/Font.h>

#include <SGUI_Basic.h>
#include <SGUI_Text.h>
#include <SGUI_FontResource.h>
#include <SGUI_IconResource.h>
#include <SGUI_Notice.h>

#include <time.h>
#include <math.h>


enum{
    NoAction      = 0x00,
    RefreshScreen = 0x01,
    GoBack        = 0x02,
    DoModify      = 0x03
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

HMI_SCREEN_OBJECT SCREEN_Clock_Show={SCREEN_Clock_Show_ID,&screenActions};

HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF){
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters){
    SGUI_NOTICE_BOX noticeBox;

    SGUI_Basic_ClearScreen(pstDeviceIF);

    SGUI_Notice_FitArea(pstDeviceIF,&noticeBox.stLayout);
    noticeBox.cszNoticeText = "当前时间:";
    noticeBox.pstIcon = &SGUI_RES_ICON_INFORMATION_16;

    noticeBox.stPalette.eEdgeColor = 0x0A;
    noticeBox.stPalette.eFillColor = 0x00;
    noticeBox.stPalette.eTextColor = 0x0F;
    noticeBox.stPalette.uiDepthBits= 4;

    SGUI_Notice_Repaint(pstDeviceIF,&noticeBox,SGUI_FONT_REF(Deng12),0);

    Refresh(pstDeviceIF,pstParameters);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters){
    char buff[30]={0};
    time_t timestamp=RTC_GetCounter();
    struct tm* pstTime;
    SGUI_POINT point;
    SGUI_RECT  rect;
    uint8_t uiOriginX;
    uint8_t uiOriginY;
    uint8_t uiPointerX;
    uint8_t uiPointerY;
    float   fDegree;

    point.iX=point.iY=0;

    rect.iX = 35;
    rect.iY = 30;
    rect.iWidth = 200;
    rect.iHeight = 12;

    pstTime=gmtime(&timestamp);
    strftime(buff,sizeof(buff), "%Y年%m月%d日 %H:%M:%S", pstTime);
	SGUI_Basic_DrawRectangle(pstDeviceIF,35,30,200,12,0,0);
    SGUI_Text_DrawText(pstDeviceIF,buff,SGUI_FONT_REF(Deng12),&rect,&point,0x0A);
    //画表盘
    SGUI_Basic_DrawCircle(pstDeviceIF,223,31,23,0x0A,0x00);
    for(uint8_t i=0;i<12;i++){
        uiOriginX   =  20 * sin(i*M_PI/6)+223;
        uiOriginY   = -20 * cos(i*M_PI/6)+31;
        uiPointerX  =  23 * sin(i*M_PI/6)+223;
        uiPointerY  = -23 * cos(i*M_PI/6)+31;
        SGUI_Basic_DrawLine(pstDeviceIF,uiOriginX,uiOriginY,uiPointerX,uiPointerY,0x08);
    }
    //时针
    fDegree = (pstTime->tm_hour % 12)*M_PI/6;
    uiPointerX =  13*sin(fDegree)+223;
    uiPointerY = -13*cos(fDegree)+31;
    SGUI_Basic_DrawLine(pstDeviceIF,223,31,uiPointerX,uiPointerY,0x0A);
    // 分针
    fDegree = (pstTime->tm_min % 60)*M_PI/30;
    uiPointerX =  17*sin(fDegree)+223;
    uiPointerY = -17*cos(fDegree)+31;
    SGUI_Basic_DrawLine(pstDeviceIF,223,31,uiPointerX,uiPointerY,0x0A);
    // 秒针
    fDegree = (pstTime->tm_sec % 60)*M_PI/30;
    uiPointerX =  20*sin(fDegree)+223;
    uiPointerY = -20*cos(fDegree)+31;
    SGUI_Basic_DrawLine(pstDeviceIF,223,31,uiPointerX,uiPointerY,0x0A);
    // 表心
    SGUI_Basic_DrawCircle(pstDeviceIF,223,31,1,0x0A,0x0A);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID){
    KEY_EVENT* pstKeyEvent;
    MappedKeyCodes_t stRelease;
    *piActionID = NoAction;
    if(pstEvent->iID == RTC_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,RTC_EVENT)){
        *piActionID = RefreshScreen;
    }else if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)){
        pstKeyEvent = (KEY_EVENT*)pstEvent;

		stRelease.cursor	= 0;
        stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*pstKeyEvent->Data.uiReleaseCount);
		mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);

        if(containsKey(&stRelease,KeyEscape)){
            *piActionID = GoBack;
        }else if(containsKey(&stRelease,KeyInsert)){
            *piActionID = DoModify;
        }

        vPortFree(stRelease.keyCodes);
    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID){
    if(iActionID == RefreshScreen){
        Refresh(pstDeviceIF,NULL);
    }else if(iActionID == GoBack){
        HMI_GoBack(NULL);
    }else if(iActionID == DoModify){
        HMI_SwitchScreen(SCREEN_Clock_Edit_ID,NULL);
    }
    return HMI_RET_NORMAL;
}
