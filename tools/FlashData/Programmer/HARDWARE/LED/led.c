#include "led.h"
   
#ifdef USE_STLINK_V2_AS_SERPROG
//��ʼ��PA7PA8PA9Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��P �˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;				 
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					
 GPIO_SetBits(GPIOA,GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9);						 

}
 
#else 
//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PA,PD�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8;				 
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					
 GPIO_SetBits(GPIOC,GPIO_Pin_7|GPIO_Pin_8);						 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
 GPIO_Init(GPIOB, &GPIO_InitStructure);
 GPIO_SetBits(GPIOB,GPIO_Pin_4);	
}
 
#endif
