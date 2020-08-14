#include <usb.h>
#include <usb/hw_config.h>
#include <delay.h>
#include <usb_init.h>
void USB_Initialize() {
    USB_Port_Set(0); 	//USB�ȶϿ�
    USB_Port_Set(1);	//USB�ٴ�����
    //USB����
    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();
}
