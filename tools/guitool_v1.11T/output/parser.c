#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef struct gui_font_header {
    uint8_t magic[4];      // 文件头
    uint32_t dwFileSize;   // 文件大小
    uint8_t nSection;      // 段信息数量
    uint8_t YSize;         // 字体高度
    uint16_t wCpFlag;      // codepageflag:  每一位表示一个codepage
    uint16_t nTotalChars;  // 字符总数
    char reserved[2];      // 保留字节
} GUI_FONT_HEADER;
typedef struct gui_font_section {
    uint16_t first;       // 段起始码
    uint16_t last;        // 段结束码
    uint32_t offsetAddr;  // GUI_FONT_INDEX首字节的偏移地址
} GUI_FONT_SECTION;

typedef struct gui_font_index {
    uint32_t width;       // 宽度
    uint32_t offsetAddr;  // 点阵数据起始偏移地址
} GUI_FONT_INDEX;

FILE* fp;
void dumpHeader(GUI_FONT_HEADER* fileHeader) {
    printf("=========文件信息=========\n");
    printf("文件类型:%c%c%c\n", fileHeader->magic[0], fileHeader->magic[1], fileHeader->magic[2]);
    printf("文件版本:%d.%d\n", (fileHeader->magic[3] & 0xF0) >> 4, fileHeader->magic[3] & 0x0f);
    printf("文件大小:%d Bytes\n", fileHeader->dwFileSize);
    printf("段信息数量:%d\n", fileHeader->nSection);
    printf("字体高度:%d\n", fileHeader->YSize);
    printf("CodePage标志位:0x%04x\n", fileHeader->wCpFlag);
    printf("字符数量:%d\n", fileHeader->nTotalChars);
}
void getFontIndex(GUI_FONT_HEADER* header, GUI_FONT_INDEX* index, uint16_t charCode) {
    GUI_FONT_SECTION section;
    // 回到文件头结束
    fseek(fp, sizeof(GUI_FONT_HEADER), SEEK_SET);
    // 检查属于哪个段
    for (uint8_t i = 0; i < header->nSection; i++) {
        fread(&section, sizeof(section), 1, fp);
        if (section.first <= charCode && charCode <= section.last) {
            break;
        }
    }
    uint32_t info = 0;
    fseek(fp, section.offsetAddr + (charCode - section.first) * sizeof(info), SEEK_SET);
    printf("indexAddr=0x%08lx\n",ftell(fp));
    fread(&info, sizeof(info), 1, fp);
    printf("rawIndex=0x%08lx\n",info);
    index->width = (info & 0xFC000000) >> 26;
    index->offsetAddr = info & 0x03FFFFFF;
    //fread(index, sizeof(GUI_FONT_INDEX), 1, fp);
}
uint8_t* nextUnicode(uint8_t* pStr, uint16_t* unicode) {
    uint8_t temp, bytesPerChar;
    if ((*pStr & 0x80) == 0x00) {
        // 单字节字符:英文字符
        *unicode = *pStr++;
    } else {
        // 多字节字符
        // 取出第一个字节
        temp = *pStr;
        bytesPerChar = 0;
        *unicode = 0x00000000;
        while (temp & 0x80) {
            bytesPerChar++;
            temp <<= 1;
        }
        *unicode = (*pStr++) & ((1 << (8 - bytesPerChar)) - 1);
        for (uint8_t i = 1; i < bytesPerChar; i++) {
            *unicode <<= 6;
            *unicode |= ((*pStr++) & 0x3F);
        }
    }
    return pStr;
}

void main() {
    fp = fopen("./Deng_U12.bin", "rb");
    if(fp==NULL){
        printf("cannot open ./Deng_U12.bin");
        exit(-1);
    }
    GUI_FONT_HEADER fileHeader;
    GUI_FONT_INDEX fontIndex;
    uint8_t buffer[1024] = {0};
    char* pBuffer = buffer;
    fread(&fileHeader, sizeof(GUI_FONT_HEADER), 1, fp);
    dumpHeader(&fileHeader);
    printf("输入一个UTF-8串>");
    scanf("%s", buffer);
    uint16_t unicode;
    while (*pBuffer != '\0') {
        pBuffer = nextUnicode(pBuffer, &unicode);
        printf("当前字符Unicode=0x%08x\n", unicode);
        getFontIndex(&fileHeader, &fontIndex, unicode);
        printf("字体宽度:%d\n", fontIndex.width);
        printf("offsetAddr=0x%08x\n", fontIndex.offsetAddr);

        fseek(fp, fontIndex.offsetAddr, SEEK_SET);

        uint32_t charDataSize = fileHeader.YSize * ((fontIndex.width + 8 - 1) / 8);
        printf("from 0x%08x read %d (0x%04x) bytes\r\n", fontIndex.offsetAddr, charDataSize, charDataSize);
        uint8_t* charData = malloc(charDataSize);

        fread(charData, 1, charDataSize, fp);
        uint8_t mask = 0x80;
        uint8_t* pChar = charData;
        for (int y = 0; y < fileHeader.YSize; y++) {
            mask = 0x80;
            for (int x = 0; x < (fontIndex.width + 8 - 1) / 8 * 8; x++) {
                if (mask == 0x00) {
                    mask = 0x80;
                    pChar++;
                }
                if (*pChar & mask) {
                    printf("x");
                } else {
                    printf("_");
                }
                mask >>= 1;
            }
            printf("\n");
            pChar++;
        }
    }
}
