#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#include "platform_config.h"


#ifdef USE_STLINK_V2_AS_SERPROG

#define LED0 PAout(9)	  //2¸öLED£¬Ò»¸ßÒ»µÍ·Ö±ðÁÁÁ
#define LED1 PAout(8)	 //STLINK_V2 Ðü¿Õ
#define LED2 PAout(7)	 //STLINK_V2 Ðü¿ÕµÄ
 
//ºìÉ«ÊÇPA9



#else

#define LED0 PBout(7)	
#define LED1 PBout(8)	
#define LED2 PBout(4)	

//ºìÉ«ÊÇPC7
//ÒôÆµÀ¶É«PC8
//À¶ÑÀÀ¶É«B4
#endif

void LED_Init(void);//³õÊ¼»¯

		 				    
#endif
