#ifndef W25Q64_H_INCLUDED
#define W25Q64_H_INCLUDED

/*********************************Copyright (c)*********************************
**
**                                 FIVE工作组
**
**---------------------------------File Info------------------------------------
** File Name:               w25q64.h
** Last modified Date:      2013/9/10 9:32:33
** Last Version:            V1.2
** Description:             none
**
**------------------------------------------------------------------------------
** Created By:              wanxuncpx
** Created date:            2013/8/6 21:12:35
** Version:                 V1.2
** Descriptions:            none
**------------------------------------------------------------------------------
** HW_CMU:                  STM32F103ZET6
** Libraries:               STM32F10x_StdPeriph_Driver
** version                  V3.5
*******************************************************************************/

/******************************************************************************
更新说明:
******************************************************************************/


/******************************************************************************
*********************************  应 用 资 料 ********************************
*******************************************************************************
    1. W25Q64 拥有32 768个页,每页256个字节故有8MB的容量
    2. W25Q64 16个页为一个扇区
    3. W25Q64 256个页为一个Block
    2. 指令表
┌------------------------┬--------┬-----------┬-----------┬-----------┬-----------┬-----------┐
|       令名             |  BYTE1  |   BYTE2    |    BYTE3   |   BYTE4    |   BYTE5    |   BYTE6    |
|                        | (CODE)  |            |            |            |            |            |
├------------------------┼--------┼-----------┼-----------┼-----------┼-----------┼-----------┤
| Write Enable           |   06h   |            |            |            |            |            |
| 允许写状态寄存器      |   50h   |            |            |            |            |            |
| Write Disable          |   04h   |            |            |            |            |            |
| Read Status Register-1 |   05h   | S7–S0     |            |            |            |            |
| Read Status Register-2 |   35h   | S15–S8    |            |            |            |            |
| Write Status Register  |   01h   | S7–S0     | S15-S8     |            |            |            |
| Page Program           |   02h   | A23–A16   | A15–A8    |  A7–A0    |D7–D0      |            |
| Quad Page Program      |   32h   | A23–A16   | A15–A8    |  A7–A0    |D7–D0,..(3)|            |
| Sector Erase (4KB)     |   20h   | A23–A16   | A15–A8    |  A7–A0    |            |            |
| Block Erase (32KB)     |   52h   | A23–A16   | A15–A8    |  A7–A0    |            |            |
| Block Erase (64KB)     |   D8h   | A23–A16   | A15–A8    |  A7–A0    |            |            |
| Chip Erase             | C7h/60h |            |            |            |            |            |
| Erase / Program Suspend|   75h   |            |            |            |            |            |
| Erase / Program Resume |   7Ah   |            |            |            |            |            |
| Power-down             |   B9h   |            |            |            |            |            |
| 复位连续读模式        |   FFh   |    FFh     |            |            |            |            |
├------------------------┼--------┼-----------┼-----------┼-----------┼-----------┼-----------┤
| Read Data              |   03h   |  A23-A16   | A15-A8     |  A7-A0     |  (D7-D0)   |            |
| Fast Read              |   0Bh   |  A23-A16   | A15-A8     |  A7-A0     |  dummy     |(D7-D0)     |
| Fast Read Dual Output  |   3Bh   |  A23-A16   | A15-A8     |  A7-A0     |  dummy     |(D7-D0, …) |
| Fast Read Quad Output  |   6Bh   |  A23-A16   | A15-A8     |  A7-A0     |  dummy     |(D7-D0, …) |
| Fast Read Dual I/O     |   BBh   |  A23-A8(2) | A7-A0,     |  M7-M0(2)  | (D7-D0,..) |            |
| Fast Read Quad I/O     |   EBh   |A23-A0,M7-M0|xxxx,D7-D0,.|(D7-D0,..)  |            |            |
| Word Read Quad I/O     |   E7h   |A23-A0,M7-M0|xx,D7-D0,.  |(D7-D0,..)  |            |            |
|Octal Word Read Quad I/O|   E3h   |A23-A0,M7-M0|(D7-D0,.)   |            |            |            |
| Set Burst with Wrap    |     77h |xxxxxx,W6-W4|            |            |            |            |
├------------------------┼--------┼-----------┼-----------┼-----------┼-----------┼-----------┤
|Release Device ID       |  ABh    | dummy      |  dummy     |  dummy     |  (ID7-ID0) |            |
|ID                      |  90h    | dummy      |  dummy     |  00h       |  (MF7-MF0) | (ID7-ID0)  |
|ID by Dual I/O          |  92h    | A23-A8     |A7-A0,M[7:0]|MF[7:0],ID[7:0] |        |            |
|ID  by Quad I/O         |  94h    | A23-A0,M[7:0]| xxxx,(MF[7:0],ID[7:0])|(MF[7:0], ID[7:0], …)| ||
|JEDEC ID                |  9Fh    | (MF7-MF0)  | (ID15-ID8) | (ID7-ID0)  |            |            |
|Read Unique ID          |  4Bh    | dummy      |  dummy     |  dummy     |  dummy     | (ID63-ID0) |
|Erase Security Registers|  44h    | A23–A16   |  A15–A8   |  A7–A0    |            |            |
|Program Security Registers|42h    | A23–A16   |  A15–A8   |  A7–A0    |  D7-D0     | D7-D0      |
|Read Security Registers |  48h    | A23–A16   |  A15–A8   |  A7–A0    |  dummy     | (D7-0)     |
└------------------------┴--------┴-----------┴-----------┴-----------┴-----------┴-----------┘
******************************************************************************/

