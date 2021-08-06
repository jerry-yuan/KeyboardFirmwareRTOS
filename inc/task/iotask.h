#ifndef IOTASK_H_INCLUDED
#define IOTASK_H_INCLUDED

#include <FreeRTOS.h>
#include "task.h"

#define R_ACK  0x55
#define R_NAK  0xAA

typedef struct{
	uint8_t* pBuffer;
	uint16_t uiLength;
} Buffer_t;

typedef uint8_t (*RequestHandler_t)(Buffer_t* pstRequest,Buffer_t* pstResponse);

extern const RequestHandler_t requestHandlers[256];
void IOTaskInitialize();


#endif /* IOTASK_H_INCLUDED */
