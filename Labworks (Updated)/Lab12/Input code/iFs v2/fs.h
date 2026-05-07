#define FS_H

#include <stdint.h>

#ifndef DISKEMU_H
#include "diskemu.h"
#endif

#define DIRECTBLOCKS		1
#define MAXFILENAME			16
#define MAXOPENFILES		256
#define POINTERSIZE			2	// sizeof(uint16_t)
#define FREEBLOCKENTRIES	(BLOCKSIZE/POINTERSIZE-1)
#define MAXINDIRECTBLOCKS	(BLOCKSIZE/POINTERSIZE)


#define REGULARFILE			0
#define DIRECTORY			1
#define SYMBOLICLINK		2

#define MAXPATHNAME			256

#define FILESIZE(fd)		inodeTable[fileDescriptors[fd].inodeNo].fileSize

typedef struct {
	uint16_t inodeBlock;	// i-node block	
	uint16_t rootDirBlock;	// root dir 1st block
	uint16_t dataBlock;		// 1st data block
	uint16_t freeList_First;// 1st block of the free-block linked list
	uint16_t freeList_Last; // last block of the free-block linked list
	uint16_t freeBlockCount;// number of free blocks
	uint16_t blockSize;		// block size
	uint16_t rootInodeNo;	// inode reserved for root dir
	char zeros[BLOCKSIZE - POINTERSIZE*7];	// padding zeros
} superBlock_t;

typedef union{
	struct {
		uint16_t mode;					// file attributes, 1st 4 bits represents file types
										//		=0 regular; =1 dir; =2 symlink
		uint16_t linkCount;				// hard link count
		uint32_t fileSize;				// size of file
		uint16_t direct[DIRECTBLOCKS];	// direct pointers to data blocks
		uint16_t singleIndirect;		// single indirect pointer
	};
	char zeros[16];						// padding zeros
} inode_t;


typedef struct {
	int state;				// =0: closed; =1 open; =2 size changed
	int fp;					// current file position
	uint16_t inodeNo;
	uint16_t indirectTable[BLOCKSIZE/POINTERSIZE];	// indirect pointers
} fileDescriptor_t;

typedef struct {
	char fileName[MAXFILENAME+1];
	uint16_t inodeNo;		// i-node number
	int deleted;			// to mark a deleted file
} dirRecord_t;

// iSFS public functions
int fs_format(char *vdisk, int dsize);
int fs_badFormat(char *pDiskName, int diskSize);
int fs_mount (char *vdisk); 
int fs_umount(); 
int fs_create(char *filename); 
int fs_open(char *filename); 
int fs_close(int fd); 
int fs_delete(char *filename); 
int fs_undelete(void);
int fs_seek(int fd, int offset); 
int fs_fileSize (int fd); 
int fs_eof(int fd);
int fs_read(int fd, void *buf, int n); 
int fs_write(int fd, void *buf, int n); 
int fs_printDir (); 
void fs_printInodeTable (); 
void fs_printInode(uint16_t inodeNo);
void fs_printSuperBlock();
int fs_diskFreeBlocks(void);
int fs_createHardLink(char *pSourceName, char *pTargetName);
int fs_createSymLink(char *pSourceName, char *pTargetName);
int fs_searchDirByName(int dirFd, char *pFileName, dirRecord_t *pDirRecord, int option);
int fs_setCurrentDir(char *pPath);
int fs_makeDir(char *pPathName);
int fs_isDir(char *pPathName);
void fs_checkDisk();
char *splitPathAndName(char *pPathName);
