#include <delay.h>

/**
 * 使用DWT实现的高精度延时函数
 */

#define  DWT_CR      *(__IO uint32_t *)0xE0001000 //< 0xE0001000 DWT_CTRL RW The Debug Watchpoint and Trace (DWT) unit
#define  DWT_CYCCNT  *(__IO uint32_t *)0xE0001004 //< 0xE0001004 DWT_CYCCNT RW Cycle Count register,
#define  DEM_CR      *(__IO uint32_t *)0xE000EDFC //< 0xE000EDFC DEMCR RW Debug Exception and Monitor Control Register.


#define  DEM_CR_TRCENA                   (1 << 24)  //DEMCR的DWT使能位
#define  DWT_CR_CYCCNTENA                (1 <<  0)  //DWT的SYCCNT使能位

#define TIMEUNIT_US 10e6
#define TIMEUNIT_MS 10e3
#define TIMEUNIT_S  1

extern uint32_t SystemCoreClock;

void Delay_TimerInitialize(void) {
    //SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    // 使能DWT外设
    DEM_CR |= (uint32_t)DEM_CR_TRCENA;

    // DWT CYCCNT寄存器计数清0
    //DWT_CYCCNT = (uint32_t)0u;

    // 使能Cortex-M DWT CYCCNT寄存器
    DWT_CR |= (uint32_t)DWT_CR_CYCCNTENA;
}
static void Delay(__IO uint32_t value,uint32_t timeUnit){
    uint32_t beginTick=(uint32_t)DWT_CYCCNT;
    uint32_t targetTick=beginTick+SystemCoreClock/timeUnit*value;
    if(beginTick>targetTick){
        while(DWT_CYCCNT>targetTick);
    }
    while(DWT_CYCCNT<targetTick);
}
void Delay_us(__IO uint32_t us) {
    Delay(us,TIMEUNIT_US);
}
void Delay_ms(__IO uint32_t ms) {
    Delay(ms,TIMEUNIT_MS);
}
//由于Systick被占用,以下函数报废
/*
void Delay_us(uint32_t us) {
    SysTick->LOAD=us*9;
    SysTick->VAL=0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while(SysTick->CTRL>>16 !=1 );
    SysTick->CTRL |=SysTick_CTRL_ENABLE_Pos;
}
void Delay_ms(__IO uint32_t ms) {
    SysTick->LOAD=ms*9000;
    SysTick->VAL=0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while(SysTick->CTRL>>16 !=1 );
    SysTick->CTRL |=SysTick_CTRL_ENABLE_Pos;
}*/
