#include <iohandlers/w25x.h>


// 0x20     W25X 信息读取
uint8_t IOH_w25Info(Buffer_t* pstRequest,Buffer_t* pstReponse){
    return R_ACK;
}
// 0x21     W25X Flash读
uint8_t IOH_w25xRead(Buffer_t* pstRequest,Buffer_t* pstResponse){
    return R_ACK;
}
// 0x22     W25X Flash写
uint8_t IOH_w25xWrite(Buffer_t* pstRequest,Buffer_t* pstResponse){
    return R_ACK;
}
