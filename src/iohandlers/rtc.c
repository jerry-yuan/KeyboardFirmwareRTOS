#include <iohandlers/rtc.h>

uint8_t IOH_rtcGetCounter(Buffer_t* pstRequest,Buffer_t* pstResponse){
	pstResponse->pBuffer=pvPortMalloc(sizeof(uint32_t));
	*((uint32_t*)pstResponse->pBuffer) = RTC_GetCounter();
	pstResponse->uiLength = sizeof(uint32_t);
	return R_ACK;
}
uint8_t IOH_rtcSetCounter(Buffer_t* pstRequest,Buffer_t* pstResponse){
	uint32_t* pTimestamp=(uint32_t*)pstRequest->pBuffer;
	RTC_SetCounter(*pTimestamp);
	return R_ACK;
}
