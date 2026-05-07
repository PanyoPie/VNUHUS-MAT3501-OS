/* ----------------------------------------------------------------------------------------
	File System.
	Course: MAT3501 - Principles of Operating System, MIM - HUS
	Summary: Provide basic functions to access FS file system. Public FS functions start 
		with fs_ prefix.
		To start using FS, one must mount the disk file to load neccessary FS meta data 
		onto memory. 
		When finish using FS, one must unmount the disk file to update FS changes back to disk.
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

#include "fs.h"

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

// Global Variables
extern char disk_name[128];   // name of virtual disk file
extern int  disk_size;        // size in bytes - a power of 2
extern int  disk_fd;          // disk file handle
extern int  disk_blockCount;  // block count on disk
extern char currentDir[]; // in fsshell.c - absolute path to current dir

static superBlock_t superBlock;		// content of super block
static inode_t inodeTable[BLOCKSIZE/sizeof(inode_t)];	// 1 block inode table
static fileDescriptor_t fileDescriptors[MAXOPENFILES]; // list of fs file handles
static int fileCount;					// current available file descriptor
static int rootFd;						// file descriptor of root dir
static int numInodes;					// number of inodes
static int currentDirFd;					// file descriptor of current dir

// supporting functions for iSFS module only
int wildCmp(char *pPattern, char *pString);
static int getFreeInode();
static int getBlockIndex(int fd, uint16_t blockSeq) ;
static void addFreeBlock(uint16_t blockNo);
static int getFreeBlock();
static int openFileByInode(uint16_t inodeNo);
static int searchBlockByName(uint16_t blockNo, char *pFileName, dirRecord_t *pDirRecord);
static int path2inode(char *pPath, dirRecord_t *pDirRecord, uint16_t *pParentInodeNo);


/* format disk
	pDiskName: disk name
	diskSize: disk size in bytes
	return: 0 on success, <0 on faults
*/
int fs_format(char *pDiskName, int diskSize)
{	int i, ret, *pVal;
	char buf[BLOCKSIZE];
	dirRecord_t dirRecord;

    strcpy (disk_name, pDiskName);
    disk_size = diskSize;
    disk_blockCount = disk_size / BLOCKSIZE;

    disk_fd = open (disk_name, O_RDWR);
    if (disk_fd == -1) return (-1);		// Cannot open disk
	
    // format super block
    superBlock.inodeBlock = 2;
    superBlock.rootDirBlock = 3;
	superBlock.rootInodeNo = 0;
	superBlock.dataBlock = 3;
	superBlock.freeBlockList = superBlock.dataBlock + 1;
	superBlock.blockSize=BLOCKSIZE;
	superBlock.freeBlockCount=disk_size / BLOCKSIZE - 4;
	ret = writeBlock(1, &superBlock);
	if (ret<0) return(-2);
	
	// prepare root dir
	memset(&buf, 0, BLOCKSIZE);
	strcpy(dirRecord.fileName,".");
	dirRecord.inodeNo = superBlock.rootInodeNo;
	dirRecord.deleted = 0;
	memcpy(buf, &dirRecord, sizeof(dirRecord_t));
	strcpy(dirRecord.fileName,"..");
	dirRecord.inodeNo = superBlock.rootInodeNo;
	dirRecord.deleted = 0;
	memcpy(buf + sizeof(dirRecord_t), &dirRecord, sizeof(dirRecord_t));
	ret = writeBlock(superBlock.rootDirBlock, buf);
	if (ret<0) return(-2);
	

	// set inode 0 reserved for root dir
	memset(inodeTable, 0, sizeof(inodeTable));
	inodeTable[superBlock.rootInodeNo].mode |= DIRECTORY << 12;
	inodeTable[superBlock.rootInodeNo].linkCount = 2;
	inodeTable[superBlock.rootInodeNo].fileSize = 2*sizeof(dirRecord_t);
	inodeTable[superBlock.rootInodeNo].direct[0] = superBlock.rootDirBlock;
	inodeTable[superBlock.rootInodeNo].singleIndirect = 0;
	ret = writeBlock(superBlock.inodeBlock, inodeTable);
	if (ret<0) return(-2);
	

	// create free block chain
	memset(&buf, 0, BLOCKSIZE);
	ret = writeBlock(disk_blockCount-1, &buf);	// last free block
	if (ret<0) return(-2);
	pVal = (int*) buf;
	for (i=superBlock.freeBlockList; i<disk_blockCount-1; i++) {
		*pVal = i+1;		// block i points to i+1
		ret = writeBlock(i, &buf);
		if (ret<0) return(-2);
	}
	
    fsync (disk_fd);
    close (disk_fd);
	
    return (0);
}


