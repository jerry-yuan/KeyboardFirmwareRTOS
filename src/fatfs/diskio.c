/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "w25x.h"
/* Definitions of physical drive number for each drive */
#define DEV_W25X		0	/* Example: Map Ramdisk to physical drive 0 */


#define   W25X_SECTOR_SIZE      512
#define   W25X_BLOCK_SIZE       8

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (BYTE pdrv) {
	DSTATUS status = STA_NOINIT;
	switch(pdrv){
	case DEV_W25X:
		status &= ~STA_NOINIT;
		break;
	}
    return status;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv) {
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (BYTE pdrv,BYTE *buff,LBA_t sector,UINT count) {
    DRESULT res=RES_PARERR;

    switch (pdrv) {
    case DEV_W25X :
        // translate the arguments here

        W25X_Read_Data(sector*W25X_SECTOR_SIZE,buff,count*W25X_SECTOR_SIZE);
		//printf("SEC[%06X]+%06X\r\n",sector,count);
        // translate the reslut code here
		res = RES_OK;
        break;
    }

    return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (BYTE pdrv,const BYTE *buff,LBA_t sector,UINT count) {
    DRESULT res;
    int result;

    switch (pdrv) {
    case DEV_W25X:
        // translate the arguments here
        printf("正在擦除0x%06X扇区...\r\n",sector);
        W25X_Erase_Sector(sector*W25X_SECTOR_SIZE);
        printf("正在写入0x%06X+%06X扇区...\r\n",sector,count);
		W25X_Write_Buffer(sector*W25X_SECTOR_SIZE,buff,count*W25X_SECTOR_SIZE);
        return 0;


    }

    return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (BYTE pdrv,BYTE cmd,void *buff) {
    DRESULT status = RES_PARERR;
    W25XJEDECId_t stJEDECId;
    uint32_t bytes;
    switch(cmd) {
    case CTRL_SYNC:
        break;
    case GET_SECTOR_SIZE:
        *(WORD*)buff = W25X_SECTOR_SIZE;
        status=RES_OK;
        break;
    case GET_BLOCK_SIZE:
        *(WORD*)buff = W25X_BLOCK_SIZE;
        status=RES_OK;
        break;
    case GET_SECTOR_COUNT:
    	W25X_Read_JEDECId(&stJEDECId);
    	if(stJEDECId.uiManufacture == 0xEF){
			*(WORD*)buff = (1<<(stJEDECId.uiFlashId & 0x0F)+16)/W25X_SECTOR_SIZE;
	    	status=RES_OK;
    	}
        break;
    }
    return status;
}
DWORD get_fattime(){
	return 0;
}
