#include <iohandlers/sysinfo.h>
#include <string.h>
// 0x00 同步函数
uint8_t IOH_syncHandler(Buffer_t* pstRequest,Buffer_t* pstResponse){
	const uint8_t syncResp[]={R_ACK,R_NAK,R_ACK,R_NAK};
	pstResponse->pBuffer=pvPortMalloc(sizeof(syncResp));
	memcpy(pstResponse->pBuffer,syncResp,sizeof(syncResp));
	pstResponse->uiLength = sizeof(syncResp);
	return R_ACK;
}
// 0x01 系统基本函数
uint8_t IOH_sysInfo(Buffer_t* pstRequest,Buffer_t* pstResponse){
	SysInfo_t* pstInfo=pvPortMalloc(sizeof(SysInfo_t));
	// 获取DevId
	pstInfo->uiDeviceId = DBGMCU_GetDEVID();
	// 获取RevId
	pstInfo->uiRevisionId = DBGMCU_GetREVID();
	// 获取96位序列号
	memcpy(&pstInfo->uiSerial,(uint8_t*)0x1FFFF7E8,12);
	// 获取闪存大小
	pstInfo->uiFlashSize = *((uint16_t*)0x1FFFF7E0);

	pstResponse->pBuffer = (uint8_t*)pstInfo;
	pstResponse->uiLength = sizeof(SysInfo_t);

	return R_ACK;
}
// 0x02 大数据量通信
uint8_t IOH_largeEcho(Buffer_t* pstRequest,Buffer_t* pstResponse){
	pstResponse->pBuffer=pstRequest->pBuffer;
	pstResponse->uiLength=pstRequest->uiLength;

	//dumpMemory(pstRequest->pBuffer,pstRequest->uiLength);
	printf("recv:%d\r\n",pstResponse->uiLength);

	pstRequest->pBuffer = NULL;
	pstRequest->uiLength = 0;

	return R_ACK;
}
