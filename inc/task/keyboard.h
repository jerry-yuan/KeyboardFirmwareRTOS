#ifndef KEYBOARD_H_INCLUDED
#define KEYBOARD_H_INCLUDED

#include <stm32f10x.h>
#include <FreeRTOS.h>

#include <task.h>
#include <queue.h>
#include <event_groups.h>


#define KEY_STATE_EVENT_UPDATE          (1<<23)

extern TaskHandle_t hKeyboardTask;
extern TaskHandle_t hKeyboardStatTask;

extern EventGroupHandle_t hKeyboardStateUpdateEvent;
extern uint32_t keyboardStatus;
void keyboardTaskInitialize();



#endif /* KEYBOARD_H_INCLUDED */
