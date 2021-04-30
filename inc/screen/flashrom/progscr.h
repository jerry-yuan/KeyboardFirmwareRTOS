#ifndef SCREEN_FLASHROM_PROGSCR_H
#define SCREEN_FLASHROM_PROGSCR_H

#include <SGUI_ProcessBar.h>
#include <HMI_Engine.h>
#include <screen/flashrom/task.h>
typedef struct{
	FLASHROM_PROGRESS_BAR_TYPE eType;
	SGUI_PROCBAR_STRUCT stProgressBar;
} FlashRomProgScrContext_t;

extern HMI_SCREEN_OBJECT SCREEN_FlashRom_Prog;

#endif
