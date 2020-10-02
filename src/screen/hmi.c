#include <hmi.h>
#include <bsp/oled.h>
#include <string.h>
#include <FreeRTOS.h>
HMI_ENGINE_OBJECT* hmiEngine;
HMI_SCREEN_OBJECT* screens[] = {
    &SCREEN_Init,
    &SCREEN_USB,
    &SCREEN_Keyboard,
    &SCREEN_Menu,
    &SCREEN_Clock_Show,
    &SCREEN_Clock_Edit,
    &SCREEN_Calculator
};

void hmiEngineInitialize() {
    hmiEngine = pvPortMalloc(sizeof(HMI_ENGINE_OBJECT));
    memset(hmiEngine,0,sizeof(HMI_ENGINE_OBJECT));
    hmiEngine->ScreenCount  = sizeof(screens) / sizeof(HMI_SCREEN_OBJECT*);
    hmiEngine->ScreenObjPtr = screens;
    hmiEngine->Interface    = screen;

    for(uint8_t i=0;i<hmiEngine->ScreenCount;i++){
        if(screens[i]->pstActions->Initialize!=NULL){
            screens[i]->pstActions->Initialize(screen);
        }
        screens[i]->pstPrevious=NULL;
    }
}
