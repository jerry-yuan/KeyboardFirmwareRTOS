#ifndef KEYSCAN_H_INCLUDED
#define KEYSCAN_H_INCLUDED

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

typedef struct KeyUpdateInfo {
    uint8_t row;
    uint8_t column;
    struct KeyUpdateInfo* next;
} KeyUpdateInfo_t;

typedef struct {
    KeyUpdateInfo_t* pressed;
    uint8_t pressedCount;
    KeyUpdateInfo_t* release;
    uint8_t releaseCount;
} KeyUpdateEvent_t;

void keyScanTaskInitialize(void);
void clearKeyUpdateInfoList(KeyUpdateInfo_t* head);


extern TaskHandle_t hKeyScanTask;
extern QueueHandle_t keyUpdateEventQueue;

#endif /* KEYSCAN_H_INCLUDED */
