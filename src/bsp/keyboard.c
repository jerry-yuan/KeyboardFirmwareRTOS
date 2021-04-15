#include <bsp/keyboard.h>
#include <delay.h>

extern uint32_t SystemCoreClock;
void KEY_Initialize() {
    // 启动接口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    // 扫描线的初始化
    GPIO_InitTypeDef KEY_SCAN;
    KEY_SCAN.GPIO_Mode=GPIO_Mode_Out_PP;
    KEY_SCAN.GPIO_Speed=GPIO_Speed_2MHz;
    KEY_SCAN.GPIO_Pin=KEY_SCAN_CLK|KEY_SCAN_CLR|KEY_SCAN_DATA|KEY_SCAN_EN|KEY_SCAN_RCLK;
    GPIO_Init(GPIOC,&KEY_SCAN);

    // 读取线的初始化
    GPIO_InitTypeDef KEY_READ;
    KEY_READ.GPIO_Mode=GPIO_Mode_IPD;
    KEY_READ.GPIO_Speed=GPIO_Speed_50MHz;
    KEY_READ.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
    GPIO_Init(GPIOC,&KEY_READ);
}
void KEY_ScanEnable() {
    GPIO_ResetBits(GPIOC,KEY_SCAN_EN);
}
void KEY_ScanDisable() {
    GPIO_SetBits(GPIOC,KEY_SCAN_EN);
}
void KEY_Clear() {
    GPIO_ResetBits(GPIOC,KEY_SCAN_CLR);
    Delay_us(1);
    GPIO_SetBits(GPIOC,KEY_SCAN_CLR);
}
void KEY_SetData(uint32_t data) {
    // 清空临时存储
    KEY_Clear();
    GPIO_SetBits(GPIOC,KEY_SCAN_EN);        // 关掉输出
    // 写入数据
    for(int i=0; i<32; i++) {
        GPIO_WriteBit(GPIOC,KEY_SCAN_DATA,(data&0x80000000)>>31);
        // 发送一个移位脉冲
        GPIO_SetBits(GPIOC,KEY_SCAN_CLK);
        GPIO_ResetBits(GPIOC,KEY_SCAN_CLK);
        data=data<<1;
    }
    // 刷新输出
    GPIO_SetBits(GPIOC,KEY_SCAN_RCLK);      // 复制到锁存器中
    GPIO_ResetBits(GPIOC,KEY_SCAN_RCLK);
    GPIO_ResetBits(GPIOC,KEY_SCAN_EN);       // 开启输出

}
void KEY_ResetPulse() {
    // 关掉输出
    GPIO_SetBits(GPIOC,KEY_SCAN_EN);
    // 清除数据
    KEY_Clear();
    // 输出一个Bit
    GPIO_SetBits(GPIOC,KEY_SCAN_DATA);
    // 发送一个时钟信号
    GPIO_SetBits(GPIOC,KEY_SCAN_CLK);
    GPIO_ResetBits(GPIOC,KEY_SCAN_CLK);
    // 重置数据线
    GPIO_ResetBits(GPIOC,KEY_SCAN_DATA);
    // 转移数据
    GPIO_SetBits(GPIOC,KEY_SCAN_RCLK);
    GPIO_ResetBits(GPIOC,KEY_SCAN_RCLK);
    // 输出脉冲
    GPIO_ResetBits(GPIOC,KEY_SCAN_EN);
}
void KEY_NextColumn() {
    // 发送一个时钟信号
    GPIO_SetBits(GPIOC,KEY_SCAN_CLK);
    Delay_us(100);
    GPIO_ResetBits(GPIOC,KEY_SCAN_CLK);
    Delay_us(100);
    // 拷贝临时数据至输出寄存器
    GPIO_SetBits(GPIOC,KEY_SCAN_RCLK);
    Delay_us(100);
    GPIO_ResetBits(GPIOC,KEY_SCAN_RCLK);
}
uint8_t KEY_Read() {
    uint16_t data=GPIO_ReadInputData(GPIOC);
    return ( data >> 8 )& 0x3F;
}
