#include <task/iotask.h>
#include <task/priorities.h>
#include <lib/utils.h>
#include <iohandlers/sysinfo.h>
#include <iohandlers/rtc.h>
#include <iohandlers/w25x.h>
#include <USB/usb.h>
#include <usb_mem.h>
#include <usb_regs.h>
#include <usb_conf.h>

// 0x00     系统基本函数

const RequestHandler_t requestHandlers[256]={
			 /*0x00              0x01			     0x02            0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F*/
	/* 0x00 */ IOH_syncHandler	,IOH_sysInfo		,IOH_largeEcho  ,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	/* 0x10 */ IOH_rtcGetCounter,IOH_rtcSetCounter	,NULL           ,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	/* 0x20 */ IOH_w25xRead     ,IOH_w25xWrite      ,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
	NULL
};

TaskHandle_t hIOTask;
extern SemaphoreHandle_t hEP3TxWait;

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
