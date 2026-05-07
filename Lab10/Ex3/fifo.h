#define FIFO_H

#ifndef RAMEMU_H
#include "ramEmu.h"
#endif


void fifo_memInit();
int fifo_lookUp();
void fifo_updateFrameAttributes(int frameNo, int state);