#define DISKEMU_H

// the following constants can be modified
#define BLOCKSIZEBITS	9				// suggested value: 9, 10, 11, 12 (i.e 512, 1024, 2048, 4096 bytes)
#define DISKNAME		"diskimage"		// name of disk emulation file

// do not modify the followings
#define BLOCKSIZE		(1<<BLOCKSIZEBITS)   	// Block size = 2^BLOCKSIZEBITS
#define MAXBLOCKCOUNT	(BLOCKSIZE/2)			// 16-bit fat node, i.e 2 bytes. MAXBLOCKCOUNT determines the size of FAT table
#define MAXPOWER2SIZE	(2*BLOCKSIZEBITS-1) 	// max disk size in byte
#define MINPOWER2SIZE	(BLOCKSIZEBITS+3)		// min disk size in byte. The disk must have no less than 8 blocks.

int readBlock (int blockNum, void *buf);
int writeBlock (int blockNum, void *buf);
int createDisk(char *pDiskName, int diskSize);