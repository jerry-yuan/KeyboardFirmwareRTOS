#ifndef RTC_H_INCLUDED
#define RTC_H_INCLUDED

#include <FreeRTOS.h>
#include <task.h>
#include <event_groups.h>

#define RTC_EVENT_BIT (0x1<<23)

extern TaskHandle_t hRTCTask;

extern EventGroupHandle_t hRTCEvent;

void RTCTaskInitialize();

#endif /* RTC_H_INCLUDED */
