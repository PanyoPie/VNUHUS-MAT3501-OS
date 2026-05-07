#define FS_H

#include <stdint.h>

#ifndef DISKEMU_H
#include "diskemu.h"
#endif

#define MAXFILENAME			16
#define MAXOPENFILES		256

typedef struct {
	uint16_t fatBlock;		// location of FAT
	uint16_t rootDir;		// location of root dir
	uint16_t rootDirSize;	// size of rootDirFile
	uint16_t dataBlock;		// location of 1st data block
	uint16_t blockSize;		// block size
	uint16_t blockCount;	// number of blocks
	char zeros[BLOCKSIZE - sizeof(uint16_t)*6];	// padding zeros
} superBlock_t;

typedef struct {
	char fileName[MAXFILENAME+1];
	int state;				// closed=0; open and unmodified = 1; open and modified = 2;
	uint16_t fatIndex;		// first index node
	uint16_t lastFatIndex;	// last fat node
	int fp;					// current file position
	int fileSize;
	
} fileDescriptor_t;

typedef struct {
	char fileName[MAXFILENAME+1];
	int fileSize;
	uint16_t fatIndex;		// 1st fat node
	int deleted;			// to mark a deleted file
} dirRecord_t;

// SFS public functions
int fs_format(char *vdisk, int dsize);
int fs_mount (char *vdisk); 
int fs_umount(); 
int fs_create(char *filename); 
int fs_open(char *filename); 
int fs_close(int fd); 
int fs_delete(char *filename); 
int fs_read(int fd, void *buf, int n); 
int fs_write(int fd, void *buf, int n); 
int fs_seek(int fd, int offset); 
int fs_fileSize (int fd); 
void fs_printDir (); 
void fs_printFat (); 
int fs_eof(int fd);
int fs_diskFreeBlocks(void);

