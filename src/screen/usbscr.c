#include <usbscr.h>
#include <stdio.h>
#include <string.h>
#include <SGUI_IconResource.h>
#include <SGUI_FontResource.h>
#include <SGUI_Text.h>
#include <SGUI_Notice.h>

#include <FreeRTOS.h>
#include <bsp/oled.h>
#include <bsp/W25Q64.h>
#include <USB/usb_pwr.h>
#include <resources/Font.h>

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

HMI_SCREEN_OBJECT SCREEN_USB= {SCREEN_USB_State_ID,&screenActions};

static SGUI_NOTICE_BOX* noticeBox=NULL;
static char*            noticeStr=NULL;
HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    if(noticeBox==NULL) {
        noticeBox=pvPortMalloc(sizeof(SGUI_NOTICE_BOX));
        SGUI_Notice_FitArea(pstDeviceIF,&noticeBox->stLayout);

        noticeBox->cszNoticeText = noticeStr = pvPortMalloc(100);//"中文测试\nрусский тест";
        noticeBox->pstIcon = &SGUI_RES_ICON_INFORMATION_16;
        noticeBox->stPalette.uiDepthBits=4;
        noticeBox->stPalette.eEdgeColor=0xA;
        noticeBox->stPalette.eFillColor=0x0;
		noticeBox->stPalette.eTextColor	= 0x0A;
    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_Basic_ClearScreen(pstDeviceIF);
    SGUI_Notice_Repaint(pstDeviceIF,noticeBox,SGUI_FONT_REF(Deng12),0);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_Notice_Repaint(pstDeviceIF,noticeBox,SGUI_FONT_REF(Deng12),0);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    *piActionID = HMI_SCREEN_ID_ANY;
    if(pstEvent->iID==USB_STATE_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,USB_STATE_EVENT)) {
        USB_STATE_EVENT* pstUSBStateEvent=(USB_STATE_EVENT*)pstEvent;
        switch(pstUSBStateEvent->Data.uiDeviceState) {
        case UNCONNECTED:
            sprintf(noticeStr,"当前状态:连接已断开!\n\n请检查与计算机的连接!");
            break;
        case ATTACHED:
            sprintf(noticeStr,"当前状态:连接建立成功!");
            *piActionID = SCREEN_Keyboard_State_ID;
            break;
        case POWERED:
            sprintf(noticeStr,"当前状态:已上电!");
            break;
        case SUSPENDED:
            sprintf(noticeStr,"当前状态:等待计算机唤醒...\n\n请稍等,长时间处于此状态可能是USB连接有问题...");
            break;
        case ADDRESSED:
            sprintf(noticeStr,"当前状态:USB地址已分配!");
            break;
        case CONFIGURED:
            sprintf(noticeStr,"当前状态:USB配置完成!");
            break;
        }
        Refresh(pstDeviceIF,NULL);
    }

    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID != HMI_SCREEN_ID_ANY){
        HMI_SwitchScreen(iActionID,NULL);
    }
    return HMI_RET_NORMAL;
}


