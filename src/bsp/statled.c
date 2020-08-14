#include<stm32f10x.h>
#include<bsp/statled.h>

void STAT_Initialize() {
    GPIO_InitTypeDef gpio;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Mode=GPIO_Mode_Out_PP;
    gpio.GPIO_Pin=GPIO_Pin_0;
    gpio.GPIO_Speed=GPIO_Speed_2MHz;
    GPIO_Init(GPIOB,&gpio);
}

void STAT_SetState(uint8_t state){
    if(state){
        STAT_Set();
    }else{
        STAT_Reset();
    }
}
void STAT_Set() {
    GPIO_SetBits(GPIOB,GPIO_Pin_0);
}
void STAT_Reset() {
    GPIO_ResetBits(GPIOB,GPIO_Pin_0);
}
void STAT_Revert(){
    GPIOB->ODR ^= GPIO_Pin_0;
}
