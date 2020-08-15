#include <bsp/rtc.h>

#include <stm32f10x.h>

void RTC_Initialize(){
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 使能 PWR 和 Backup 时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* 允许访问 Backup 区域 */
	PWR_BackupAccessCmd(ENABLE);

	/* 使能 LSE */
	RCC_LSEConfig(RCC_LSE_ON);

	/* 等待 LSE 准备好 */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

	/* 选择 LSE 作为 RTC 时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

	/* 使能 RTC 时钟 */
	RCC_RTCCLKCmd(ENABLE);

	/* 等待 RTC 寄存器 同步
	 * 因为RTC时钟是低速的，内环时钟是高速的，所以要同步
	 */
	RTC_WaitForSynchro();

	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();

	/* 设置 RTC 分频: 使 RTC 周期为1s  */
	/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) = 1HZ */
	RTC_SetPrescaler(32767);

	/* 确保上一次 RTC 的操作完成 */
	RTC_WaitForLastTask();

	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
