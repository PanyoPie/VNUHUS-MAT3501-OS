/* ----------------------------------------------------------------------------------------
	Main program for testing the virtual memory emulation.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Read user's input as <vAddr> to read from or <vAddr> <value> to write to the
		virtual memory, -1 to exit.
	Compilation: run 'make' or
		gcc -o main main.c virtualMem.c ramEmu.c FIFO.c CLOCK.c LRU.c
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef RAMEMU_H
#include "ramEmu.h"
#endif

#ifndef VIRTUALMEM_H
#include "virtualMem.h"
#endif

int algo;

void main(int argc, char *argv[])
{	int virtualAddr, pageNo, frameNo, offset, value, ret;
	unsigned char  byteVal;
	char para[50];

	if (argc!=2) {
		fprintf(stderr, "Usage: %s <FIFO / CLOCK / LRU>\n", argv[0]);
		exit(-1);
	}
	if (strcmp(argv[1],"FIFO")==0) algo = FIFO;
	else if (strcmp(argv[1],"CLOCK")==0) algo = CLOCK;
	else if (strcmp(argv[1],"LRU")==0) algo = LRU;
	else {
		fprintf(stderr, "Algorithm %s is not supported\n", argv[1]);
		exit(-1);
	}
		
	vm_initMem(algo);
	printf("Virtual memory size = %dKB, #pages = %d\n", NUMPAGES*PAGESIZE/1024, NUMPAGES);
	printf("Physical memory size: %dKB, #frames = %d\n", NUMFRAMES*FRAMESIZE/1024, NUMFRAMES);
	printf("Page size: %dKB\n", PAGESIZE/1024);
	printf("Instruction: Enter <addr> to read from, <addr> <value> to write to, or -1 to exit.\n");

	do {
		printf("\nYour input: ");
		fgets(para, 50, stdin); printf("\n");
		ret = sscanf(para, "%d %d", &virtualAddr, &value);
		byteVal = value ;
		
		if (ret==1) { // reading from memory
			if (vm_getByte(virtualAddr, &byteVal)>=0)
				printf("Vmem[%i] == %i\n", virtualAddr, byteVal);
			else printf("Unable to read at %i\n", virtualAddr);
		} else if (ret==2) {
			if (vm_setByte(virtualAddr, byteVal)>=0)
				printf("Vmem[%i] := %i\n", virtualAddr, byteVal);
			else printf("Unable to write to %i\n", virtualAddr);
		}
		else continue;
		
		vm_printPageTable();
		printPhysicalMemMap();
		
	} while (virtualAddr>=0);
}