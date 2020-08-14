#ifndef HMI_H_INCLUDED
#define HMI_H_INCLUDED

#include <HMI_Engine.h>

#include <screen/initscr.h>
#include <screen/usbscr.h>
#include <screen/keyboardscr.h>
#include <screen/menuscr.h>

void hmiEngineInitialize();

extern HMI_ENGINE_OBJECT* hmiEngine;

#endif /* HMI_H_INCLUDED */
