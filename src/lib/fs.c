#include "lib/fs.h"

#include "stdio.h"

void FS_Initialize(){
	FRESULT eResult;
	FATFS* pstFlash=pvPortMalloc(sizeof(FATFS));
	DIR dir;
	FILINFO info;

    eResult=f_mount(pstFlash,"0",1);

	if(eResult != FR_OK){
		printf("Mount Filesystem Failed:%d\r\n",eResult);
		return;
	}
	eResult=f_opendir(&dir,"0:");
	if(eResult!=FR_OK){
		printf("Open DIr failed %d\r\n",eResult);
		return;
	}

	while(f_readdir(&dir,&info)==FR_OK){
		if(info.fname[0]=='\0')
			break;
		printf("%s\r\n",info.fname);
	}
}
