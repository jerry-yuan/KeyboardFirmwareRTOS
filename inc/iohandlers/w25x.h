#ifndef IOHANDLER_W25X_H_INCLUDED
#define IOHANDLER_W25X_H_INCLUDED

#include <task/iotask.h>

// 0x20     W25X Flash相关
uint8_t IOH_w25xInfo(Buffer_t* pstContext,Buffer_t* pstResponse);
uint8_t IOH_w25xRead(Buffer_t* pstRequest,Buffer_t* pstResponse);
uint8_t IOH_w25xWrite(Buffer_t* pstRequest,Buffer_t* pstResponse);
#endif /* IOHANDLER_W25X_H_INCLUDED */
