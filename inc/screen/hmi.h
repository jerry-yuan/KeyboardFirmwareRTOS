#ifndef HMI_H_INCLUDED
#define HMI_H_INCLUDED

#include <HMI_Engine.h>

#include <screen/initscr.h>
#include <screen/usbscr.h>
#include <screen/keyboardscr.h>
#include <screen/menuscr.h>

#include <screen/clock/show.h>
#include <screen/clock/edit.h>

#include <screen/calculator/calculator.h>

#include <screen/totp-authenticator/show.h>

void hmiEngineInitialize();

extern HMI_ENGINE_OBJECT* hmiEngine;

#endif /* HMI_H_INCLUDED */
