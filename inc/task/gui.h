#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED
#include <bsp/oled.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <SGUI_Basic.h>
#include <HMI_Engine.h>
#include <task/priorities.h>

void guiTaskInitialize();

extern HMI_ENGINE_OBJECT*   hmiEngine;

extern QueueHandle_t        hEventQueue;
#endif /* GUI_H_INCLUDED */
