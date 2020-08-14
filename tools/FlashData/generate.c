#include <stdio.h>
#include <string.h>
#include <stdlib.h>
FILE* fout;
int offset=0;
void fillConstToFile(int target,char value){
	while(offset<target && offset<=0x800000){
		fputc(value,fout);
		offset++;
	}
}

void clean_stdin(){
	int c;
	do{
		c=getchar();
	}while(c!='\n'&&c!=EOF);
}
int main(){
	char fileLocation[100]={0};
	fout=fopen("./output.bin","wb");
	if(fout==NULL){
		printf("Failed to open output.bin with write mode.\n");
		exit(2);
	}
	while(offset<=0x7FFFFF){
		printf("输入指令[0x%06X]>",offset);
		scanf("%s",fileLocation);
		clean_stdin();
		if(strcmp(fileLocation,"exit")==0){
			// 退出程序
			break;
		}else if(strcmp(fileLocation,"fill")==0){
			//填充至
			int target=0;
			while(target<=offset){
				if(target>0){
					printf("地址无效,请重新输入!\n");
				}
				printf("输入目标地址[0x%06X]>",offset);
				scanf("0x%x",&target);
			}
			fillConstToFile(target,0);
		}else if(strcmp(fileLocation,"file")==0){
			//填充文件
			printf("输入文件路径>");
			scanf("%s",fileLocation);
			clean_stdin();
			FILE* fin=fopen(fileLocation,"rb");
			if(fin==NULL){
				printf("无法打开文件!\n");
				continue;
			}
			int size;
			fseek(fin,0,SEEK_END);
			fgetpos(fin,(fpos_t*)&size);
			fseek(fin,0,SEEK_SET);
			printf("size=%d\n",size);
			int temp=offset;
			while(size-- && offset<0x7FFFFF){
				fputc(fgetc(fin),fout);
				offset++;
				printf("\r正在拷贝数据,已经拷贝%d字节,当前地址[%06X]",offset-temp,offset);
			}
			printf("拷贝完成\n");
			printf("成功拷贝%d字节\n",offset-temp);
			fclose(fin);
		}else{
			printf("无法识别的命令:%s\n",fileLocation);
		}
	}
	printf("写数据结束,填0中...\n");
	fillConstToFile(0x800000,0);
	fclose(fout);
	exit(0);

}
