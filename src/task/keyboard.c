#include <task/keyboard.h>
#include <task/keyscan.h>
#include <task/priorities.h>
#include <task/gui.h>

#include <bsp/oled.h>
#include <bsp/W25Q64.h>

#include <usb/hw_config.h>
#include <usb_lib.h>

#include <screen/consts.h>

//#define STAMP() printf("STAMP:%s:%d\r\n",__FILE__,__LINE__)

const uint32_t keyboardMap[6][21]= {
    //1      2      3      4      5      6      7      8      9      10     11     12     13     14     15     16     17     18     19     20     21
    //None-ESC   None-None  None-F1    None-F2    None-F3    None-F4    None-F5    None-F6    None-F7    None-F8    None-F9    None-F10   None-F11   None-F12   None-PtSr  None-ScLK  None-PAUS  None-None  None-None  None-None  None-None
    {0x00000029,0x00000000,0x0000003A,0x0000003B,0x0000003C,0x0000003D,0x0000003E,0x0000003F,0x00000040,0x00000041,0x00000042,0x00000043,0x00000044,0x00000045,0x00040046,0x00000047,0x00080048,0x00000000,0x00000000,0x00000000,0x00000000},
    //None-`     None-1!    None-2@    None-3#    None-4$    None-5%    None-6^    None-7&    None-8*    None-9(    None-0)    None--_    None-=+    None-BS    None-Inst  No 0ne-Home  None-PgUp  None-NuLk  None-/     None-*     None--
    {0x00000035,0x0000001E,0x0000001F,0x00000020,0x00000021,0x00000022,0x00000023,0x00000024,0x00000025,0x00000026,0x00000027,0x0000002D,0x0000002E,0x0000002A,0x00000049,0x0000004A,0x0000004B,0x00000053,0x00000054,0x00000055,0x00000056},
    //None-Tab   None-Q     None-W     None-E     None-R     None-T     None-Y     None-U     None-I     None-O     None-P     None-[{    None-]}    None-\|    None-DELE  None-End   None-PgDn  None-Num7  None-Num8  None-Num9  None-None
    {0x0000002B,0x00000014,0x0000001A,0x00000008,0x00000015,0x00000017,0x0000001C,0x00000018,0x0000000C,0x00000012,0x00000013,0x0000002F,0x00000030,0x00000031,0x0000004C,0x0000004D,0x0000004E,0x0000005F,0x00000060,0x00000061,0x00000027},
    //None-Cap   None-A     None-S     None-D     None-F     None-G     None-H     None-J     None-K     None-L     None-;:    None-'"    None-None  None-ENTR  None-None  None-None  None-None  None-Num4  None-NUM5  None-Num6  None-NPls
    {0x00000039,0x00000004,0x00000016,0x00000007,0x00000009,0x0000000A,0x0000000B,0x0000000D,0x0000000E,0x0000000F,0x00000033,0x00000034,0x00000000,0x00000028,0x00000000,0x00000000,0x00000000,0x0000005C,0x0000005D,0x0000005E,0x00000057},
    //None-LSft  None-Z     None-X     None-C     None-V     None-B     None-N     None-M     None-,<    None-.>    None-/?    None-None  None-RSft  None-None  None-None  None-Up    None-None  None-Num1  None-Num2  None-Num3  None-None
    {0x00000102,0x0000001D,0x0000001B,0x00000006,0x00000019,0x00000005,0x00000011,0x00000010,0x00000036,0x00000037,0x00000038,0x00000000,0x00000120,0x00000000,0x00000000,0x00100052,0x00000000,0x00000059,0x0000005A,0x0000005B,0x00000027},
    //None-LCtl  None-LWin  None-LAlt  None-None  None-None  None-Spac  None-None  None-None  None-None  None-LAlt  Fn         None-Cont  None-RCtl  None-None  None-Left  None-Down  None-Rigt  None-Num0  None-None  None-.Del  None-NEnt
    {0x00000101,0x00000108,0x00000104,0x00000000,0x00000000,0x0000002C,0x00000000,0x00000000,0x00000000,0x00000140,0x80000000,0x00000065,0x00020110,0x00000000,0x00020050,0x00200051,0x0001004F,0x00000062,0x00000000,0x00000063,0x00000058}
};