#ifndef _W25Q64_H_
#define _W25Q64_H_

/******************************************************************************
********************************* 文件引用部分 ********************************
******************************************************************************/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_dma.h"

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include <bsp/flashMap.h>
/******************************************************************************
******************************* 系 统 参 数 定 义******************************
******************************************************************************/
/*---------------------*
*     USART优先级定义
*----------------------*/
#define W25X_DMA_TC_PRIO    2           /* 中断优先级          */

/*---------------------*
*      参数配置定义
*----------------------*/
#define W25X_PAGE_SIZE      256         /* 定义页大小          */
#define W25X_PAGE_NUM       32768       /* 定义页的个数        */
#define W25X_SECTOR_SIZE    4096        /* 每扇区的大小        */
#define W25X_PAGES_PS       (W25X_SECTOR_SIZE/W25X_PAGE_SIZE)  /* 每扇区的页数 */

#define W25X_BUFF_NUM       2           /* 定义SRAM中缓冲个数  */
#define W25X_DUMMY_BYTE     0xFF        /* 空读字节定义        */

/******************************************************************************
********************************** 需自定义 ***********************************
****************************** 参数配置、引脚定义  ****************************
******************************************************************************/

/*---------------------*
*     本地硬件连接
*----------------------*/


/*---------------------*
*     常用指令定义
*----------------------*/
#define W25X_ReadStatusReg      0x05    //读状态寄存器1
#define W25X_ReadStatusReg2     0x35    //读状态寄存器2
#define W25X_WriteStatusReg     0x01    //写状态寄存器1
#define W25X_ReadUniqueID       0x4B    //读取唯一ID

#define W25X_WriteEnable        0x06    //写使能
#define W25X_WriteDisable       0x04    //写关闭

#define W25X_ReadData           0x03    //读数据
#define W25X_PageProgram        0x02    //写FLASH

#define W25X_ChipErase          0x60    //也可为0x60
#define W25X_SectorErase        0x20    //4KB擦除


#define W25X_PowerDown          0xB9    //掉电,低功耗
#define W25X_ReleasePowerDown   0xAB    //恢复上电

/******************************************************************************
********************************* 参数宏定义 *********************************
******************************************************************************/
/*---------------------*
*       参数定义
*----------------------*/
//#define W25X_WP_DIS()        (GPIOB->BRR = GPIO_Pin_0)
//#define W25X_WP_EN()         (GPIOB->BSRR  = GPIO_Pin_0)
#define W25X_CS_L()          (GPIOA->BRR  = GPIO_Pin_15)
#define W25X_CS_H()          (GPIOA->BSRR = GPIO_Pin_15)
/******************************************************************************
********************************* 数 据 声 明 *********************************
******************************************************************************/
/*---------------------*
*      输出数据
*----------------------*/
extern uint8_t* W25X_Buffer;//[W25X_SECTOR_SIZE];
extern volatile bool sem_W25X_DMA_Busy;         //用户只读
extern volatile bool sem_W25X_DMA_RxRdy;        //用户读取,清零

/******************************************************************************
********************************* 函 数 声 明 *********************************
******************************************************************************/
/*---------------------*
*    输出函数
*----------------------*/
//初始化
extern void FLASH_Initialize();
extern void W25X_GPIO_Config(void);         //配置GPIO口
extern void W25X_Init(void);                //初始化SPI

//获取状态
extern uint8_t W25X_ReadSR(void);           //读取状态寄存器
extern uint16_t W25X_ReadID(void);          //读取ID号
extern void W25X_Wait_Busy(void);           //等待W25X直到空闲
extern bool W25X_Read_BusyState(void);      //读取W25X的忙状态(不等待)

//控制状态
extern void W25X_Write_Enable(void);        //写使能
extern void W25X_Write_Disable(void);       //写禁止
extern void SPI_Flash_PowerDown(void);      //掉电
extern void SPI_Flash_WakeUp(void);         //唤醒

//擦除相关(有等待选择)
extern void W25X_Erase_Chip(bool bwait);    //全片擦除,要等待写完成,需要21秒
extern void W25X_Erase_Sector(uint32_t Dst_Addr,bool bwait);    //扇区擦除,实际需要65ms

extern void W25X_Read_Data(uint8_t * pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);     //250us执行完毕
extern void W25X_DMARead_Data(uint8_t * pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);  //5us执行,75us后结束
extern void W25X_Read_Page(uint8_t * pBuffer,uint32_t PageAddr);    //读出一页,300us
extern void W25X_Write_Page(uint8_t * pBuffer,uint32_t PageAddr);   //写入一页,必先擦除,事先有些等待,600us
extern void W25X_Read_UID(uint8_t * pBuffer);   //读取W25X的唯一MAC,<5us

/******************************************************************************
***********************************   END  ************************************
******************************************************************************/
#endif

#endif /* W25Q64_H_INCLUDED */

