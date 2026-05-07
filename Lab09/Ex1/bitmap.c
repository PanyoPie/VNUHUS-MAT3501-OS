/* ----------------------------------------------------------------------------------------
	Memory allocation simulation using bitmap.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#define PAGESIZE	4096
#define MAXBYTES	8
#define MAXPAGES	8*MAXBYTES
#define MAXPROCESSES	256

#define B2b_t "%c%c%c%c%c%c%c%c"
#define B2b(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 
  
static unsigned char bitmap[MAXBYTES];

static struct {
	unsigned int state; 	// 0 not in memory, 1 in memory
	unsigned int startPage;
	unsigned int numPages;
} processList[MAXPROCESSES];

/* Bitmap functions --------------------------------------------------------------*/

/*
Get a bit from memory bitmap at given position (i.e page number)
Params:
	bitPos - position in the bitmap
Return:
	value of the bit
*/
int getBit(int bitPos)
{	int i, j;
	unsigned char val;
		
	i = bitPos / 8;	// byte position
	j = 7 - bitPos % 8;	// bit position in byte
	val = bitmap[i] << 7-j;
	val = val >> 7;
	return (val);
}

/*
Set a bit value (0/1) to memory bitmap at given position (i.e page number)
Params:
	bitPos - position in the bitmap
	value - bit value to set (0/1)
Return: none
*/
void setBit(int bitPos, int value)
{	int i, j;
			
	i = bitPos / 8;	// byte position
	j = 7 - bitPos % 8;	// bit position in byte
	
	if (value) bitmap[i] |= (1 << j);
	else bitmap[i] &= (0xFF ^ (1<<j));
}

/* 
Flip k bits in memory bitmap at position bitPos 
Params:
	bitPos - starting position
	k - number of bits to flipBits
Return: none
*/
void flipBits(int bitPos, int k)
{	int i;

	for (i=bitPos; i<bitPos+k; i++) 
		if (getBit(i)) setBit(i,0);
		else setBit(i,1);
		//setBit(i,!getBit(i));
}

/*
Look for k zero bits in memory bitmap using first fit, 
Params:
	k - number of pages requested
Return: the 1st bit position
	or -1 if not found
*/
int firstFitSearch(int k)
{	int i, j;
	
	// write your code here
	for (i = 0; i <= MAXPAGES - k; i++) {
		for (j = 0; j < k; j++) {
			if (getBit(i + j)) {
				break;
			}
		}

		if (j == k) {
			return i;
		}
	}

	return -1;
}


/*
Look for k zero bits in memory bitmap using next fit
Params:
	k - number of pages requested
Return: the 1st bit position
	or -1 if not found
*/
int nextFitSearch(int k)
{	static int lastPos = 0; 	// static variable remember value of the last run
	int i, j;

	// continue from the last position
	for (i=lastPos; i<=MAXPAGES-k; i++) {
		for (j=0; j<k; j++) 
			if (getBit(i+j)) break;	
		if (j==k) break;
	}
	if (i<=MAXPAGES-k) { lastPos=i+k; return i; }
	
	// if not found, resume the search from the beginning
	for (i=0; i<=lastPos-k; i++) {
		for (j=0; j<k; j++) 
			if (getBit(i+j)) break;	
		if (j==k) break;
	}
	if (i<=lastPos-k) { lastPos=i+k; return i; }
	else return -1;
}


/*
Look for k zero bits in memory bitmap using best fit
Params:
	k - number of pages requested
Return: the 1st bit position
	or -1 if not found
*/
int bestFitSearch(int k)
{	int i, j, count, minCount = MAXPAGES+1, pos = -1;

	for (i=0; i<=MAXPAGES-k; i++) {
		count = 0;
		while ( (i+count<MAXPAGES) && (!getBit(i+count)) ) count++;
		if ( (count>=k)&&(count < minCount) ) { 
			minCount = count; 
			pos = i;
		}
		i += count;
	}
	return (pos);
}

/*
Look for k zero bits in memory bitmap using worst fit
Params:
	k - number of pages requested
Return: the 1st bit position
	or -1 if not found
*/
int worstFitSearch(int k)
{	int i, j, count, maxCount = 0, pos = -1;

	// write your code here
	for (i = 0; i < MAXPAGES; i++) {
		count = 0;

		while ((i + count < MAXPAGES) && (!getBit(i + count))) {
			count++;
		}

		if (count >= k && count > maxCount) {
			maxCount = count;
			pos = i;
		}

		i += count;
	}

	return pos;
}


/* Memory management functions ---------------------------------------------------*/

/*
Load a process into memory
Params:
	name - process name (one letter only)
	pages - number of pages requested
Return:
	 0 - succesful
	-1 - process already in loaded
	-2 - memory is full
*/
int loadProcess(unsigned char name, int pages)
{	int pos;
	
	if (processList[name].state) return(-1); // already loaded
		
	// use fist/next/best/worst fit algo here
	pos = worstFitSearch(pages);
	
	if (pos<0) return (-2);				// memory is full

	processList[name].startPage = pos;
	processList[name].numPages = pages;
	processList[name].state = 1;
	flipBits(pos, pages);
	return 0;
}

/*
Unload a process from memory given the process letter name
Params:
	name - process name
Return: 0 if successful, -1 otherwise
*/
int unloadProcess(unsigned char name)
{	
	if (processList[name].state==0) return(-1);	// not in memory
	flipBits(processList[name].startPage, processList[name].numPages);
	processList[name].state = 0;			// mark unused
	return 0;
}

// Test code
void main(void)
{	int i, pages, err; 
	unsigned char name;
			
	printf("Instruction: input a letter for process name and number of pages, e.g a 10. Enter '!' to terminate\n");
	
	// Print starting memory map
	for (i=0; i<MAXBYTES; i++) printf(" "B2b_t, B2b(bitmap[i]));
	printf("\n");
	
	// get input
	while (1) {
		scanf("%c", &name);
		if (name=='!') break;
		scanf("%d", &pages);
		if (((name >= 'a')&&(name<='z')) ||
			((name >= 'A')&&(name<='Z')) ) {
			if (pages>0) {
				err = loadProcess(name, pages);
				switch (err) {
				case -1: fprintf(stderr, "Process %c is already in memory\n", name); break;
				case -2: fprintf(stderr, "Process list is full\n"); break;
				case -3: fprintf(stderr, "Memory is full\n"); break;
				}
			}
			else unloadProcess(name);
			// Print memory map
			for (i=0; i<MAXBYTES; i++) printf(" "B2b_t, B2b(bitmap[i]));
			printf("\n");
		}
	}
}


