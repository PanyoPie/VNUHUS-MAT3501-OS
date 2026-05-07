/* ----------------------------------------------------------------------------------------
	Make a SFS disk.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Create a disk image file to emulate the FS disk. One can choose the disk size
		by specifying the power of 2 size, in byte.
------------------------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>

#include "diskemu.h"

int main (int argc, char *argv[])
{
	int power, size ,ret, err;
	char pDiskName[128]; 

	if (argc != 2) {
		printf("usage: %s <power2size>\n", argv[0]);
		exit (1); 
	}

	strcpy (pDiskName, DISKNAME); 
	power = atoi(argv[1]); 
	
	ret = createDisk(pDiskName, power);
	switch (ret) {
	case  0: 
		size = 1 << power; 
		printf ("Created virtual disk '%s' of size=%dKB, %d blocks, block size = %dB\n", 
			pDiskName, size/1024, size/BLOCKSIZE, BLOCKSIZE); 
		err = 0;
		break;
	case -1: 
		printf("<power2size> must be between %d and %d\n", MINPOWER2SIZE, MAXPOWER2SIZE); 
		err = -1;
		break;
	case -2:
		printf ("Could not create virtual disk\n"); 
		err = -1; 
		break;
	case -3:
		printf ("Could not write blank blocks to virtual disk\n"); 
		err = -1;
		break;
	}
	return(err);
}