/*
   Mount the file system and make it ready: load super block, inode table into RAM,
   open root directory.
   return: 0 on success, <0 on faults
*/
int fs_mount (char *pDiskName)
{
    struct stat finfo;

    strcpy (disk_name, pDiskName);
    disk_fd = open (disk_name, O_RDWR);
    if (disk_fd == -1) return(-1);

    fstat (disk_fd, &finfo);
    disk_size = (int) finfo.st_size;
    disk_blockCount = disk_size / BLOCKSIZE;

    // start mounting
	readBlock(1, &superBlock);				// load superblock to RAM
	if (superBlock.freeBlockCount==0 || superBlock.blockSize!=BLOCKSIZE) {		
		// disk not formatted or incompatible blocksize
		close(disk_fd);
		return(-2); 
	}
	readBlock(superBlock.inodeBlock, &inodeTable);	// load inode block to RAM
	
	// root dir has file descriptor 0
	rootFd = 0;
	fileDescriptors[rootFd].state = 1;			// file is open
	fileDescriptors[rootFd].fp = 0;
	fileDescriptors[rootFd].inodeNo = superBlock.rootInodeNo;
	fileCount++;
	currentDirFd = rootFd;	// starting from root dir
	
	numInodes = BLOCKSIZE/sizeof(inode_t);
	
	/*printf ("fs_mount: mounting %s, size=%dKB, free=%dKB\n", disk_name,
        (int) finfo.st_size/1024, fs_diskFreeBlocks()*BLOCKSIZE/1024);
	*/
	
    return (0);
}


/* unmount the file system: write changes in superblock, inode table back to disk
*/
int fs_umount()
{
	for (int i=0; i<MAXOPENFILES; i++)	// close all open files
		if (fileDescriptors[i].state>0) fs_close(i);
	
    writeBlock(1, &superBlock);
	writeBlock(superBlock.inodeBlock, &inodeTable);	// update inode table to disk

    fsync (disk_fd);
    close (disk_fd);
    return (0);
}


/* create a file in current directory 
	return: 0 success, <0 faults
*/
int fs_create(char *pPathName)
{	dirRecord_t dirRecord;
	int inodeNo, ret;
	uint16_t parentInodeNo, mode;
	char *pFileName, pathName[MAXPATHNAME];
	int dirFd;
	
	if (strlen(pPathName)>=MAXPATHNAME) return(-5);
	strcpy(pathName, pPathName);
		
    // search dir if filename already exists
    ret = path2inode(pathName, &dirRecord, &parentInodeNo);
    if (ret==0) return(-1);			// file already exists
	if (ret==-1) return(-2);		// or invalid path

	pFileName = splitPathAndName(pathName);

	// open parent dir
    if (strlen(pathName)==0 || pathName==pFileName) {
		if (strlen(pathName)==0) dirFd = rootFd;
		else dirFd = currentDirFd;
	} 
	else {
		dirFd = openFileByInode(parentInodeNo);
		if (dirFd<0) return(-3);	// path is invalid
	}

	// prepare dir record entry
	strcpy(dirRecord.fileName, pFileName);
	dirRecord.deleted = 0;
	inodeNo = getFreeInode();
	if (inodeNo<0) {
		fprintf(stderr, "Inode table full\n");
		return(-4);	// out of inodes
	}
	dirRecord.inodeNo = inodeNo;

	// write to the parent dir
	fs_seek(dirFd, FILESIZE(dirFd));
	fs_write(dirFd, &dirRecord, sizeof(dirRecord_t));
	if (dirFd != currentDirFd && dirFd!=rootFd) fs_close(dirFd);

	// update inode table
	inodeTable[parentInodeNo].linkCount++;	// dir link count++
	mode = inodeTable[inodeNo].mode;
	mode = (mode << 4)>>4;
	mode |= (REGULARFILE<<12);
	inodeTable[inodeNo].mode = mode;	// newly created one is a regular file

	return (0);
 }


/* open file filename 
	return: 0 success, <0 faults
*/
int fs_open(char *pPathName)
{	dirRecord_t dirRecord;
	uint16_t parentInodeNo;
	char buf[BLOCKSIZE], pathName[MAXPATHNAME], linkName[MAXPATHNAME];
	int ret, fd = -1;

	strcpy(pathName, pPathName); 
	ret = path2inode(pathName, &dirRecord, &parentInodeNo);
	if (ret<0) return(-1);	// invalid name

	// check if it is a symlink, open the target file instead
	if (inodeTable[dirRecord.inodeNo].mode>>12 == SYMBOLICLINK) {
		readBlock(inodeTable[dirRecord.inodeNo].direct[0], buf);
		sscanf(buf, "%s", linkName);
		
		ret = path2inode(linkName, &dirRecord, &parentInodeNo);
		if (ret<0) return (fd);	// invalide link name
	}
		
	fd = openFileByInode(dirRecord.inodeNo);
	return (fd);
}

/* close an open file */
int fs_close (int fd)
{	uint16_t indBlock;

    // write your code
	indBlock = inodeTable[fileDescriptors[fd].inodeNo].singleIndirect;
	if ((indBlock>0)&&(fileDescriptors[fd].state==2))
		// update changes in indirect table to disk
		writeBlock(indBlock, fileDescriptors[fd].indirectTable);
	
	fileDescriptors[fd].state = 0; // closed
	
    return (0);
}

/* check for end of file
	return: 1 on yes, 0 on no
*/
int fs_eof(int fd)
{
    return (fileDescriptors[fd].fp>=inodeTable[fileDescriptors[fd].inodeNo].fileSize);
}

/* seek to position offset in an open file */
int fs_seek(int fd, int offset)
{
    int position = -1;

	if (offset <= FILESIZE(fd)) {
		fileDescriptors[fd].fp = offset;
		position = offset;
	}

    return (position);
}

/* get size of an open file */
int fs_fileSize (int fd)
{   int size = -1;
    
	size = FILESIZE(fd);
    return (size);
}

