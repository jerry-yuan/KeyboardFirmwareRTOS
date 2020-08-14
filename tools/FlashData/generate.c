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
		printf("����ָ��[0x%06X]>",offset);
		scanf("%s",fileLocation);
		clean_stdin();
		if(strcmp(fileLocation,"exit")==0){
			// �˳�����
			break;
		}else if(strcmp(fileLocation,"fill")==0){
			//�����
			int target=0;
			while(target<=offset){
				if(target>0){
					printf("��ַ��Ч,����������!\n");
				}
				printf("����Ŀ���ַ[0x%06X]>",offset);
				scanf("0x%x",&target);
			}
			fillConstToFile(target,0);
		}else if(strcmp(fileLocation,"file")==0){
			//����ļ�
			printf("�����ļ�·��>");
			scanf("%s",fileLocation);
			clean_stdin();
			FILE* fin=fopen(fileLocation,"rb");
			if(fin==NULL){
				printf("�޷����ļ�!\n");
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
				printf("\r���ڿ�������,�Ѿ�����%d�ֽ�,��ǰ��ַ[%06X]",offset-temp,offset);
			}
			printf("�������\n");
			printf("�ɹ�����%d�ֽ�\n",offset-temp);
			fclose(fin);
		}else{
			printf("�޷�ʶ�������:%s\n",fileLocation);
		}
	}
	printf("д���ݽ���,��0��...\n");
	fillConstToFile(0x800000,0);
	fclose(fout);
	exit(0);

}
