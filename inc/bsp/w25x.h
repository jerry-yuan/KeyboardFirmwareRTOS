#ifndef W25X_H_INCLUDED
#define W25X_H_INCLUDED

#include <stm32f10x.h>

#define W25X_MAX_PAGE					0x100
#define W25X_SECTOR_SIZE				0x1000

#define W25X_CMD_WRITE_ENABLE			0x06
#define W25X_CMD_WRITE_DISABLE			0x04
#define W25X_CMD_READ_STATUS			0x05
#define W25X_CMD_WRITE_STATUS			0x01
#define W25X_CMD_READ_DATA				0x03
#define W25X_CMD_FAST_READ				0x0B
#define W25X_CMD_FAST_READ_D			0x3B
#define W25X_CMD_PAGE_PROGRAM			0x02
#define W25X_CMD_ERASE_BLOCK			0xD8
#define W25X_CMD_ERASE_SECTOR			0x20
#define W25X_CMD_ERASE_CHIP				0xC7
#define W25X_CMD_POWER_DOWN				0xB9
#define W25X_CMD_POWER_UP_ID			0xAB
#define W25X_CMD_READ_MANUFACTURE_ID	0x90
#define W25X_CMD_READ_JEDEC_ID			0x9F

#define W25X_DUMMY_BYTE					0xFF

#define W25X_STATUS_MASK_SRP			0x01<<7
#define W25X_STATUS_MASK_TB				0x01<<5
#define W25X_STATUS_MASK_BP2			0x01<<4
#define W25X_STATUS_MASK_BP1			0x01<<3
#define W25X_STATUS_MASK_BP0			0x01<<2
#define W25X_STATUS_MASK_WEL			0x01<<1
#define W25X_STATUS_MASK_BUSY			0x01<<0

typedef uint8_t W25XManufactureId_t;
typedef uint16_t W25XFlashId_t;
#pragma pack(1)
typedef struct{
	W25XManufactureId_t uiManufacture;
	W25XFlashId_t		uiFlashId;
} W25XJEDECId_t;
#pragma pack()
typedef uint8_t W25XStatus_t;

void W25X_Initialize();
void W25X_Set_WriteState(FunctionalState writeState);
void W25X_Read_Status(W25XStatus_t* puiStatus);
void W25X_Write_Status(W25XStatus_t puiStatus);
void W25X_Read_Data(uint32_t uiAddress,void* pBuffer,uint32_t length);
void W25X_Read_Data_Fast(uint32_t uiAddress,void* pBuffer,uint32_t length);
void W25X_Write_Page(uint32_t uiAddress,void* pBuffer,uint32_t length);
void W25X_Erase_Block(uint32_t uiAddress);
void W25X_Erase_Sector(uint32_t uiAddress);
void W25X_Erase_Chip();
void W25X_Read_JEDECId(W25XJEDECId_t* pstJEDECId);


#endif /* W25X_H_INCLUDED */