/* Delete a file name 
   Free direct blocks, indirect blocks.
   The file entry in directory is marked deleted, instead of being completely removed.
*/
int fs_delete(char *pPathName)
{	dirRecord_t dirRecord;
	int i, j, numBlocks, dirFd, found=0;
	uint16_t indirectBlock[BLOCKSIZE/sizeof(uint16_t)];
	inode_t *pInode;
	char *pFileName, pathName[MAXPATHNAME];

	if (strlen(pPathName)>=MAXPATHNAME) return(-5);
	strcpy(pathName, pPathName);
		
	// delete multiple files with wild cards
	pFileName = splitPathAndName(pathName);
	if (strcmp(pFileName,".")==0 || strcmp(pFileName,"..")==0) 
		return (-1);	// invalid file name

	if (strlen(pathName)==0 || pathName==pFileName) {
		if (strlen(pathName)==0) dirFd = rootFd;
		else dirFd = currentDirFd;
	} 
	else dirFd = fs_open(pathName);	// open paren dir
	if (dirFd<0) return (-2);			// invalid path

	while (fs_searchDirByName(dirFd, pFileName, &dirRecord, found)) {	
		found = 1;	// file name found, also used for search option
		if (inodeTable[dirRecord.inodeNo].mode>>12 == DIRECTORY &&
			inodeTable[dirRecord.inodeNo].linkCount>2) continue;
		// mark file deleted
		dirRecord.deleted=1;
		fs_seek(dirFd, fileDescriptors[dirFd].fp - sizeof(dirRecord_t));
		fs_write(dirFd, &dirRecord, sizeof(dirRecord_t));
		inodeTable[fileDescriptors[dirFd].inodeNo].linkCount--;
		
		// reduce link count
		pInode = &inodeTable[dirRecord.inodeNo];
		if (pInode->mode>>12 == DIRECTORY) pInode->linkCount = 0; // remove an empty dir with linkcount=2
		else pInode->linkCount--;	// remove a normal file
		if (pInode->linkCount>0) return 0;  // there are still hardlinks, no need to free file blocks
		
		// linkCount==0, free allocated blocks
		if (pInode->fileSize==0) numBlocks=0;
		else numBlocks = (pInode->fileSize-1)/BLOCKSIZE + 1;
		// free direct blocks
		for (i=0; (i<DIRECTBLOCKS)&&(i<numBlocks); i++) addFreeBlock(pInode->direct[i]);
		// free indirect blocks
		if (numBlocks>DIRECTBLOCKS) { // single indirect pointer is in use
			readBlock(pInode->singleIndirect, &indirectBlock);
			j = numBlocks - DIRECTBLOCKS;
			for (i=0; i<j; i++) addFreeBlock(indirectBlock[i]);
			addFreeBlock(pInode->singleIndirect);
		}
		
		// set other inode attributes to zero
		// memset(pInode, 0, sizeof(inode_t));
	}
	if (dirFd != currentDirFd && dirFd!=rootFd) fs_close(dirFd);

    if (found) return 0;
	else return (-3);	// no such file
}

/* read from an open file 
	fd - file descriptor
	buf - buffer to store data
	n - number of bytes to read
	return: number of bytes read, -1 on faults
*/
int fs_read(int fd, void *buf, int n)
{	uint16_t beginBlock, endBlock, noOfBlocks, blockIndex, i, tmp;
	unsigned char blockBuf[BLOCKSIZE];
    int bytes_read = -1;

    // file not open or size = 0
	if ((fileDescriptors[fd].state==0) || (FILESIZE(fd)==0))
		return bytes_read;	
	
	// file is open, size > 0
	beginBlock = fileDescriptors[fd].fp / BLOCKSIZE; 
	endBlock = (fileDescriptors[fd].fp + n - 1) / BLOCKSIZE; 
	noOfBlocks = (FILESIZE(fd)-1) / BLOCKSIZE + 1;
	
	// just read till the last block of file
	if (endBlock > noOfBlocks - 1) endBlock = noOfBlocks - 1;  
	
	// copy content from the 1st block
	blockIndex = getBlockIndex(fd, beginBlock);	// convert to block number
	readBlock(blockIndex, blockBuf);
	tmp = fileDescriptors[fd].fp % BLOCKSIZE; // offset of fp in the 1st block
	if (n<=BLOCKSIZE - tmp) {
		memcpy(buf, blockBuf + tmp, n);
		bytes_read = n;
		fileDescriptors[fd].fp += n;
		return (bytes_read);
	}
	memcpy(buf, blockBuf + tmp, BLOCKSIZE - tmp);
	buf += BLOCKSIZE - tmp;
	bytes_read = BLOCKSIZE - tmp;
	
	// copy content of middle blocks
	for (i=beginBlock+1; i<endBlock; i++) {
		blockIndex = getBlockIndex(fd, i);	// get next block
		readBlock(blockIndex, blockBuf);
		memcpy(buf, blockBuf, BLOCKSIZE);
		buf += BLOCKSIZE;
		bytes_read += BLOCKSIZE;
	}
	
	// copy content from the last block
	if (endBlock > beginBlock) {
		blockIndex = getBlockIndex(fd, endBlock);	// get the last block
		readBlock(blockIndex, blockBuf);
		memcpy(buf, blockBuf, (fileDescriptors[fd].fp + n) % BLOCKSIZE);
		bytes_read += (fileDescriptors[fd].fp + n) % BLOCKSIZE;
	}
	if (fileDescriptors[fd].fp + n <= inodeTable[fileDescriptors[fd].inodeNo].fileSize)
		fileDescriptors[fd].fp += n;
	else fileDescriptors[fd].fp = inodeTable[fileDescriptors[fd].inodeNo].fileSize;

    return (bytes_read);

}