//键盘事件发送任务
TaskHandle_t hKeyboardTask=NULL;
//键盘状态显示任务
TaskHandle_t hKeyboardStatTask=NULL;
//键盘状态更新事件
EventGroupHandle_t hKeyboardStateUpdateEvent=NULL;
//键盘状态
uint32_t keyboardStatus=0x00;
static void clearList(KeyUpdateInfo_t* head) {
    KeyUpdateInfo_t* temp;
    while(head!=NULL) {
        temp=head;
        head=head->next;
        vPortFree(temp);
    }
}

static void mapKeyCodes(KeyUpdateInfo_t* pCurrent,uint32_t* pKeyCode) {
    uint32_t* pFirstKeyCode=pKeyCode;
    printf("mapping code:");
    while(NULL != pCurrent) {
        printf("(%d,%d) ",pCurrent->row,pCurrent->column);
        *pKeyCode=keyboardMap[pCurrent->row][pCurrent->column];
        if((*pKeyCode & KEY_Fn_BIT_MASK ) && (pKeyCode!=pFirstKeyCode)) {
            *pKeyCode       = *pKeyCode ^ *pFirstKeyCode;
            *pFirstKeyCode  = *pKeyCode ^ *pFirstKeyCode;
            *pKeyCode       = *pKeyCode ^ *pFirstKeyCode;
        }
        pKeyCode++;
        pCurrent=pCurrent->next;
    }
    printf("\r\n");
}

void keyboardTask(void) {
    BaseType_t xReturn=pdPASS;

    KeyUpdateEvent_t keyUpdateEvent;
    KEY_EVENT* pEvent=NULL;

    while(1) {
        xReturn=xQueueReceive(keyUpdateEventQueue,&keyUpdateEvent,portMAX_DELAY);
        if(xReturn==pdFALSE) {
            // 读取失败就跳过
            continue;
        }
        pEvent                  = pvPortMalloc(sizeof(KEY_EVENT));
        pEvent->Head.iID        = KEY_EVENT_ID;
        pEvent->Head.iSize      = sizeof(KEY_EVENT);

        pEvent->Data.stPress.keyCodes       = (uint32_t*)pvPortMalloc(sizeof(uint32_t)*keyUpdateEvent.pressedCount);
        pEvent->Data.stPress.cursor         = 0;
        pEvent->Data.stPress.length         = keyUpdateEvent.pressedCount;

        pEvent->Data.stRelease.keyCodes     = (uint32_t*)pvPortMalloc(sizeof(uint32_t)*keyUpdateEvent.releaseCount);
        pEvent->Data.stRelease.cursor       = 0;
        pEvent->Data.stRelease.length       = keyUpdateEvent.releaseCount;

        mapKeyCodes(keyUpdateEvent.pressed,pEvent->Data.stPress.keyCodes);
        mapKeyCodes(keyUpdateEvent.release,pEvent->Data.stRelease.keyCodes);

        clearList(keyUpdateEvent.pressed);
        clearList(keyUpdateEvent.release);

        xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);

    }
}
static void keyboardStatusTask() {
    EventBits_t xEvent;
    KEYBOARD_STATE_EVENT* pEvent;
    while(1) {
        xEvent=xEventGroupWaitBits(hKeyboardStateUpdateEvent,
                                   KEY_STATE_EVENT_UPDATE,
                                   pdTRUE,
                                   pdFALSE,
                                   portMAX_DELAY);

        if(xEvent & KEY_STATE_EVENT_UPDATE) {
            pEvent = pvPortMalloc(sizeof(KEYBOARD_STATE_EVENT));

            pEvent->Head.iID    = KEYBOARD_STATE_EVENT_ID;
            pEvent->Head.iSize  = sizeof(KEYBOARD_STATE_EVENT);
            pEvent->Data        = keyboardStatus;

            xQueueSend(hEventQueue,&pEvent,portMAX_DELAY);

        }
        xEventGroupClearBits(hKeyboardStateUpdateEvent,0xff);

    }
}

void keyboardTaskInitialize() {
    BaseType_t xReturn = pdPASS;

    hKeyboardStateUpdateEvent=xEventGroupCreate();
    xReturn = xTaskCreate((TaskFunction_t)keyboardTask,"keyboard",128,NULL,TASK_KEYBOARD_PRIORITY,&hKeyboardTask);
    if(xReturn == pdPASS) {
        printf("Create keyboard task success!\r\n");
    }
    xReturn = xTaskCreate((TaskFunction_t)keyboardStatusTask,"keyboard-state",128,NULL,TASK_KEYBOARD_STATE_PRIORITY,&hKeyboardStatTask);
    if(xReturn == pdPASS) {
        printf("Create keyboard stat task success!\r\n");
    }
}
