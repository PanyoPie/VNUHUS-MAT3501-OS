/* ----------------------------------------------------------------------------------------
	CLOCK page replacement algorithm.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: f page i is used, set its attribute R=1. Upon a page request, erase attribute R 
		and move on for a page with R=0. See physicalMem_t in ramEmu.h
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#ifndef RAMEMU_H
#include "ramEmu.h"
#endif

#ifndef CLOCK_H
#include "clock.h"
#endif

// initiate physical memory for CLOCK algo
void clock_memInit()
{
    // write your code here
	physicalMem.current = 0;
	for(int i=0; i < NUMFRAMES; i++)
        physicalMem.clock[i] = -1;
}


/* look up for a physical frame to be replaced by CLOCK algo
Instruction: frame i has a label clock[i]
	step 1 - search for a page frame with label R=0, during the search erase all labels on the way
	step 2 - remember the frame position found
	step 3 - move the clock handle to the next position
	step 4 - return the found position
*/
int clock_lookUp()
{	int p;
	
	// search for a page with R==0; otherwise clear R:=0 on the way
	// notes: -	physicalMem.current is the current frame position
	//		  -	physicalMem.clock[i] stores the R label of page i
	...
	
	// set p to the position found
	...

 	// move the clock handle to the next position
	...	
	
	return p;
}

/*	Update a frame's attributes after each access
	Params: frameNo	- frame number
			state	- set or clear a frame's attributes
*/
void clock_updateFrameAttributes(int frameNo, int state)
{
	// if state !=0 then set R:=1 otherwise clear R:=0
	...
}