/* write to an open file 
	fd - file descriptor
	buf - data to write
	n - number of bytes to write
	return: number of bytes written, <0 on faults
*/
int fs_write(int fd, void *buf, int n)
{	uint16_t beginBlock, endBlock, noOfBlocks, blockIndex; 
	int i, j, tmp, blockNo;
	unsigned char blockBuf[BLOCKSIZE];
    int bytes_written = -1;

    // write your code
	if (fileDescriptors[fd].state) { // make sure the file is open
		beginBlock = fileDescriptors[fd].fp / BLOCKSIZE; 
		endBlock = (fileDescriptors[fd].fp + n - 1) / BLOCKSIZE; 

		if (FILESIZE(fd) == 0) noOfBlocks = 0;
		else noOfBlocks = (FILESIZE(fd) - 1) / BLOCKSIZE + 1;
		
		// first check if there is enough disk space
		if (fs_diskFreeBlocks() < (endBlock + 1 - noOfBlocks)) {
			fprintf(stderr, "Out of disk space\n");
			return (-1); 
		}
		// check if file size exceeds max limit
		if (endBlock+1 > DIRECTBLOCKS+MAXINDIRECTBLOCKS) {
			fprintf(stderr, "File exceeds max size\n");
			return (-2); 
		}
		
		// allocate new blank blocks to the file 
		for (i=noOfBlocks; i<endBlock+1; i++) {
			blockNo = getFreeBlock();
			if (blockNo < 0) {
				fprintf(stderr, "Out of disk space.\n");
				return(-1);
			}
			
			if (i<DIRECTBLOCKS) inodeTable[fileDescriptors[fd].inodeNo].direct[i] = blockNo;
			else { // add to indirect table
				j = i - DIRECTBLOCKS;
				if (j==0) { // first indirect block
					tmp = getFreeBlock(); // for indirect table
					if (blockNo < 0) {
						fprintf(stderr, "Out of disk space.\n");
						return(-1);
					}
					inodeTable[fileDescriptors[fd].inodeNo].singleIndirect = tmp;
				}
				fileDescriptors[fd].indirectTable[j] = blockNo;
			}
		}
		
		// write to the 1st block
		blockIndex = getBlockIndex(fd, beginBlock);	// convert to block number
		readBlock(blockIndex, blockBuf);			// read block content
		tmp = fileDescriptors[fd].fp % BLOCKSIZE; 	// offset of fp in the 1st block
		// update block content
		if (BLOCKSIZE - tmp >= n) { 
			memcpy(blockBuf + tmp, buf, n);
			buf += n;	
			bytes_written = n;
		}
		else {
			memcpy(blockBuf + tmp, buf, BLOCKSIZE - tmp);
			buf += BLOCKSIZE - tmp;
			bytes_written = BLOCKSIZE - tmp;
		}
		writeBlock(blockIndex, blockBuf); 			// write back the change to disk
		
		// write to middle blocks
		for (i=beginBlock+1; i<endBlock; i++) {
			memcpy(blockBuf, buf, BLOCKSIZE);
			blockIndex = getBlockIndex(fd, i);	// get next block
			writeBlock(blockIndex, blockBuf);
			buf += BLOCKSIZE;
			bytes_written += BLOCKSIZE;
		}
		
		// write to the last block
		if (endBlock > beginBlock) {
			blockIndex = getBlockIndex(fd, endBlock);	// get the last block
			readBlock(blockIndex, blockBuf);		// read block content
			memcpy(blockBuf, buf, (fileDescriptors[fd].fp + n) % BLOCKSIZE); // update changes
			writeBlock(blockIndex, blockBuf);	// write back to disk
			bytes_written += (fileDescriptors[fd].fp + n) % BLOCKSIZE;
		}
	}
	if (fileDescriptors[fd].fp + n > inodeTable[fileDescriptors[fd].inodeNo].fileSize) {
		inodeTable[fileDescriptors[fd].inodeNo].fileSize = fileDescriptors[fd].fp + n;
		fileDescriptors[fd].state = 2;	// file size changed
	}
	fileDescriptors[fd].fp += n;

    return (bytes_written);
}


/* create a hard link 
	pSourceName - original file
	pTargetName - name of hard-linked file
	return: 0 on success, <0 on faults
*/
int fs_createHardLink(char *pSourceName, char *pTargetName)
{	dirRecord_t dirRec;
	int ret, dirFd;
	uint16_t parentInodeNo;
	char pathName[MAXPATHNAME], *pFileName;
	
	strcpy(pathName, pSourceName); 
	if (fs_isDir(pathName)) return(-2);	// cannot hard link to a dir
	
	ret = path2inode(pathName, &dirRec, &parentInodeNo);

	if (ret<0) return(-1);					// source file does not exist;
	inodeTable[dirRec.inodeNo].linkCount++;	// increase hard link count
	
	strcpy(pathName, pTargetName);
	pFileName = splitPathAndName(pathName);
	strcpy(dirRec.fileName, pFileName);	// replace with target name
	if (strlen(pathName)==0 || pathName==pFileName) {
		if (strlen(pathName)==0) {dirFd = rootFd; printf("xxxx\n");}
		else dirFd = currentDirFd;
	}
	else dirFd = fs_open(pathName); 	// open the parent dir
	fs_seek(dirFd, fs_fileSize(dirFd)); 			
	fs_write(dirFd, &dirRec, sizeof(dirRecord_t));   // append target file to dir
	inodeTable[fileDescriptors[dirFd].inodeNo].linkCount++;
	if (dirFd != currentDirFd && dirFd!=rootFd) fs_close(dirFd);
	
	return 0;
}

