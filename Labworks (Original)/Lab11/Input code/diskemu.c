/* ----------------------------------------------------------------------------------------
	Disk emulation.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Emulate a disk by using a file to store its disk blocks. One can access 
		a disk block through provided functions readBlock() and writeBlock().
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "diskemu.h"

// Global Variables
char disk_name[128];   // name of virtual disk file
int  disk_size;        // size in bytes - a power of 2
int  disk_fd;          // disk file handle
int  disk_blockCount;  // block count on disk

/*
   Reads block blocknum into buffer buf.
   You will not modify the readBlock() function.
   Returns -1 if error. Should not happen.
*/
int readBlock (int blockNum, void *buf)
{
    int offset, n;

    if (blockNum >= disk_blockCount)
        return (-1); //error

    offset = lseek(disk_fd, blockNum * BLOCKSIZE, SEEK_SET);
    if (offset == -1) return(-1);		// lseek error

    n = read(disk_fd, buf, BLOCKSIZE);
    if (n != BLOCKSIZE) return (-2);	// error in reading disk file

    return (0);
}


/*
    Puts buffer buf into block blocknum.
    You will not modify the writeBlock() function
    Returns -1 if error. Should not happen.
*/
int writeBlock (int blockNum, void *buf)
{
    int offset, n;

    if (blockNum >= disk_blockCount)
        return (-1); //error

    offset = lseek (disk_fd, blockNum * BLOCKSIZE, SEEK_SET);
    if (offset == -1) return(-1);  		// lseek error

    n = write (disk_fd, buf, BLOCKSIZE);
    if (n != BLOCKSIZE) return (-2);	// error in writing to disk file

    return (0);
}


int createDisk(char *pDiskName, int power)
{	int numBlocks, ret, n, fd, i, diskSize;
	unsigned char buf[BLOCKSIZE];

	if ( (power<MINPOWER2SIZE)||(power>MAXPOWER2SIZE) ) return(-1);
	
	diskSize = 1 << power;
	numBlocks = diskSize / BLOCKSIZE; 

	ret = open (pDiskName,  O_CREAT |O_TRUNC | O_RDWR, 0666); 
	if (ret == -1) return (-2);		// could not create disk
		
	bzero ((void *)buf, BLOCKSIZE); 
	
	fd = open (pDiskName, O_RDWR); 
	
	for (i=0; i < numBlocks; ++i) {
		n = write (fd, buf, BLOCKSIZE); 
		if (n != BLOCKSIZE) {
			return (-3); 	// could not write blank blocks to disk
		}
	}
	
	close (fd); 

	return 0;
}
