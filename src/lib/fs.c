#include "lib/fs.h"

void FS_Initialize(){
	FRESULT eResult;
	FATFS* pstFlash=pvPortMalloc(sizeof(FATFS));


    eResult=f_mount(pstFlash,"0",1);

	if(eResult != FR_OK){
		printf("Mount Filesystem Failed:%d\r\n",eResult);
		return;
	}
}
