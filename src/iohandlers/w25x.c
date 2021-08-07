#include <iohandlers/w25x.h>
#include <bsp/w25x.h>
typedef struct {
    uint32_t uiAddr;
    uint16_t uiLength;
}W25XRW_t;

// 0x20     W25X 信息读取
uint8_t IOH_w25xInfo(Buffer_t* pstRequest,Buffer_t* pstResponse){
    W25XJEDECId_t* pstJEDECId=pvPortMalloc(sizeof(W25XJEDECId_t));

    W25X_Read_JEDECId(pstJEDECId);

    pstResponse->pBuffer = (uint8_t*)pstJEDECId;
    pstResponse->uiLength = sizeof(W25XJEDECId_t);

    return R_ACK;
}
// 0x21     W25X Flash读
uint8_t IOH_w25xRead(Buffer_t* pstRequest,Buffer_t* pstResponse){
    W25XRW_t* pstRWReq = (W25XRW_t*)pstRequest->pBuffer;

    pstResponse->pBuffer = pvPortMalloc(pstRWReq->uiLength);
    pstResponse->uiLength = pstRWReq->uiLength;

    W25X_Read_Data(pstRWReq->uiAddr,pstResponse->pBuffer,pstRWReq->uiLength);

    return R_ACK;
}
// 0x22     W25X Flash写
uint8_t IOH_w25xWrite(Buffer_t* pstRequest,Buffer_t* pstResponse){
    W25XRW_t* pstRWReq = (W25XRW_t*)pstRequest->pBuffer;

    uint8_t* bufferWrite = pstRequest->pBuffer+sizeof(W25XRW_t);

    W25X_Write_Buffer(pstRWReq->uiAddr,bufferWrite,pstRWReq->uiLength);

    return R_ACK;
}