/* create a symbolic link 
	pSourceName - original file	
	pTargetName - name of symbolic link
	return: 0 on success, <0 on faults
*/
int fs_createSymLink(char *pSourceName, char *pTargetName)
{	dirRecord_t dirRec;
	int ret, tfd;
	uint16_t parentInodeNo, mode;
	char pathName[MAXPATHNAME];
	char absSourceName[MAXPATHNAME];	// absolute path to store in symlink

	// Build absolute path for the symlink target
	// If pSourceName already starts with '/', it is absolute; otherwise prepend currentDir
	if (pSourceName[0] == '/') {
		if (strlen(pSourceName) >= MAXPATHNAME) return(-5);
		strcpy(absSourceName, pSourceName);
	} else {
		// currentDir is empty string when at root, otherwise "/dir/subdir"
		if (strlen(currentDir) + 1 + strlen(pSourceName) + 1 >= MAXPATHNAME) return(-5);
		strcpy(absSourceName, currentDir);
		strcat(absSourceName, "/");
		strcat(absSourceName, pSourceName);
	}

	strcpy(pathName, absSourceName);

	ret = path2inode(pathName, &dirRec, &parentInodeNo);
	if (ret<0) return(-1);	// source file does not exist

	ret = fs_create(pTargetName);
	if (ret<0) return(-2);	// cannot create symlink file
	tfd = fs_open(pTargetName);
	if (tfd<0) return(-3);	// cannot open symlink file
	// Store the absolute path so the symlink works from any directory
	ret = fs_write(tfd, absSourceName, strlen(absSourceName));
	
	mode = inodeTable[fileDescriptors[tfd].inodeNo].mode;
	mode = (mode << 4)>>4;
	mode |= (SYMBOLICLINK<<12);
	inodeTable[fileDescriptors[tfd].inodeNo].mode = mode;
	fs_close(tfd);
	
	return 0;
}


/* 	search current directory for a given file name 
	pDirRecord points to the file entry found
	option: 0 search from the beginning / 1 search next
*/
int fs_searchDirByName(int dirFd, char *pFileName, dirRecord_t *pDirRecord, int option)
{	static int pos = 0;		// remember the last entry position
	int i, numEntries, ret=0;
	
	if (option==0) pos = 0;	// reset the search from beginning
	numEntries = inodeTable[fileDescriptors[dirFd].inodeNo].fileSize / sizeof(dirRecord_t);
	fs_seek(dirFd, pos*sizeof(dirRecord_t));
	for (i=pos; i<numEntries; i++) {
		fs_read(dirFd, pDirRecord, sizeof(dirRecord_t));
		if ((pDirRecord->deleted!=1)&& wildCmp(pFileName, pDirRecord->fileName)) {
			ret = 1; // found
			break;
		}
	}
	if (i<numEntries) pos = i+1;
	else pos = i;
	return(ret);	// not found
}

/* set current dir to a path name, starting from current dir
	return 0: no fault, -1 path invalid, -2 max open file limit
*/
int fs_setCurrentDir(char *pPath)
{	dirRecord_t dirRecord;
	uint16_t parentInodeNo;
	char pathName[MAXPATHNAME], linkName[MAXPATHNAME];
	int ret, dirFd;
	char buf[BLOCKSIZE];
	
	if (strlen(pPath)>=MAXPATHNAME) return(-5);		// path too long
	strcpy(pathName, pPath);
	
	dirFd = currentDirFd;		// remember current dir

	if (strlen(pathName)==0) { 
		currentDirFd = rootFd; 
		return 0;
	}
	if (pathName[0]=='/') currentDirFd = rootFd;	// absolute path
	ret = path2inode(pathName, &dirRecord, &parentInodeNo);
	if (ret<0) {

		currentDirFd = dirFd;	// failed, restore previous current dir
		return (-1);			// invalid path
	}
	if (inodeTable[dirRecord.inodeNo].mode >> 12 != DIRECTORY) {
		if (inodeTable[dirRecord.inodeNo].mode >> 12 == SYMBOLICLINK) {
			readBlock(inodeTable[dirRecord.inodeNo].direct[0], buf);
			sscanf(buf, "%s", linkName);
			ret = path2inode(linkName, &dirRecord, &parentInodeNo);
			if (ret<0) return (-3);	// invalid link name
			if (inodeTable[dirRecord.inodeNo].mode >> 12 != DIRECTORY)
				return(-2); // link name is not a dir
		}
		else return(-2); // not a dir
	}
	
	if (dirRecord.inodeNo != superBlock.rootInodeNo)
		dirFd = openFileByInode(dirRecord.inodeNo);
	else dirFd = rootFd;
	if (currentDirFd != rootFd) fs_close(currentDirFd);
	
	currentDirFd = dirFd;	// set new current dir

	return 0;
}


