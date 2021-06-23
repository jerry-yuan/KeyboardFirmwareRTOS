#include <task/iotask.h>
#include <task/priorities.h>
#include <lib/utils.h>
#include <USB/usb.h>
#include <usb_mem.h>
#include <usb_regs.h>
#include <usb_conf.h>
#include <string.h>

#define R_ACK  0x55
#define R_NAK  0xAA

typedef struct{
	uint8_t* pBuffer;
	uint16_t uiLength;
} Buffer_t;
typedef struct{
	uint32_t uiDeviceId;
	uint32_t uiRevisionId;
	uint8_t  uiSerial[12];
	uint16_t  uiFlashSize;
} SysInfo_t;
//typedef SGUI_DEVPF_IF_DEFINE(R, FN, PARAM)
typedef uint8_t (*RequestHandler_t)(Buffer_t* pstRequest,Buffer_t* pstResponse);
// 0x00
static uint8_t syncHandler(Buffer_t* pstRequest,Buffer_t* pstResponse);
static uint8_t sysInfo(Buffer_t* pstRequest,Buffer_t* pstResponse);
// 0x10
static uint8_t rtcGetCounter(Buffer_t* pstRequest,Buffer_t* pstResponse);
static uint8_t rtcSetCounter(Buffer_t* pstRequest,Buffer_t* pstResponse);
// 0x20
static uint8_t largeEcho(Buffer_t* pstRequest,Buffer_t* pstResponse);

const RequestHandler_t requestHandlers[256]={
			 /*    0x00      0x01			 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F*/
	/* 0x00 */ syncHandler	,sysInfo		,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	/* 0x10 */ rtcGetCounter,rtcSetCounter	,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	/* 0x20 */ largeEcho,
	NULL
};

