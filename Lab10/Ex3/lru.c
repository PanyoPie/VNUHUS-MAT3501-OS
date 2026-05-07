/* ----------------------------------------------------------------------------------------
	LRU page replacement algorithm.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Current time is represented as a counter starting from 0, increased everytime 
		a page is loaded. If a page is loaded or referred to, its time is updated with current 
		time. When a page fault occures, the page with the smallest time will be replaced.
		See physicalMem_t in ramEmu.h. Field 'current' is used as the time counter.
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


#ifndef RAMEMU_H
#include "ramEmu.h"
#endif

#ifndef LRU_H
#include "lru.h"
#endif

// Get elapsed time in seconds
static long getTime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return(currentTime.tv_sec );
}

// initiate physical memory for LRU algo
void lru_memInit()
{
    // write your code here
	physicalMem.current = 0;
    for(int i=0; i < NUMFRAMES; i++)
        physicalMem.time[i] = -1;
}


/* look up for a physical frame to be replaced by LRU algo
Instruction: search for the page frame with smallest time value
*/
int lru_lookUp()
{	int lruPage=0;
	// write your code here
	for(int i=1; i < NUMFRAMES; i++) // search for page with smallest time value
		if(physicalMem.time[i] < physicalMem.time[lruPage]) lruPage = i;

    return lruPage;
}

void lru_updateFrameAttributes(int frameNo, int state)
{
	if (state) {	// !=0 set attributes
		physicalMem.time[frameNo] = physicalMem.current; // mark the time the page was loaded
		physicalMem.current++;	// increase the time counter
	}
	else {			// ==0 clear attributes
		physicalMem.time[frameNo] = -1; // mark the frame as available
	}
}
