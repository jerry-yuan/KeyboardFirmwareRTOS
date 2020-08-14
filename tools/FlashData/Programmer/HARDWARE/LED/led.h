#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#include "platform_config.h"


#ifdef USE_STLINK_V2_AS_SERPROG

#define LED0 PAout(9)	  //2��LED��һ��һ�ͷֱ����
#define LED1 PAout(8)	 //STLINK_V2 ����
#define LED2 PAout(7)	 //STLINK_V2 ���յ�
 
//��ɫ��PA9



#else

#define LED0 PBout(7)	
#define LED1 PBout(8)	
#define LED2 PBout(4)	

//��ɫ��PC7
//��Ƶ��ɫPC8
//������ɫB4
#endif

void LED_Init(void);//��ʼ��

		 				    
#endif
