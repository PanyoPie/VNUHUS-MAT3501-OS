/* ----------------------------------------------------------------------------------------
	Virtual memory emulation
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Disk image of virtual memory is kept in a file. A virtual page is loaded into 
		the physical memory when a user accesses a byte on that page. Users can read or write 
		a byte from/to virtual memory. A page table is used to translate virtual addresses
		to physical addresses, and FIFO/CLOCK/LRU algorithms are provided for page replacement.
		To add a new page replacement algo, one must write a module with 2 functions: 
		memory init, and page replacement lookup, then add them in vm_initMem() function.
		One must also run vm_initMem() to instantiate the virtual memory.
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#ifndef RAMEMU_H
#include "ramEmu.h"
#endif

#ifndef FIFO_H
#include "fifo.h"
#endif

#ifndef CLOCK_H
#include "clock.h"
#endif

#ifndef LRU_H
#include "lru.h"
#endif


#define VIRTUALMEMFILE	"vmem.dat"

static int pageTable[NUMPAGES][2];	// [i][0] is the ram frame number, [i][1] is the entry validity

/* printing functions --------------------------------------- */

// for binary printing purpose only
static int getBit(u_int16_t value, int pos)	
{	int i, j;
		
	if ((pos>15)||(pos<0)) return (-1);
	
	value = value << (15-pos);
	value = value >> 15;
	return (value);
}

static void binaryPrint(int value, int bitsNum)
{	int i;
	for (i=bitsNum; i>0; i--) {
		printf("%1d", getBit(value, i-1)); 
	}
}

void vm_printPageTable()
{	int i;
	printf("\nPage table:\n");
	printf("Page	Frame	State\n");
	printf("----	-----	-----\n");
	for (i=0; i<NUMPAGES; i++)
		printf("%4d\t%4d\t%c\n", i, pageTable[i][0], pageTable[i][1]?'T':'F');
}

/* Internal functions for virtual memory management  --------------------- */

// Convert an virtual address to physical address using Page Table
static int virtual2PhysicalAddr(int virtualAddr)
{	int pageNo, frameNo, offsetAddr, physicalAddr;
	
	
	pageNo = virtualAddr >> OFFSETBITS; // pageNo = virtualAddr / PAGESIZE;	
	offsetAddr = virtualAddr & (PAGESIZE - 1); 	//offsetAddr = virtualAddr % PAGESIZE;

	if (pageTable[pageNo][1]) { // the entry is still valid
		frameNo = pageTable[pageNo][0]; // get the physical page frame
		// replace pageNumber by frameNumber, keep the offset address
		physicalAddr = (frameNo<<OFFSETBITS) | offsetAddr; //physicalAddr = frameNo*FRAMESIZE + offsetAddr;
	}
	else return(-1);	// not yet loaded in , page fault
	
	printf("Virtual address = "); 
	binaryPrint(pageNo, PAGEBITS); printf(" "); binaryPrint(offsetAddr, OFFSETBITS);
	printf("\n");
	
	printf("Physical address = "); 
	binaryPrint(frameNo, FRAMEBITS); printf(" "); binaryPrint(offsetAddr, OFFSETBITS);
	printf("\n");
	
	return(physicalAddr);
}

// check if a virtual address is currently mapped to physical memory 
static int isInPhysicalMem(int virtualAddr)
{	int pageNo;
	
	pageNo = virtualAddr >> OFFSETBITS; 	// = virtualAddr / PAGESIZE;	
	if (pageTable[pageNo][1]) return 1;		// i.e return(pageTable[pageNo][1]);
	else return 0;
}


// write content of physical frame frameNo to disk image as virtual page pageNo
static int write2Disk(int frameNo, int pageNo)
{	FILE *fp;
	
	fp = fopen(VIRTUALMEMFILE, "r+");
	if (fp==NULL) return (-1);
	fseek(fp, pageNo*PAGESIZE, SEEK_SET);
	fwrite(physicalMem.ram + frameNo*FRAMESIZE, FRAMESIZE, 1, fp);
	fclose(fp);
	return 0;
}

// load virtual page pageNo from disk to physical frame frameNo
static int readFromDisk(int pageNo, int frameNo)
{	FILE *fp;
	
	fp = fopen(VIRTUALMEMFILE, "r");
	if (fp==NULL) return (-1);
	fseek(fp, pageNo*PAGESIZE, SEEK_SET);
	fread(physicalMem.ram + frameNo*FRAMESIZE, FRAMESIZE, 1, fp);
	fclose(fp);
	return 0;
}

