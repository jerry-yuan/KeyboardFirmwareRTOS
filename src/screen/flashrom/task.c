#include <screen/flashrom/task.h>
#include <screen/consts.h>
#include <lib/SerProg.h>
#include <lib/FIFOBuffer.h>
#include <lib/utils.h>
#include <bsp/usart.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <string.h>

#include <task/gui.h>

#define BUFFER_SIZE	2000
#define bprintf(FORMAT,...) pSaveChar+=sprintf(pSaveChar,FORMAT,__VA_ARGS__)

extern FIFO_t* pRxFIFO;

void W25X_SendBuffer(const void* pBuffer,uint16_t length);
void W25X_ReceiveBuffer(void* buffer,uint16_t length);

void flashRomTask(void) {
    uint32_t	slen;     /* SPIOP write len */
    uint32_t	rlen;     /* SPIOP read len  */
    uint16_t 	tlen;     /* SPIOP temp len */
    uint32_t	freq_req;

    uint8_t		buffer[BUFFER_SIZE]= {0};
    FLASHROM_LOCK_EVENT* pstLockEvent = NULL;
    FLASHROM_PBAR_SETUP_EVENT* pstSetupEvent = NULL;
    FLASHROM_PBAR_UPDATE_EVENT* pstUpdateEvent = NULL;
	BaseType_t	xResult = pdFALSE;
    // 清空USART缓冲区
    FIFO_Clear(pRxFIFO);
    while(1) {
        switch(FIFO_ReadByte(pRxFIFO)) {
        case S_CMD_NOP:				// 0x00
            // NOP
            buffer[0] = S_ACK;
            USART_SendBuffer(buffer,1);
            break;
        case S_CMD_Q_IFACE:			// 0x01
            // Query programmer iface version
            buffer[0] = S_ACK;
            // 16bit version (nonzero)
            buffer[1] = S_IFACE_VERSION;
            buffer[2] = 0;
            USART_SendBuffer(buffer,3);
            break;
        case S_CMD_Q_CMDMAP:		// 0x02
            // Query supported commands bitmap
            memset(buffer,0,33);
            // ACK
            buffer[0] = S_ACK;
            // 32 bytes (256 bits) of supported cmds flags(little endian)
            buffer[1] = (S_CMD_MAP & 0x000000FF)>>0;
            buffer[2] = (S_CMD_MAP & 0x0000FF00)>>8;
            buffer[3] = (S_CMD_MAP & 0x00FF0000)>>16;
            buffer[4] = (S_CMD_MAP & 0xFF000000)>>24;

            USART_SendBuffer(buffer,33);
            break;
        case S_CMD_Q_PGMNAME:		// 0x03
            // Query programmer name
            memset(buffer,0,17);
            // ACK
            buffer[0] = S_ACK;
            // (max) 16 bytes string (null padding)
            memcpy(buffer+1,S_PGM_NAME,sizeof(S_PGM_NAME));

            USART_SendBuffer(buffer,17);
            break;
        case S_CMD_Q_SERBUF:		// 0x04
            // Query serial buffer size
            // ACK
            buffer[0] = S_ACK;
            // 16bit size
            buffer[1] = (BUFFER_SIZE & 0x00FF)>>0;
            buffer[2] = (BUFFER_SIZE & 0xFF00)>>8;

            USART_SendBuffer(buffer,3);
            break;
        case S_CMD_Q_BUSTYPE:		// 0x04
            // Query supported bustypes
            // TODO: LPC / FWH IO support via PP-Mode
            // ACK
            buffer[0] = S_ACK;
            //  8-bit flags (as per flashrom)
            buffer[1] = S_SUPPORTED_BUS;

            USART_SendBuffer(buffer,2);
            break;
        case S_CMD_Q_CHIPSIZE:
            break;
        case S_CMD_Q_OPBUF:
            // TODO: opbuf function 0
            break;
        case S_CMD_Q_WRNMAXLEN:
            break;
        case S_CMD_R_BYTE:
            break;
        case S_CMD_R_NBYTES:
            break;
        case S_CMD_O_INIT:
            break;
        case S_CMD_O_WRITEB:
            // TODO: opbuf function 1
            break;
        case S_CMD_O_WRITEN:
            // TODO: opbuf function 2
            break;
        case S_CMD_O_DELAY:
            // TODO: opbuf function 3
            break;
        case S_CMD_O_EXEC:
            // TODO: opbuf function 4
            break;
        case S_CMD_SYNCNOP:			// 0x10
            // Sync NOP
            // NAK
            buffer[0] = S_NAK;
            // ACK
            buffer[1] = S_ACK;
            USART_SendBuffer(buffer,2);
            break;
        case S_CMD_Q_RDNMAXLEN:
            // TODO
            break;
        case S_CMD_S_BUSTYPE:		// 0x12
            // Set used bustype

            /* We do not have multiplexed bus interfaces,
             * so simply ack on supported types, no setup needed. */
            if((FIFO_ReadByte(pRxFIFO) | S_SUPPORTED_BUS)==S_SUPPORTED_BUS) {
                // ACK
                buffer[0] = S_ACK;
            } else {
                // NAK
                buffer[0] = S_NAK;
            }
            USART_SendBuffer(buffer,1);
            break;
        case S_CMD_O_SPIOP:			// 0x13
            // Perform SPI operation

            // 24 bit slen
            FIFO_Read(pRxFIFO,(uint8_t*)&slen,3);
            FIFO_Read(pRxFIFO,(uint8_t*)&rlen,3);
            //bprintf("slen=%d rlen=%d\r\n",slen,rlen);
            memset(buffer,0,sizeof(buffer));
            GPIO_ResetBits(GPIOA,GPIO_Pin_15);
			while(slen){
				tlen = MIN(BUFFER_SIZE,slen);
                tlen = FIFO_Read(pRxFIFO,buffer,tlen);
                W25X_SendBuffer(buffer,tlen);
				slen -= tlen;
            }
			buffer[0] = S_ACK;
			USART_SendBuffer(buffer,1);
            while(rlen) {
				tlen = MIN(BUFFER_SIZE,rlen);
				W25X_ReceiveBuffer(buffer,tlen);
				USART_SendBuffer(buffer,tlen);
				rlen -= tlen;
            }

            GPIO_SetBits(GPIOA,GPIO_Pin_15);
            //select_chip();

            /* TODO: handle errors with S_NAK */
            //if(slen) spi_bulk_write(slen);
            //usb_putc(S_ACK);
            //if(rlen) spi_bulk_read(rlen);

            //unselect_chip();
            break;
        case S_CMD_S_SPI_FREQ:
            // Set SPI clock frequency in Hz

            // 32-bit requested frequency
            FIFO_Read(pRxFIFO,(uint8_t*)&freq_req,sizeof(freq_req));
            //freq_req = usb_getu32();
            if(freq_req == 0) {
				// NAK
				buffer[0] = S_NAK;
				USART_SendBuffer(buffer,1);
            } else {
            	// ACK
            	buffer[0] = S_ACK;
				// 32-bit set frequency
				memcpy(buffer+1,&freq_req,sizeof(freq_req));
				USART_SendBuffer(buffer,5);
            }
            break;
		case S_CMD_S_LOCKSTATE:
			pstLockEvent = pvPortMalloc(sizeof(FLASHROM_LOCK_EVENT));

			pstLockEvent->Head.iID = FLASHROM_LOCK_EVENT_ID;
			pstLockEvent->Head.iSize = sizeof(FLASHROM_LOCK_EVENT);

			pstLockEvent->Data = FIFO_ReadByte(pRxFIFO);
			xResult = xQueueSend(hEventQueue,&pstLockEvent,0);
			if(xResult == pdTRUE){
				buffer[0] = S_ACK;
			}else{
				buffer[0] = S_NAK;
			}
			USART_SendBuffer(buffer,1);
			break;
		case S_CMD_S_PROGPARAM:
			pstSetupEvent = pvPortMalloc(sizeof(FLASHROM_PBAR_SETUP_EVENT));

			pstSetupEvent->Head.iID = FLASHROM_PBAR_SETUP_EVENT_ID;
			pstSetupEvent->Head.iSize = sizeof(FLASHROM_PBAR_SETUP_EVENT);

			pstSetupEvent->Data.eProgressBarType = FIFO_ReadByte(pRxFIFO);
			FIFO_Read(pRxFIFO,(uint8_t*)&pstSetupEvent->Data.uiBytesTotal,sizeof(uint32_t));

			xResult = xQueueSend(hEventQueue,&pstSetupEvent,0);
			if(xResult == pdTRUE){
				buffer[0] = S_ACK;
			}else{
				buffer[0] = S_NAK;
			}
			USART_SendBuffer(buffer,1);
			break;
		case S_CMD_S_PROGSTATE:
			pstUpdateEvent = pvPortMalloc(sizeof(FLASHROM_PBAR_UPDATE_EVENT));

			pstUpdateEvent->Head.iID = FLASHROM_PBAR_UPDATE_EVENT_ID;
			pstUpdateEvent->Head.iSize = sizeof(FLASHROM_PBAR_UPDATE_EVENT);

			FIFO_Read(pRxFIFO,(uint8_t*)&pstUpdateEvent->Data,sizeof(uint32_t));

			xResult = xQueueSend(hEventQueue,&pstUpdateEvent,0);
			if(xResult == pdTRUE){
				buffer[0] = S_ACK;
			}else{
				buffer[0] = S_NAK;
			}
			USART_SendBuffer(buffer,1);
			break;
        default:
            break; // TODO: Debug malformed command
        }
    }
}
