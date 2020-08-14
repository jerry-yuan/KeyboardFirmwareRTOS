#ifndef KEYBOARD_SCAN_H_INCLUDED
#define KEYBOARD_SCAN_H_INCLUDED
#include <stm32f10x.h>

#define KEY_SCAN_EN GPIO_Pin_0
#define KEY_SCAN_CLR GPIO_Pin_1
#define KEY_SCAN_RCLK GPIO_Pin_2
#define KEY_SCAN_CLK GPIO_Pin_3
#define KEY_SCAN_DATA GPIO_Pin_4

void KEY_Initialize();
void KEY_NextColumn();
void KEY_ResetPulse();
uint8_t KEY_Read();

extern uint8_t* keyboardPushStatus;
extern uint8_t keyboardPushedFlag;

#endif /* KEYBOARD_H_INCLUDED */
