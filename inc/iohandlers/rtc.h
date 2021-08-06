#ifndef IOHANDLER_RTC_H_INCLUDED
#define IOHANDLER_RTC_H_INCLUDED

#include <task/iotask.h>

// 0x10     RTC相关
uint8_t IOH_rtcGetCounter(Buffer_t* pstRequest,Buffer_t* pstResponse);
uint8_t IOH_rtcSetCounter(Buffer_t* pstRequest,Buffer_t* pstResponse);

#endif /* IOHANDLER_RTC_H_INCLUDED */
