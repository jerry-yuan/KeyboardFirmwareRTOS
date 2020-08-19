#ifndef BGLIGHT_H_INCLUDED
#define BGLIGHT_H_INCLUDED

#include <stm32f10x.h>
#include <stm32f10x_gpio.h>

#define LED_PORT            GPIOB
#define LED_ENABLE_PIN      GPIO_Pin_8
#define LED_RCLK_PIN        GPIO_Pin_9
#define LED_CLEAR_PIN       GPIO_Pin_10
#define LED_COL_DATA_PIN    GPIO_Pin_11
#define LED_COL_CLK_PIN     GPIO_Pin_12
#define LED_ROW_DATA_PIN    GPIO_Pin_13
#define LED_ROW_CLK_PIN     GPIO_Pin_14

void LED_Initialize();
void LED_TurnOn();
void LED_TurnOff();
void LED_Clear();
void LED_Swap();

void LED_PrepareRowData(uint8_t data);
void LED_ResetColScan();
void LED_NextCol();
void LED_SetColData(uint32_t data);

#endif /* LED_H_INCLUDED */