/* make a directory 
	retur: 0 on success, <0 on faults
*/
int fs_makeDir(char *pPathName)
{	dirRecord_t dirRecord;
	int inodeNo, ret;
	uint16_t parentInodeNo, mode;
	char *pFileName, pathName[MAXPATHNAME];
	int dirFd, newDirFd;
	
	if (strlen(pPathName)>=MAXPATHNAME) return(-5);
	strcpy(pathName, pPathName);
		
    // search dir if filename already exists
    ret = path2inode(pathName, &dirRecord, &parentInodeNo);
    if (ret==0) return(-1);			// file already exists
	if (ret==-1) return(-2);		// or invalid path

	pFileName = splitPathAndName(pathName);

	// open parent dir
    if (strlen(pathName)==0 || pathName==pFileName) {
		if (strlen(pathName)==0) dirFd = rootFd;
		else dirFd = currentDirFd;
	}
	else {
		dirFd = openFileByInode(parentInodeNo);
		if (dirFd<0) return(-3);	// path is invalid
	}

	// prepare dir record entry
	strcpy(dirRecord.fileName, pFileName);
	dirRecord.deleted = 0;
	inodeNo = getFreeInode();
	if (inodeNo<0) {
		fprintf(stderr, "Inode table full\n");
		return(-4);	// out of inodes
	}
	dirRecord.inodeNo = inodeNo;

	// write to the parent dir
	fs_seek(dirFd, FILESIZE(dirFd));
	fs_write(dirFd, &dirRecord, sizeof(dirRecord_t));
	if (dirFd != currentDirFd && dirFd!=rootFd) fs_close(dirFd);

	// update inode table
	inodeTable[parentInodeNo].linkCount++;
	mode = inodeTable[inodeNo].mode;
	mode = (mode << 4)>>4;
	mode |= (DIRECTORY<<12);
	inodeTable[inodeNo].mode = mode;
	inodeTable[inodeNo].linkCount = 2;
	
	// write 2 default files . and .. to the new dir
	newDirFd = openFileByInode(dirRecord.inodeNo);
	
	strcpy(dirRecord.fileName,".");
	dirRecord.inodeNo = inodeNo;
	dirRecord.deleted = 0;
	fs_write(newDirFd, &dirRecord, sizeof(dirRecord_t));
	
	strcpy(dirRecord.fileName,"..");
	dirRecord.inodeNo = parentInodeNo;
	dirRecord.deleted = 0;
	fs_write(newDirFd, &dirRecord, sizeof(dirRecord_t));
	
	fs_close(newDirFd);
	
	return (0);
 }
 
 /* return number of files in the directory (>=2), 0 if not a dir */
 int fs_isDir(char *pPathName)
 {	int fd;
 
	fd = fs_open(pPathName);
	if (fd<0) return 0;
	if (inodeTable[fileDescriptors[fd].inodeNo].mode >> 12 == DIRECTORY &&
		inodeTable[fileDescriptors[fd].inodeNo].linkCount>0)
		return (inodeTable[fileDescriptors[fd].inodeNo].linkCount);
	return 0;
 }

/*	return number of free blocks */
int fs_diskFreeBlocks(void)
{		
	return superBlock.freeBlockCount;
}

/* print content of current directory with optional path and wildcards*/
int fs_printDir (char *pPathName)
{	int n, i, dirFd, count=0;
	dirRecord_t dirRecord;
	char *pPattern;
	char buf[BLOCKSIZE], pathName[MAXPATHNAME], linkName[MAXPATHNAME];
	
	memset(pathName, 0, MAXPATHNAME-1);
	if (pPathName !=NULL) strcpy(pathName, pPathName); 
	else strcat(pathName, "*");

	if (strstr(pathName,"*")==NULL && strstr(pathName,"?")==NULL) {
		if (pathName[strlen(pathName)-1]=='/') strcat(pathName,"*");
		else strcat(pathName,"/*");
	}

	pPattern = splitPathAndName(pathName);

	if (strlen(pathName)==0 || pathName==pPattern) {
		if (strlen(pathName)==0) dirFd = rootFd;
		else dirFd = currentDirFd;
	} 
	else {
		dirFd = fs_open(pathName);
		if (dirFd<0) return(-1);	// invalid path
	}
	
	n = inodeTable[fileDescriptors[dirFd].inodeNo].fileSize / sizeof(dirRecord_t);
	fs_seek(dirFd, 0);
	printf("LC	Size	Inode	File name\n");
	printf("--	------	-----	----------------\n");
	for (i=0; i<n; i++) {
		fs_read(dirFd, &dirRecord, sizeof(dirRecord_t));
		if ( (dirRecord.deleted!=1)&& wildCmp(pPattern, dirRecord.fileName) ) {
			printf("%2d\t%6d\t%04Xh\t%s", inodeTable[dirRecord.inodeNo].linkCount,
			inodeTable[dirRecord.inodeNo].fileSize, dirRecord.inodeNo,
			dirRecord.fileName);
			if (inodeTable[dirRecord.inodeNo].mode>>12==SYMBOLICLINK) {
				readBlock(inodeTable[dirRecord.inodeNo].direct[0], buf);
				sscanf(buf, "%s", linkName);
				printf("->%s", linkName);
			} 
			else if (inodeTable[dirRecord.inodeNo].mode>>12==DIRECTORY)
				printf("\t<DIR>");
			printf("\n");
			count++;
		}
	}
	printf("----------------------------------------\n");
	if (superBlock.freeBlockCount*BLOCKSIZE < 1024)
		printf("Total %d.\tFree space %dB\n", count, superBlock.freeBlockCount*BLOCKSIZE);
	else
		printf("Total %d.\tFree space %dKB\n", count, superBlock.freeBlockCount*BLOCKSIZE/1024);
	if (dirFd != currentDirFd && dirFd!=rootFd) fs_close(dirFd);
	return 0;
}

