#ifndef USBSTATE_H_INCLUDED
#define USBSTATE_H_INCLUDED
#include <FreeRTOS.h>
#include <queue.h>
#include <task/priorities.h>

void USBDeviceStateTaskInitialize();

extern QueueHandle_t hUSBDeviceStateMessageQueue;

#endif /* USBSTATE_H_INCLUDED */
