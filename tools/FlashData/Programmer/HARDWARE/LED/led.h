#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#include "platform_config.h"


#ifdef USE_STLINK_V2_AS_SERPROG

#define LED0 PAout(9)	  //2个LED，一高一低分别亮�
#define LED1 PAout(8)	 //STLINK_V2 悬空
#define LED2 PAout(7)	 //STLINK_V2 悬空的
 
//红色是PA9



#else

#define LED0 PBout(7)	
#define LED1 PBout(8)	
#define LED2 PBout(4)	

//红色是PC7
//音频蓝色PC8
//蓝牙蓝色B4
#endif

void LED_Init(void);//初始化

		 				    
#endif