/* print inode table, only non-zero inodes are displayed */
void fs_printInodeTable()
{	int i, j;

	printf("I-node Table:\n");
	printf("Inode  Mode  LC  Size    IndPtr  DirPtr\n");
	printf("-----  ----  --  ------  ------  ------\n");
	for (i=0; i< numInodes; i++) {
		if (inodeTable[i].linkCount>0) { // print only occupied inode
			printf("%5d  %4d  %2d  %6d  %6d  ", i, inodeTable[i].mode>>12,
				inodeTable[i].linkCount, inodeTable[i].fileSize, 
				inodeTable[i].singleIndirect);
			for (j=0; j<DIRECTBLOCKS; j++) printf("%d, ", inodeTable[i].direct[j]);
			printf("\b\b \n");
		}
	} 
	return;
}


/* print content of an inode */
void fs_printInode(uint16_t inodeNo)
{	inode_t *pInode;
	uint16_t buf[BLOCKSIZE/sizeof(uint16_t)];
	int i;

	if (inodeNo>numInodes) {
		fprintf(stderr, "Inode %d is out of range\n", inodeNo);
		return;
	}
	pInode = &inodeTable[inodeNo];
	printf("I-node %d:\n", inodeNo);
	printf("Mode = "B2b_t" "B2b_t" (binary)\n", B2b(pInode->mode >> 8), B2b(pInode->mode));
	printf("Link count = %d\n", pInode->linkCount);
	printf("File size = %d bytes\n", pInode->fileSize);
	
	printf("Direct pointers = ");
	for (i=0; i<DIRECTBLOCKS; i++) printf("%d, ", pInode->direct[i]);
	printf("\b\b \n");
	
	printf("Indirect pointers = %d: ", pInode->singleIndirect);
	if (pInode->singleIndirect==0) return;
	readBlock(pInode->singleIndirect, (char*)buf);
	for (i=0; i<BLOCKSIZE/sizeof(uint16_t)-1; i++) 
		if (buf[i]>0) printf("%d, ", buf[i]);
	printf("\b\b \n");
	
	return;
}

/* print inode table */
void fs_printSuperBlock()
{	
	printf("Super block:\n");
	printf("Inode block = %d\n", superBlock.inodeBlock);
	printf("Start of data block = %d\n", superBlock.dataBlock);
	printf("First free block = %d\n", superBlock.freeBlockList);
	printf("Free blocks = %d\n", superBlock.freeBlockCount);
	printf("Block size = %d\n", superBlock.blockSize);
	printf("Root directory inode = %d\n", superBlock.rootInodeNo);
	return;
}


/* Supporting functions ---------------------------------------------------------------- */

/*	String compare with wildcards using recursive algo 
	Return 1 if found, 0 otherwise
*/

int wildCmp(char *pPattern, char *pString)
{
	if(*pPattern=='\0' && *pString=='\0')		
		return 1;
		
	if(*pPattern=='?' || *pPattern==*pString)	
		return wildCmp(pPattern+1,pString+1);
		
	if (*pPattern=='*') {
		if (strlen(pString)==0 && strlen(pPattern)>1) return 0;
		else return wildCmp(pPattern+1,pString) || wildCmp(pPattern,pString+1);	
	}
	return 0;
}

/*	String compare with wildcards using non-recursive algo (faster)
	Return 1 if found, 0 otherwise
*/
/* 
int wildCmp(char *pPattern,char *pString) 
{	char *cp = NULL, *mp = NULL;

	while ((*pString) && (*pPattern != '*')) {
		if ((*pPattern != *pString) && (*pPattern != '?')) {
		  return 0;
		}
		pPattern++;
		pString++;
	}

	while (*pString) {
		if (*pPattern == '*') {
			if (!*++pPattern) {
				return 1;
			}
			mp = pPattern;
			cp = pString+1;
		} 
		else if ((*pPattern == *pString) || (*pPattern == '?')) {
			pPattern++;
			pString++;
		} else {
			pPattern = mp;
			pString = cp++;
		}
	}

	while (*pPattern == '*') {
		pPattern++;
	}
	return !*pPattern;
}

 */


/* look in inode table for a free inode
	return inode number or <0 on faults
 */
int getFreeInode() 
{	
	for (int i=superBlock.rootInodeNo+1; i< numInodes; i++) {	// inode 0 reserved for root dir
		if (inodeTable[i].linkCount==0) {
			memset(&inodeTable[i], 0, sizeof(inode_t));
			inodeTable[i].linkCount = 1;	// mark as occupied once
			return(i);
		}
	}
	return(-1);
}


/* convert a file block sequence to physical data block */
int getBlockIndex(int fd, uint16_t blockSeq) 
{		
	if (blockSeq<DIRECTBLOCKS) 
		return(inodeTable[fileDescriptors[fd].inodeNo].direct[blockSeq]);
	else 
		return(fileDescriptors[fd].indirectTable[blockSeq-DIRECTBLOCKS]);
}

