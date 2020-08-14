#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H
#include "platform_config.h"
#include "usb_type.h" 

  
 
#define USB_USART_TXFIFO_SIZE   1024	//USB���⴮�ڷ���FIFO��С		
#define USB_USART_REC_LEN	 	200		//USB���ڽ��ջ���������ֽ���

#define BULK_MAX_PACKET_SIZE 0x00000040            /* Max packet size for FullSpeed bulk transfer */
#define VCP_DATA_SIZE        0x40                  /* Should be the same as BULK_MAX_PACKET_SIZE */

#define USB_SC_PORT						GPIOC							//D+��������
#define USB_SC_PIN						GPIO_Pin_9
#define USB_SC_CLOCK					RCC_APB2Periph_GPIOC

//����һ��USB USART FIFO�ṹ��
typedef struct  
{										    
	u8  buffer[USB_USART_TXFIFO_SIZE];	//buffer
	vu16 writeptr;						//дָ��
	vu16 readptr;						//��ָ��
}_usb_usart_fifo; 
extern _usb_usart_fifo uu_txfifo;		//USB���ڷ���FIFO

extern u8  USB_USART_RX_BUF[USB_USART_REC_LEN]; //���ջ���,���USB_USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USB_USART_RX_STA;   					//����״̬���	
 
//USBͨ�ô��뺯������
void Set_USBClock(void);
void Enter_LowPowerMode(void);
void Leave_LowPowerMode(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void USB_Port_Set(u8 enable);
void IntToUnicode (u32 value,u8 *pbuf,u8 len);
void Get_SerialNum(void);




extern void     usb_putc(char data);
extern char     usb_getc(void);
extern uint32_t usb_getu24(void);
extern uint32_t usb_getu32(void);
extern void     usb_putu32(uint32_t ww);
extern void     usb_sync(void);


#endif  




























