#include <bsp/bglight.h>
#include <delay.h>
void LED_Initialize(){
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

    GPIO_InitStructure.GPIO_Mode     = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin      = 0;
    GPIO_InitStructure.GPIO_Pin     |= LED_ENABLE_PIN;
    GPIO_InitStructure.GPIO_Pin     |= LED_RCLK_PIN;
    GPIO_InitStructure.GPIO_Pin     |= LED_CLEAR_PIN;
    GPIO_InitStructure.GPIO_Pin     |= LED_COL_CLK_PIN;
    GPIO_InitStructure.GPIO_Pin     |= LED_COL_DATA_PIN;
    GPIO_InitStructure.GPIO_Pin     |= LED_ROW_CLK_PIN;
    GPIO_InitStructure.GPIO_Pin     |= LED_ROW_DATA_PIN;
    GPIO_InitStructure.GPIO_Speed    = GPIO_Speed_2MHz;

    GPIO_Init(LED_PORT,&GPIO_InitStructure);

    // 关灯
    LED_TurnOff();
    // 清除临时寄存器
    LED_Clear();
    // 清除输出寄存器
    LED_Swap();
    // 开灯
    LED_TurnOn();

}

void LED_TurnOn(){
    GPIO_ResetBits(LED_PORT,LED_ENABLE_PIN);
}
void LED_TurnOff(){
    GPIO_SetBits(LED_PORT,LED_ENABLE_PIN);
}
void LED_Clear(){
    //一个脉冲清空临时存储寄存器
    GPIO_ResetBits(LED_PORT,LED_CLEAR_PIN);
    Delay_us(1);
    GPIO_SetBits(LED_PORT,LED_CLEAR_PIN);
}
void LED_Swap(){
    GPIO_SetBits(GPIOB,LED_RCLK_PIN);
    Delay_us(1);
    GPIO_ResetBits(GPIOB,LED_RCLK_PIN);
}

void LED_PrepareRowData(uint8_t data){
    for(int i=0;i<8;i++){
        if(data&0x80){
            GPIO_SetBits(LED_PORT,LED_ROW_DATA_PIN);
        }else{
            GPIO_ResetBits(LED_PORT,LED_ROW_DATA_PIN);
        }
        //Toggle
        GPIO_SetBits(LED_PORT,LED_ROW_CLK_PIN);
        Delay_us(1);
        GPIO_ResetBits(LED_PORT,LED_ROW_CLK_PIN);
        // onstage
        LED_Swap();
        data <<= 1;
    }
}

void LED_ResetColScan(){
    GPIO_SetBits(LED_PORT,LED_COL_DATA_PIN);
    LED_NextCol();
    GPIO_ResetBits(LED_PORT,LED_COL_DATA_PIN);
}
void LED_NextCol(){
    //Toggle
    GPIO_SetBits(GPIOB,LED_COL_CLK_PIN);
    Delay_us(1);
    GPIO_ResetBits(GPIOB,LED_COL_CLK_PIN);
}
void LED_SetColData(uint32_t data){
    // 清空临时存储
    //LED_Clear();
    GPIO_SetBits(GPIOB,LED_ENABLE_PIN);        // 关掉输出
    // 写入数据
    for(int i=0; i<32; i++) {
        if( data & 0x80000000){
            GPIO_SetBits(LED_PORT,LED_COL_DATA_PIN);
        }else{
            GPIO_ResetBits(LED_PORT,LED_COL_DATA_PIN);
        }
        // 发送一个移位脉冲
        GPIO_SetBits(GPIOB,LED_COL_CLK_PIN);
        Delay_us(1);
        GPIO_ResetBits(GPIOB,LED_COL_CLK_PIN);
        data <<= 1;
    }
    // 刷新输出
    GPIO_SetBits(GPIOB,LED_RCLK_PIN);      // 复制到锁存器中
    Delay_us(1);
    GPIO_ResetBits(GPIOB,LED_RCLK_PIN);
    GPIO_ResetBits(GPIOB,LED_ENABLE_PIN);       // 开启输出
}
