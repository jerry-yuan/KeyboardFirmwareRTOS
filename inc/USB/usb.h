#ifndef USB_H_INCLUDED
#define USB_H_INCLUDED

#include <lib/FIFOBuffer.h>

void USB_Initialize();

extern FIFO_t* pUSBFIFO;

#endif /* USB_H_INCLUDED */
