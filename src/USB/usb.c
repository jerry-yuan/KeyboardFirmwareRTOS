#include <usb.h>
#include <usb/hw_config.h>
#include <delay.h>
#include <usb_init.h>

FIFO_t* pUSBFIFO=NULL;

void USB_Initialize() {

	pUSBFIFO = pvPortMalloc(sizeof(FIFO_t));
	FIFO_Inititalize(pUSBFIFO,pvPortMalloc(100),100);

    USB_Port_Set(0); 	//USB先断开
    USB_Port_Set(1);	//USB再次连接
    //USB配置
    USB_Interrupts_Config();
    Set_USBClock();
    USB_Init();
}
