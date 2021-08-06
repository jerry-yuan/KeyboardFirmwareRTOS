#ifndef SYSINFO_H_INCLUDED
#define SYSINFO_H_INCLUDED

#include "task/iotask.h"


typedef struct{
	uint32_t uiDeviceId;
	uint32_t uiRevisionId;
	uint8_t  uiSerial[12];
	uint16_t  uiFlashSize;
} SysInfo_t;

// 0x00     系统基本函数相关
uint8_t IOH_syncHandler(Buffer_t* pstRequest,Buffer_t* pstResponse);
uint8_t IOH_sysInfo(Buffer_t* pstRequest,Buffer_t* pstResponse);
uint8_t IOH_largeEcho(Buffer_t* pstRequest,Buffer_t* pstResponse);
#endif /* SYSINFO_H_INCLUDED */
