#ifndef DISPLAY_H
#define DISPLAY_H

#include <PxMatrix.h>

extern PxMATRIX display;
extern boolean clockMode;

void setupDisplay();
uint16_t randomColor();

#endif