TaskHandle_t hIOTask;
extern SemaphoreHandle_t hEP3TxWait;
static uint8_t syncHandler(Buffer_t* pstRequest,Buffer_t* pstResponse){
	const uint8_t syncResp[]={R_ACK,R_NAK,R_ACK,R_NAK};
	pstResponse->pBuffer=pvPortMalloc(sizeof(syncResp));
	memcpy(pstResponse->pBuffer,syncResp,sizeof(syncResp));
	pstResponse->uiLength = sizeof(syncResp);
	return R_ACK;
}
static uint8_t sysInfo(Buffer_t* pstRequest,Buffer_t* pstResponse){
	SysInfo_t* pstInfo=pvPortMalloc(sizeof(SysInfo_t));
	// 获取DevId
	pstInfo->uiDeviceId = DBGMCU_GetDEVID();
	// 获取RevId
	pstInfo->uiRevisionId = DBGMCU_GetREVID();
	// 获取96位序列号
	memcpy(&pstInfo->uiSerial,(uint8_t*)0x1FFFF7E8,12);
	// 获取闪存大小
	pstInfo->uiFlashSize = *((uint16_t*)0x1FFFF7E0);

	pstResponse->pBuffer = pstInfo;
	pstResponse->uiLength = sizeof(SysInfo_t);

	return R_ACK;
}
static uint8_t rtcGetCounter(Buffer_t* pstRequest,Buffer_t* pstResponse){
	pstResponse->pBuffer=pvPortMalloc(sizeof(uint32_t));
	*((uint32_t*)pstResponse->pBuffer) = RTC_GetCounter();
	pstResponse->uiLength = sizeof(uint32_t);
	return R_ACK;
}
static uint8_t rtcSetCounter(Buffer_t* pstRequest,Buffer_t* pstResponse){
	uint32_t* pTimestamp=(uint32_t*)pstRequest->pBuffer;
	RTC_SetCounter(*pTimestamp);
	return R_ACK;
}
static uint8_t largeEcho(Buffer_t* pstRequest,Buffer_t* pstResponse){
	pstResponse->pBuffer=pstRequest->pBuffer;
	pstResponse->uiLength=pstRequest->uiLength;

	//dumpMemory(pstRequest->pBuffer,pstRequest->uiLength);
	printf("recv:%d\r\n",pstResponse->uiLength);

	pstRequest->pBuffer = NULL;
	pstRequest->uiLength = 0;

	return R_ACK;
}
static void safeSend(uint8_t uiStatusCode,uint8_t* pBuffer,uint16_t uiLength){
	uint8_t uiPackageSize=0;
	uint8_t uiDataLength=0;
	uint8_t packageHeader[4]={0};
	// 发送第一包数据:发送状态码和长度
	while(GetEPTxStatus(ENDP3)!=EP_TX_NAK){
		//xSemaphoreTake(hEP3TxWait,portMAX_DELAY);
	}
	// 构建第一包包头
	packageHeader[0] = uiStatusCode;
	*((uint16_t*)(packageHeader+1))=uiLength;
	if(uiLength>0){
		// 写入数据以2字节为一组,多出来的一个空位不浪费填上数据.
		packageHeader[3] = *pBuffer;
		uiLength -= 1;
		pBuffer += 1;
		uiPackageSize += 1;
	}
	UserToPMABufferCopy(packageHeader,GetEPTxAddr(ENDP3),4);
	// 凑齐第一包
	if(uiLength>0){
		uiDataLength = MIN(60,uiLength);
		UserToPMABufferCopy(pBuffer,GetEPTxAddr(ENDP3)+4,uiDataLength);
		uiLength -= uiDataLength;
		pBuffer  += uiDataLength;
		uiPackageSize += uiDataLength;
	}
	// 加上头
	uiPackageSize += 3;
	SetEPTxCount(ENDP3,uiPackageSize);
	SetEPTxValid(ENDP3);
	// 发送剩余的数据
	while(uiLength>0){
		while(GetEPTxStatus(ENDP3)!=EP_TX_NAK){
			//xSemaphoreTake(hEP3TxWait,portMAX_DELAY);
		}
		uiPackageSize = MIN(64,uiLength);
		UserToPMABufferCopy(pBuffer,GetEPTxAddr(ENDP3),uiPackageSize);
		SetEPTxCount(ENDP3,uiPackageSize);
		SetEPTxValid(ENDP3);
		uiLength -= uiPackageSize;
		pBuffer  += uiPackageSize;
	}
	// 如果最后一个包是64字节要补发个空包
	if(uiPackageSize==64){
		while(GetEPTxStatus(ENDP3)!=EP_TX_NAK){
			//xSemaphoreTake(hEP3TxWait,portMAX_DELAY);
		}
		SetEPTxCount(ENDP3,0);
		SetEPTxValid(ENDP3);
	}
}
static void ioTask(void){
	uint8_t uiCommand=0;
	uint8_t uiStatusCode=R_NAK;
	Buffer_t stRequest,stResponse;
	stRequest.pBuffer = stResponse.pBuffer=NULL;
	while(1){
		stRequest.uiLength = stResponse.uiLength=0;
		// 读取命令ID
		uiCommand=FIFO_ReadByte(pUSBFIFO);
		// 读取参数区长度
		FIFO_SafeRead(pUSBFIFO,(uint8_t*)&stRequest.uiLength,sizeof(uint16_t));
		// 读取参数区
		if(stRequest.uiLength>0){
			stRequest.pBuffer = pvPortMalloc(stRequest.uiLength);
			FIFO_SafeRead(pUSBFIFO,stRequest.pBuffer,stRequest.uiLength);
		}
		// 处理请求
		if(requestHandlers[uiCommand]){
			uiStatusCode=requestHandlers[uiCommand](&stRequest,&stResponse);
		}else{
			uiStatusCode=R_NAK;
			printf("[IO]Undefined Command:%02X\n",uiCommand);
		}
		// 发送响应
		safeSend(uiStatusCode,stResponse.pBuffer,stResponse.uiLength);
		// 清理现场
		if(stRequest.pBuffer){
			vPortFree(stRequest.pBuffer);
			stRequest.pBuffer  = NULL;
		}
		if(stResponse.pBuffer){
			vPortFree(stResponse.pBuffer);
			stResponse.pBuffer = NULL;
		}
	}

}
void IOTaskInitialize(){
	BaseType_t xResult=pdFALSE;
	xResult = xTaskCreate((TaskFunction_t)ioTask,"io",512,NULL,TASK_IO_PRIORITY,&hIOTask);
	if(xResult==pdTRUE){
		printf("create ioTask success!\n");
	}
}
