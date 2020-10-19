#ifndef CALCULATOR_H_INCLUDED
#define CALCULATOR_H_INCLUDED

#include <HMI_Engine.h>
#include <consts.h>

typedef struct{
	int64_t iValue;
	uint8_t uiShift;
} AccurateFloatNumber_t;

extern HMI_SCREEN_OBJECT SCREEN_Calculator;

#endif /* CALCULATOR_H_INCLUDED */
