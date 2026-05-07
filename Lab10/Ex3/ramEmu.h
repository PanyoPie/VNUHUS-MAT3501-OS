#define RAMEMU_H

#ifndef CONFIG_H
#include "config.h"
#endif

typedef struct {
	char ram[NUMFRAMES*FRAMESIZE];		// physical memory
	int algo;							// FIFO/CLOCK/LRU
	int frameFlag[NUMFRAMES];			// flag content change in frames
	int frameMap[NUMFRAMES];			// frameMap[i] tells which virtual page is loaded at frame i
	int numFrames;		 // number of physical memory frames - **obsolete**
	int current;		 // current frame position
	union {
		int used[NUMFRAMES];	// FIFO: if page frame i is occupied, used[i]==1, ==0 otherwise
		int clock[NUMFRAMES];	// CLOCK: Keeps the R attribute, =1 if the page was used, =0 if not or erased
		int time[NUMFRAMES];	// LRU: time a page is loaded into physical memory
	};
	int (*fpReplacementLookUp)();		// algorithm for page replacement
	void (*fpUpdateFrameAttributes)();	// set or clear frame attributes used for FIFO/CLOCK/LRU algos
} physicalMem_t;



extern physicalMem_t physicalMem;

void initPhysicalMem();
int searchPage(int page);
void printPhysicalMemMap();


