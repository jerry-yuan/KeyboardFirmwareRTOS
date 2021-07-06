#include <screen/totp-authenticator/show.h>
#include <stdio.h>
#include <bsp/oled.h>
#include <bsp/w25x.h>
#include <bsp/flashMap.h>

#include <lib/keyboard.h>
#include <lib/crypto/base32.h>
#include <lib/crypto/hmac.h>
#include <lib/crypto/sha1.h>

#include <resources/Font.h>
#include <resources/fontawesome.h>

#include <math.h>

#include <SGUI_Text.h>
#include <SGUI_FontResource.h>

#include <HMI_Engine.h>
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

HMI_SCREEN_OBJECT SCREEN_TOTPAuth_Show= {SCREEN_TOTPAuth_Show_ID,&screenActions};

enum {
    NoAction = 0,
    GoBack = 1,
    RefreshScreen = 0x80,
    ReloadRecord  = 0x81
};
typedef struct {
    char strAccount[96];
    char strIssuer[32];
    char dbSecret[128];
} TOTP_Record;
typedef struct {
    uint8_t uiRecordOffsets[255];
    uint8_t uiTotalCount;
} TOTP_Header;
typedef struct {
    SGUI_RECT stDisplayArea;
    SGUI_AREA_SIZE stTextExtent;
    uint16_t uiOffset;
    const char* pcText;
} TOTP_Marquee;
typedef struct {
    TOTP_Header stHeader;
    uint8_t uiCurrentId;
    TOTP_Record stCurrent;
    char pcActiveCode[6];
    uint64_t uiCodeExpired;
    uint32_t uiSecretLength;
    char pcSecretDecoded[80];
    TOTP_Marquee stAccountMarquee;
} ScreenContext_t;


static ScreenContext_t* pstContext=NULL;

static void loadRecord() {
    TOTP_Record* pstCurrent=&pstContext->stCurrent;
    W25X_Read_Data(
        FLASH_ADDR_GA_STORAGE+sizeof(TOTP_Header)+sizeof(TOTP_Record)*pstContext->stHeader.uiRecordOffsets[pstContext->uiCurrentId],
        pstCurrent,
        sizeof(TOTP_Record)
    );
    pstContext->uiCodeExpired  = RTC_GetCounter();
    pstContext->uiSecretLength = BASE32_decode((uint8_t*)pstCurrent->dbSecret,strlen(pstCurrent->dbSecret),(uint8_t*)pstContext->pcSecretDecoded);
}
static void calcCode() {
	uint64_t stTimestamp = (RTC_GetCounter() - 28800)/30;
	uint8_t  digest[20];
	// 小端转大端
	stTimestamp = ((stTimestamp & 0x00000000ffffffff) << 32) | ((stTimestamp & 0xffffffff00000000) >> 32);
    stTimestamp = ((stTimestamp & 0x0000ffff0000ffff) << 16) | ((stTimestamp & 0xffff0000ffff0000) >> 16);
    stTimestamp = ((stTimestamp & 0x00ff00ff00ff00ff) <<  8) | ((stTimestamp & 0xff00ff00ff00ff00) >>  8);
	// 根据密钥获取hmac_sha1的值
	HMAC(SHA1,(uint8_t*)pstContext->pcSecretDecoded,pstContext->uiSecretLength,(uint8_t*)&stTimestamp,sizeof(stTimestamp),(uint8_t*)digest);
	// 根据Hash构建dbc
	uint8_t offset = digest[19] & 0x0f;
	uint32_t dbc = (digest[offset + 0] & 0x7f) << 24 |
				   (digest[offset + 1] & 0xff) << 16 |
                   (digest[offset + 2] & 0xff) << 8  |
                   (digest[offset + 3] & 0xff);
	// 截取前6位为验证码
	sprintf(pstContext->pcActiveCode,"%06d",(int)(dbc%1000000));
	pstContext->uiCodeExpired = (RTC_GetCounter()/30+1)*30;
}