// handle page fault when a virtual address is not currently mapped to physical memory, i.e not found in Page Table
static int pageFault(int virtualAddr)
{	int pageNo, frameNo, i, p;
	
	// pick a frame to be replaced
	frameNo = (*(physicalMem.fpReplacementLookUp))();
	p = physicalMem.frameMap[frameNo];

	if (p>=0) // a virtual page was previously mapped to this frame
		pageTable[p][1] = 0;	// mark the entry as not valid/unavailable
	
	// write to disk if frame content has been changed
	if (physicalMem.frameFlag[frameNo]) {
		if (write2Disk(frameNo, physicalMem.frameMap[frameNo])<0) return (-1);
		printf("** Page %d at frame %d is saved to disk\n", p, frameNo);
	}
	
	// load new virtual page into physical memory
	pageNo = virtualAddr >> OFFSETBITS; //pageNo = virtualAddr / PAGESIZE;
	if (readFromDisk(pageNo, frameNo)<0) return (-1);
	physicalMem.frameFlag[frameNo] = 0;
	physicalMem.frameMap[frameNo] = pageNo;
	
	// update page table
	pageTable[pageNo][0] = frameNo;
	pageTable[pageNo][1] = 1;
	
	if (p>=0) printf("** Page fault: page %d is replaced by %d at frame %d\n", p, pageNo, frameNo); 
	
	return (0);
}

/* public function for accessing virtual memory ------------------------------------- */

// write byteVal to virtual memory at virtualAddr
int vm_setByte(int virtualAddr, char byteVal)
{	int physicalAddr, pageNo, frameNo;
	
	if ( (virtualAddr<0)||(virtualAddr>=(1<<(PAGEBITS+OFFSETBITS))) ) 
		return(-1);	// out of range
	
	
	if (!isInPhysicalMem(virtualAddr)) pageFault(virtualAddr);
	
	physicalAddr = virtual2PhysicalAddr(virtualAddr);
	frameNo = physicalAddr >> OFFSETBITS;
	physicalMem.ram[physicalAddr] = byteVal;
	
	physicalMem.frameFlag[frameNo] = 1;					// frame content changed
	(*physicalMem.fpUpdateFrameAttributes)(frameNo, 1);	// update frame attributes every time the page is used
	
	return 0;
}

// read a byte from virtual memory at virtualAddr
int vm_getByte(int virtualAddr, char *pByte)
{	int physicalAddr, frameNo;

	if ( (virtualAddr<0)||(virtualAddr>=(1<<(PAGEBITS+OFFSETBITS))) ) 
		return(-1);	// out of range
	
	
	if (!isInPhysicalMem(virtualAddr)) pageFault(virtualAddr);
	
	physicalAddr = virtual2PhysicalAddr(virtualAddr);
	*pByte = physicalMem.ram[physicalAddr];
	
	// update frame attributes every time the page is used
	frameNo = physicalAddr >> OFFSETBITS;
	(*physicalMem.fpUpdateFrameAttributes)(frameNo, 1);	
	
	return 0;
}

// initiate virtual memory,  physical memory
int vm_initMem(int algo)	// init virtual and physical memory
{	FILE *fp;
	char buf[PAGESIZE];
	int i, j;
	
	fp = fopen(VIRTUALMEMFILE, "w");
	if (fp==NULL) {
		fprintf(stderr, "Cannot create image file for virtual memory\n");
		return(-1);
	}
	for (i=0; i<NUMPAGES; i++) {
		for (j=0; j<PAGESIZE; j++) buf[j] = i; // page i filled with i
		fwrite(buf, 1, PAGESIZE, fp);	
	}
	fclose(fp);
	
	
	// Initialize physical memory and replacement algo
	initPhysicalMem(algo);
	
	switch (algo) {
	case FIFO:
		fifo_memInit();
		physicalMem.fpReplacementLookUp=&fifo_lookUp;
		physicalMem.fpUpdateFrameAttributes=&fifo_updateFrameAttributes;
		break;
	case CLOCK:
		clock_memInit();
		physicalMem.fpReplacementLookUp=&clock_lookUp;
		physicalMem.fpUpdateFrameAttributes=&clock_updateFrameAttributes;
		break;
	case LRU:
		lru_memInit();
		physicalMem.fpReplacementLookUp=&lru_lookUp;
		physicalMem.fpUpdateFrameAttributes=&lru_updateFrameAttributes;
		break;
	default: return (-1);
	}
		
	return 0;
}

