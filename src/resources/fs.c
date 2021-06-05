#include <resources/fs.h>
#include <bsp/W25X.h>
#include <FreeRTOS.h>
#include <string.h>
lfs_t* pstW25Xfs=NULL;

static int w25x_read_block(const struct lfs_config* config,lfs_block_t block,lfs_off_t off,void* buffer,lfs_size_t size){
	W25X_Read_Data(block*config->block_size+off,buffer,size);
	return 0;
}
static int w25x_prog_block(const struct lfs_config* config,lfs_block_t block,lfs_off_t off,const void* buffer,lfs_size_t size){
	W25X_Write_Buffer(block*config->block_size+off,buffer,size);
	return 0;
}
static int w25x_erase_block(const struct lfs_config* config,lfs_block_t block){
	W25X_Erase_Sector(block*config->block_size);
	return 0;
}
static int w25x_sync_block(const struct lfs_config* config){
	return LFS_ERR_OK;
}


void travel_dir(const char* path,const char* name,uint16_t level){
	struct lfs_dir cDir;
	struct lfs_info cInfo;
	char nextPath[50]={0};
	lfs_dir_open(pstW25Xfs,&cDir,path);
	// 打印当前目录
	for(uint16_t i=1;i<level;i++){
		printf("| ");
	}
	if(level>0){
		printf("|-");
	}
	printf("%s\n",name);
	while(lfs_dir_read(pstW25Xfs,&cDir,&cInfo)){
		if(strcmp(".",cInfo.name)==0 || strcmp("..",cInfo.name)==0){
			continue;
		}
		if(cInfo.type==LFS_TYPE_REG){
			for(uint16_t i=0;i<level;i++){
				printf("| ");
			}
			printf("|-%s\t%ld\n",cInfo.name,cInfo.size);
		}else if(cInfo.type==LFS_TYPE_DIR){
			sprintf(nextPath,"%s/%s",path,cInfo.name);
			travel_dir(nextPath,cInfo.name,level+1);
		}
	}
	lfs_dir_close(pstW25Xfs,&cDir);
}

void FS_Initialize(){
	struct lfs_config* pstW25XCfg;
	W25XJEDECId_t stJEDEC;
	int uiLfsResult;
	// 分配空间
	pstW25Xfs  = pvPortMalloc(sizeof(lfs_t));
	pstW25XCfg = pvPortMalloc(sizeof(struct lfs_config));
	// 初始化数据
	pstW25XCfg->read  = w25x_read_block;
	pstW25XCfg->prog  = w25x_prog_block;
	pstW25XCfg->erase = w25x_erase_block;
	pstW25XCfg->sync  = w25x_sync_block;
	// 获取Flash型号
	W25X_Read_JEDECId(&stJEDEC);
	pstW25XCfg->read_size      = 256;		// block_size
	pstW25XCfg->prog_size      = 256;		// block_size
	pstW25XCfg->block_size     = 4096;		//
	pstW25XCfg->block_count    = (1<<(stJEDEC.uiFlashId&0x7F))/pstW25XCfg->block_size;
	pstW25XCfg->cache_size     = 4096;		// block_size
	pstW25XCfg->lookahead_size = 256;		// 8192
	pstW25XCfg->block_cycles   = 512;		// 512

	// 挂载文件系统
	uiLfsResult = lfs_mount(pstW25Xfs,pstW25XCfg);
	if(uiLfsResult){
		printf("No LittleFS on Flash, ERASING ALL CHIP and try to format as LittleFS...\n");
		exit(0);
		W25X_Erase_Chip();
		lfs_format(pstW25Xfs,pstW25XCfg);
		lfs_mount(pstW25Xfs,pstW25XCfg);
	}
	printf("LittleFS Initialized.\n");
	// 输出所有文件
	travel_dir("/","/",0);
}


