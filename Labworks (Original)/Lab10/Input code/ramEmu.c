/* ----------------------------------------------------------------------------------------
	Physical memory emulation
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Physical memory is emulated as an array of bytes and related attributes for
		page replacement and swapping purposes. One must first run initPhysicalMem() before use.
------------------------------------------------------------------------------------------- */

#include <stdio.h>

#ifndef RAMEMU_H
#include "ramEmu.h"
#endif

physicalMem_t physicalMem;

void initPhysicalMem(int algo)
{	int i;
	physicalMem.algo = algo;
	for (i=0; i<NUMFRAMES; i++) 
		physicalMem.frameMap[i]=-1; // no virtual pages loaded yet
}

int searchPage(int page){
	int i;
    //Search if virtual memory page 'page' is already in physical memory
    for(i = 0; i < NUMFRAMES; i++)
        if(physicalMem.frameMap[i] == page) return i;
    return -1;

}

void printPhysicalMemMap()
{	int i;

    //Print all physical memory pages to screen
	printf("\nPhysical memory frame map:\n");
	for (i=0; i<NUMFRAMES; i++) 
		printf("Fr%-3d ", i);
	printf("\n");
	for (i=0; i<NUMFRAMES; i++) 
		printf("----- ");
	printf("\n");
    for (i=0; i<NUMFRAMES; i++) 
		if (physicalMem.frameMap[i]<0) printf(" ... |");
		else printf(" %3d |", physicalMem.frameMap[i]);
	printf("\n");
	for (i=0; i<NUMFRAMES; i++) 
		printf("----- ");
	printf("\n");
	
	switch (physicalMem.algo) {
	case FIFO:
		for (i=0; i<NUMFRAMES; i++) 
			printf("  %1d   ", physicalMem.used[i]);
		printf("\tused\n");
		break;
	case CLOCK:
		for (i=0; i<NUMFRAMES; i++) 
			printf("  %1d   ", physicalMem.clock[i]);
		printf("\tclock\n");
		break;
	case LRU:
		for (i=0; i<NUMFRAMES; i++) 
			printf("  %1d   ", physicalMem.time[i]);
		printf("\ttime\n");
		break;
	}
}

