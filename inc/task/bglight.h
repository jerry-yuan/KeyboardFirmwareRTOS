#ifndef TASK_BG_LIGHT_H_INCLUDED
#define TASK_BG_LIGHT_H_INCLUDED

#include <FreeRTOS.h>
#include <task.h>

void BgLightTaskTaskInitialize();

extern TaskHandle_t hBgLightTask;

#endif /* TASK_BG_LIGHT_H_INCLUDED */