/* add a block to the free block list */
void addFreeBlock(uint16_t blockNo)
{	char buf[BLOCKSIZE];
	uint16_t *pNo;
	
	// insert to free block list
	memset(buf, 0, BLOCKSIZE);
	pNo = (uint16_t*) buf;
	*pNo = superBlock.freeBlockList;
	writeBlock(blockNo, buf);
	
	superBlock.freeBlockList = blockNo;	
	superBlock.freeBlockCount++;
}

/* allocate a free block, then remove from the free block list */
int getFreeBlock()
{	char buf[BLOCKSIZE];
	int ret=-1;
	
	if (superBlock.freeBlockList>0) {
		ret = superBlock.freeBlockList;					// allocate the 1st block
		readBlock(ret, buf);
		superBlock.freeBlockList = *((uint16_t*)buf);	// next block becomes first
		superBlock.freeBlockCount--;
	}
	
	return(ret);
	
}

/* 	search a data block of a directory by file name
	return inode number via pInodeNo
*/
int searchBlockByName(uint16_t blockNo, char *pFileName, dirRecord_t *pDirRecord)
{	char buf[BLOCKSIZE];
	dirRecord_t *pRecords;;
	int i;

	readBlock(blockNo, buf);
	pRecords = (dirRecord_t*) buf;
	for (i=0; i<BLOCKSIZE/sizeof(dirRecord_t); i++) {
		if (strcmp(pRecords[i].fileName, pFileName)==0 &&
			pRecords[i].deleted==0) break;
	}
	if (i<BLOCKSIZE/sizeof(dirRecord_t)) {
		memcpy(pDirRecord, &pRecords[i], sizeof(dirRecord_t));
		return 1;		// found
	}
	return 0;	// not found
}

/*	convert from path name to inode.
	return: 0 no error, -1 path not found, -2 file not found
*/
int path2inode(char *pPath, dirRecord_t *pDirRecord, uint16_t *pParentInodeNo)
{	inode_t *pInode;
	uint16_t indirectBlock[MAXINDIRECTBLOCKS];
	char pathName[MAXPATHNAME];
	char* argv[MAXPATHNAME];
	int argc=0, i, j, inodeNo, parentInodeNo, found=0;
	
	// split parameters
	strcpy(pathName, pPath);
	i=0;
	argv[i] = strtok(pathName, "/\r\n");
	while ((argv[i]!=NULL)&&(i<MAXPATHNAME-1)) {
		i++;
		argv[i] = strtok(NULL, "/\r\n");
	}
	argc = i;

	// start searching from root or current dir
	if (pPath[0]=='/') inodeNo = fileDescriptors[rootFd].inodeNo;
	else inodeNo = fileDescriptors[currentDirFd].inodeNo;
	parentInodeNo = inodeNo;	// in case the for loop is skipped over
	strcpy(pDirRecord->fileName,".");
	pDirRecord->inodeNo = inodeNo;
	pDirRecord->deleted = 0;

	for (i=0; i<argc; i++) {
		parentInodeNo = inodeNo;
		pInode = &inodeTable[inodeNo];
		for (j=0; j<DIRECTBLOCKS; j++) {	// search direct blocks
			if (j*BLOCKSIZE > pInode->fileSize) break;
			if (searchBlockByName(pInode->direct[j], argv[i], pDirRecord)) {
				found = 1;
				break;
			} else found=0;
		}

		// if not found, search indirect blocks
		if (!found) {
			readBlock(pInode->singleIndirect, indirectBlock);
			for (j=0; j<MAXINDIRECTBLOCKS; j++) {
				if (j*BLOCKSIZE > pInode->fileSize) break;
				if (searchBlockByName(indirectBlock[j], argv[i], pDirRecord)) {
					found = 1;
					break;
				} else found=0;
			}
		}
		if (!found) {
			*pParentInodeNo = parentInodeNo;
			if (i<argc-1) return (-1);	// path not found
			else return(-2);			// file not found
		}
		else inodeNo = pDirRecord->inodeNo;
	}
	*pParentInodeNo = parentInodeNo;	// return the parent inode number found

	return 0;
}

/* split a path and file name
	return pointer to filename. if no path is specified, then pPathName will 
		either be zero length or pPathName==pFileName
	note: the original string could be modified, i.e split into 2 strings
*/
char *splitPathAndName(char *pPathName)
{	int i;

	// split path and file name
	for (i=strlen(pPathName)-1; i>=0; i--) {
		if (pPathName[i]=='/') break;
	}
	if (i>=0) // split file name and path
		pPathName[i] = 0;
	return(pPathName + i + 1);
}

/* open file by inode 
	return file descriptor on success, <0 on faults
*/
static int openFileByInode(uint16_t inodeNo)
{	int fd = -1;

	if (inodeNo>=numInodes) return(fd); // inode out of range

	// set up file descriptor
	fileDescriptors[fileCount].state = 1;	// mark file as open
	fileDescriptors[fileCount].inodeNo = inodeNo;
	fileDescriptors[fileCount].fp = 0;
	
	// read indirect pointer table into RAM if any
	if (inodeTable[inodeNo].singleIndirect>0) // i.e != NULL
		readBlock(inodeTable[inodeNo].singleIndirect, fileDescriptors[fileCount].indirectTable);	// load indirect block into memory
	
	fd = fileCount;
	// move to the next available descriptor
	do fileCount = (fileCount+1)%MAXOPENFILES;
	while ( (fileDescriptors[fileCount].state!=0)&&(fileCount!=fd));
	if (fileCount==fd) return (-2);	// reaching max number of open files

    return (fd);
}