HMI_ENGINE_RESULT Initialize(SGUI_SCR_DEV* pstDeviceIF) {
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Prepare(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_RECT stRect= {0,0,256,12};
    SGUI_POINT stPoint= {0,0};
    SGUI_AREA_SIZE stArea;

    // 初始化Context
    if(pstContext == NULL) {
        pstContext = pvPortMalloc(sizeof(ScreenContext_t));
        memset(pstContext,0,sizeof(ScreenContext_t));
        // 读取Headers
        W25X_Read_Data(FLASH_ADDR_GA_STORAGE,&pstContext->stHeader,sizeof(TOTP_Header));
        if(pstContext->stHeader.uiTotalCount>0) {
			loadRecord();
        }
    }
    const char* pcText = FA_KEY_SKELETON " GA身份验证器";
    // 清屏
    SGUI_Basic_ClearScreen(pstDeviceIF);
    // 绘制标题
    SGUI_Text_GetTextExtent(pcText,SGUI_FONT_REF(Deng12),&stArea);
    stRect.iX     = 1;
    stRect.iY     = 1;
    stRect.iWidth = 91;
    stRect.iHeight = 12;
    SGUI_Text_DrawText(pstDeviceIF,pcText,SGUI_FONT_REF(Deng12),&stRect,&stPoint,0x0A);
    // 绘制标题水平线
    SGUI_Basic_DrawHorizontalLine(pstDeviceIF,1,253,14,0x0A);
    // 绘制小图标
    stRect.iWidth  = 18;
    stRect.iHeight = 12;
    // 绘制小图标(账号)
    pcText = FA_USER_SHIELD " ";
    stRect.iY      = 16;
    SGUI_Text_DrawText(pstDeviceIF,pcText,SGUI_FONT_REF(Deng12),&stRect,&stPoint,0x0A);
    // 绘制小图标(发行者)
    pcText = FA_GLOBE " ";
    stRect.iY     = 28;
    SGUI_Text_DrawText(pstDeviceIF,pcText,SGUI_FONT_REF(Deng12),&stRect,&stPoint,0x0A);
    Refresh(pstDeviceIF,NULL);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT Refresh(SGUI_SCR_DEV* pstDeviceIF, const void* pstParameters) {
    SGUI_RECT stRect= {0,20,256,12};
    SGUI_POINT stPoint= {0,0};
    SGUI_AREA_SIZE stArea;
    char pcBuffer[10]= {0};
    TOTP_Record* pstRecord = &pstContext->stCurrent;
    // 绘制右上角序号标识
    sprintf(pcBuffer,"%d/%d",pstContext->uiCurrentId+1,pstContext->stHeader.uiTotalCount);
    if(pstContext->stHeader.uiTotalCount <= 0) {
        pcBuffer[0] = '-';
    }
    SGUI_Text_GetTextExtent(pcBuffer,SGUI_FONT_REF(FONT_8),&stArea);
    stRect.iX = pstDeviceIF->stSize.iWidth - stArea.iWidth - 3;
    stRect.iY = 1;
    stRect.iHeight = stArea.iHeight;
    stRect.iWidth  = stArea.iWidth;
    SGUI_Basic_DrawRectangle(pstDeviceIF,stRect.iX,stRect.iY,stRect.iWidth,stRect.iHeight,0x00,0x00);
    SGUI_Text_DrawText(pstDeviceIF,pcBuffer,SGUI_FONT_REF(FONT_8),&stRect,&stPoint,0x0A);
    if(pstContext->stHeader.uiTotalCount <= 0) {
        return HMI_RET_NORMAL;
    }
    // 绘制账号名
    stRect.iX = 20;
    stRect.iY = 16;
    stRect.iHeight = 12;
    stRect.iWidth  = 108;
    SGUI_Basic_DrawRectangle(pstDeviceIF,stRect.iX,stRect.iY,stRect.iWidth,stRect.iHeight,0x00,0x00);
    SGUI_Text_DrawText(pstDeviceIF,pstRecord->strAccount,SGUI_FONT_REF(Deng12),&stRect,&stPoint,0x0A);
    // 绘制发行者
    stRect.iY = 29;
    SGUI_Basic_DrawRectangle(pstDeviceIF,stRect.iX,stRect.iY,stRect.iWidth,stRect.iHeight,0x00,0x00);
    SGUI_Text_DrawText(pstDeviceIF,pstRecord->strIssuer,SGUI_FONT_REF(Deng12),&stRect,&stPoint,0x0A);
	// 绘制激活码
	if(pstContext->uiCodeExpired < RTC_GetCounter()){
		calcCode();
		SGUI_Basic_DrawRectangle(pstDeviceIF,130,18,126,44,0x00,0x00);
		stRect.iY = 18;
		stRect.iHeight = 44;
		stRect.iWidth  = 20;
		pcBuffer[1] = 0;
		for(uint8_t i=0;i<6;i++){
			pcBuffer[0] = pstContext->pcActiveCode[i];
			SGUI_Text_GetTextExtent(pcBuffer,SGUI_FONT_REF(LCD44),&stArea);
			stRect.iX = 151 + i*21 - stArea.iWidth;
			SGUI_Text_DrawText(pstDeviceIF,pcBuffer,SGUI_FONT_REF(LCD44),&stRect,&stPoint,0x0A);
		}
	}
	// 绘制倒计时
	sprintf(pcBuffer,"%02ds",(int)(pstContext->uiCodeExpired-RTC_GetCounter()));
	stRect.iX = 1;
	stRect.iY = 55;
	stRect.iWidth = 24;
	stRect.iHeight = 8;
	SGUI_Basic_DrawRectangle(pstDeviceIF,stRect.iX,stRect.iY,stRect.iWidth,stRect.iHeight,0x00,0x00);
	SGUI_Text_DrawText(pstDeviceIF,pcBuffer,SGUI_FONT_REF(FONT_8),&stRect,&stPoint,0x0A);
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT ProcessEvent(SGUI_SCR_DEV* pstDeviceIF,const HMI_EVENT_BASE* pstEvent, SGUI_INT* piActionID) {
    KEY_EVENT* pstKeyEvent;
    MappedKeyCodes_t stRelease;
    KeyboardUsageCode_t uiKeyCode;
    *piActionID = NoAction;
    if(pstEvent->iID == RTC_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,RTC_EVENT)) {
        *piActionID = RefreshScreen;
    } else if(pstEvent->iID == KEY_EVENT_ID && HMI_PEVENT_SIZE_CHK(pstEvent,KEY_EVENT)) {
        pstKeyEvent = (KEY_EVENT*)pstEvent;

        stRelease.cursor	= 0;
        stRelease.length	= pstKeyEvent->Data.uiReleaseCount;
        stRelease.keyCodes	= pvPortMalloc(sizeof(uint32_t)*pstKeyEvent->Data.uiReleaseCount);
        mapKeyCodes(pstKeyEvent->Data.pstRelease,stRelease.keyCodes);

        if(containsKey(&stRelease,KeyEscape)) {
            *piActionID = GoBack;
        } else if(containsKey(&stRelease,KeyLeft)) {
            pstContext->uiCurrentId=(pstContext->uiCurrentId+pstContext->stHeader.uiTotalCount-1)%pstContext->stHeader.uiTotalCount;
            *piActionID = ReloadRecord;
        } else if(containsKey(&stRelease,KeyRight)) {
            pstContext->uiCurrentId=(pstContext->uiCurrentId+1)%pstContext->stHeader.uiTotalCount;
            *piActionID = ReloadRecord;
        } else if(containsKey(&stRelease,KeyInsert)) {
        	uint8_t* pKeys = pvPortMalloc(6);
        	for(uint8_t i=0;i<6;i++){
				pKeys[i] = (pstContext->pcActiveCode[i]-'0'+9)%10+Key1;
        	}
        	sendKeysToHost(pKeys,6);
        	vPortFree(pKeys);
        } else if(containsKeys(&stRelease,&uiKeyCode,3,KeyDelete,KeyEnter,KeyDeleteForward)){
        	sendKeysToHost(&uiKeyCode,1);
        }
        vPortFree(stRelease.keyCodes);
    }
    return HMI_RET_NORMAL;
}
HMI_ENGINE_RESULT PostProcess(SGUI_SCR_DEV* pstDeviceIF,  HMI_ENGINE_RESULT eProcResult, SGUI_INT iActionID) {
    if(iActionID == GoBack) {
        vPortFree(pstContext);
        pstContext = NULL;
        HMI_GoBack(NULL);
    } else if(iActionID == ReloadRecord) {
        loadRecord();
    }

    if(iActionID & 0x80) {
        Refresh(pstDeviceIF,NULL);
    }
    return HMI_RET_NORMAL;
}


