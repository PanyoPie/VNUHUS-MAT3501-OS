#define CLOCK_H

#ifndef RAMEMU_H
#include "ramEmu.h"
#endif


void clock_memInit();
int clock_lookUp();
void clock_updateFrameAttributes(int frameNo, int state);