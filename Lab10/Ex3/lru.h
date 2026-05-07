#define LRU_H

#ifndef RAMEMU_H
#include "ramEmu.h"
#endif


void lru_memInit();
int lru_lookUp();
void lru_updateFrameAttributes(int frameNo, int state);