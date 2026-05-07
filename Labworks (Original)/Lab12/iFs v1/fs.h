#define FS_H

#include <stdint.h>

#ifndef DISKEMU_H
#include "diskemu.h"
#endif

#define DIRECTBLOCKS		1
#define MAXFILENAME			16
#define MAXOPENFILES		256
#define MAXINDIRECTBLOCKS	(BLOCKSIZE/sizeof(uint16_t))

#define REGULARFILE			0
#define DIRECTORY			1
#define SYMBOLICLINK		2

#define MAXPATHNAME			256

#define FILESIZE(fd)		inodeTable[fileDescriptors[fd].inodeNo].fileSize

typedef struct {
	uint16_t inodeBlock;	
	uint16_t rootDirBlock;	
	uint16_t dataBlock;		
	uint16_t freeBlockList;	
	uint16_t freeBlockCount;
	uint16_t blockSize;		
	uint16_t rootInodeNo;	
	char zeros[BLOCKSIZE - sizeof(uint16_t)*7];	
} superBlock_t;

typedef union{
	struct {
		uint16_t mode;					
										
		uint16_t linkCount;				
		uint32_t fileSize;				
		uint16_t direct[DIRECTBLOCKS];	
		uint16_t singleIndirect;		
	};
	char zeros[16];						
} inode_t;


typedef struct {
	int state;				
	int fp;					
	uint16_t inodeNo;
	uint16_t indirectTable[BLOCKSIZE/sizeof(uint16_t)];	
} fileDescriptor_t;

typedef struct {
	char fileName[MAXFILENAME+1];
	uint16_t inodeNo;			
	int deleted;			
} dirRecord_t;


int fs_format(char *vdisk, int dsize);
int fs_badFormat(char *vdisk, int dsize);
int fs_mount (char *vdisk); 
int fs_umount(); 
int fs_create(char *filename); 
int fs_open(char *filename); 
int fs_close(int fd); 
int fs_delete(char *filename); 
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
