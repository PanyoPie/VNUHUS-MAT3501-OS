/* ----------------------------------------------------------------------------------------
	A simple FAT File System.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Provide basic functions to access SFS file system. Public FS functions start 
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

// Global Variables
extern char disk_name[128];   // name of virtual disk file
extern int  disk_size;        // size in bytes - a power of 2
extern int  disk_fd;          // disk file handle
extern int  disk_blockCount;  // block count on disk

superBlock_t superBlock;		// content of super block
uint16_t fat[MAXBLOCKCOUNT*sizeof(uint16_t)];	// 16-bit File Allocation Table 
fileDescriptor_t fileDescriptors[MAXOPENFILES]; // list of fs file handles
int fileCount;					// current available file descriptor
int rootFd;						// file handle of root dir

// supporting functions for sfs module only
static int searchDirByName(char *pFileName, dirRecord_t *pDirRecord);
// static int searchDirByIndex(uint16_t idx, dirRecord_t *pDirRecord);
static int getFatIndex(int fd, uint16_t blockSeq);
static int allocateNewBlock(int fd);



/* format disk of size dsize */
int fs_format(char *vdisk, int dsize)
{	int i, ret;

    strcpy (disk_name, vdisk);
    disk_size = dsize;
    disk_blockCount = disk_size / BLOCKSIZE;

    disk_fd = open (disk_name, O_RDWR);
    if (disk_fd == -1) return (-1);		// Cannot open disk
	
    // format super block
    superBlock.fatBlock=2;
	superBlock.rootDir=3;
	superBlock.rootDirSize=0;
	superBlock.dataBlock=3;
	superBlock.blockSize=BLOCKSIZE;
	superBlock.blockCount=disk_size / BLOCKSIZE;
	ret = writeBlock(1, &superBlock);
	if (ret<0) return(-2);
	
	// format FAT
	for (i=0; i<MAXBLOCKCOUNT; i++) fat[i]=0;
	ret = writeBlock(superBlock.fatBlock, &fat);
	if (ret<0) return(-2);
	
    fsync (disk_fd);
    close (disk_fd);
	
    return (0);
}


/* Mount the file system and make it ready: load super block, inode table into RAM,
   open root directory.
*/
int fs_mount (char *vdisk)
{
    struct stat finfo;

    strcpy (disk_name, vdisk);
    disk_fd = open (disk_name, O_RDWR);
    if (disk_fd == -1) {
        printf ("fs_mount: disk open error %s\n", disk_name);
        exit(1);
    }

    fstat (disk_fd, &finfo);
    disk_size = (int) finfo.st_size;
    disk_blockCount = disk_size / BLOCKSIZE;

    readBlock(1, &superBlock);				// load superblock to RAM
	readBlock(superBlock.fatBlock, &fat);	// load FAT to RAM
		
	fileDescriptors[0].state = 1;
	fileDescriptors[0].fatIndex = 3;
	fileDescriptors[0].lastFatIndex = 3;
	fileDescriptors[0].fp = 0;
	fileDescriptors[0].fileSize = superBlock.rootDirSize;
	rootFd = 0;
	fileCount++;

    /*printf ("fs_mount: mounting %s, size=%dKB, free=%dKB\n", disk_name,
        (int) finfo.st_size/1024, fs_diskFreeBlocks()*BLOCKSIZE/1024);
	*/
	
    return (0);
}

/* unmount the file system: write changes in superblock, inode table back to disk
*/
int fs_umount()
{
    superBlock.rootDirSize = fileDescriptors[0].fileSize;
	writeBlock(1, &superBlock);
	writeBlock(superBlock.fatBlock, &fat);	// update fat to disk

    fsync (disk_fd);
    close (disk_fd);
    return (0);
}


/* create a file in root directory with name filename */
int fs_create(char *filename)
{	dirRecord_t dirRecord;
    // search root dir if filename already exists
	if (!searchDirByName(filename, &dirRecord)) { // append if not yet exists
		//memcpy(dirRecord.fileName, filename, MAXFILENAME);
		strcpy(dirRecord.fileName, filename);
		dirRecord.fileSize = 0;
		dirRecord.fatIndex = 0xFFFF; // no blocks yet allocated 
		dirRecord.deleted = 0;
		fs_seek(rootFd, fileDescriptors[rootFd].fileSize);
		fs_write(rootFd, &dirRecord, sizeof(dirRecord));
	} 
	else printf("File %s already exists.\n",filename);
	
	return (0);
}


/* open file filename */
int fs_open(char *pFileName)
{	uint16_t i, j;
	dirRecord_t dirRecord;
	int index = -1;

    // write your code
	if (searchDirByName(pFileName, &dirRecord)) {
		strcpy(fileDescriptors[fileCount].fileName, pFileName);
		fileDescriptors[fileCount].state = 1;	// file is open
		fileDescriptors[fileCount].fatIndex = dirRecord.fatIndex;
		fileDescriptors[fileCount].fp = 0;
		fileDescriptors[fileCount].fileSize = dirRecord.fileSize;
		// find last fat node
		i = dirRecord.fatIndex;
		j=i;
		while (i!=0xFFFF) {
			j = i;
			i = fat[i];
		}
		index = fileCount;
		fileDescriptors[index].lastFatIndex = j;
		fileCount++;
	}
    return (index);
}

/* close an open file */
int fs_close (int fd)
{	dirRecord_t dirRecord;

    // write your code
	if (fileDescriptors[fd].state==2) { // file size has been changed
		// update file attribute
		if (!searchDirByName(fileDescriptors[fd].fileName, &dirRecord)) {
			printf("Khong tim thay ten tep\n");
			return (-1);  // cannot close sfs file
		}
		// update file size
		dirRecord.fileSize = fileDescriptors[fd].fileSize;
		dirRecord.fatIndex = fileDescriptors[fd].fatIndex;

		// go back to the beginning of the record in root directory
		fileDescriptors[rootFd].fp -= sizeof(dirRecord);
		fs_write(rootFd, &dirRecord, sizeof(dirRecord));
	}
	
	fileDescriptors[fd].state = 0; // closed
	

    return (0);
}

int fs_eof(int fd)
{
    return (fileDescriptors[fd].fp>=fileDescriptors[fd].fileSize);
}


/* seek to position offset in an open file */
int fs_seek(int fd, int offset)
{
    int position = -1;

    // write your code
	if (offset <= fileDescriptors[fd].fileSize) {
		fileDescriptors[fd].fp = offset;
		position = offset;
	}

    return (position);
}

/* get size of an open file */
int fs_fileSize (int fd)
{
    int size = -1;

    // write your code
	size = fileDescriptors[fd].fileSize;

    return (size);
}

/* Delete a file name 
   For the purpose of intuitivity, deleting a file will definitely erase its fat nodes.
   In real OS, the file block chain remains untouched in the fat table, and file blocks are 
   added to the free block list. 
   The file entry in directory is marked deleted, instead of being completely removed.
*/
int fs_delete(char *filename)
{	dirRecord_t dirRecord;
	int index, tmp;

    // write your code
	if (searchDirByName(filename, &dirRecord)) {
		// mark file deleted
		dirRecord.deleted=1;
		fs_seek(rootFd, fileDescriptors[rootFd].fp - sizeof(dirRecord_t));
		fs_write(rootFd, &dirRecord, sizeof(dirRecord_t));
		
		// erase fat nodes
		index = dirRecord.fatIndex;
		while (fat[index]!=0xFFFF) {
			tmp = fat[index];
			fat[index] = 0;
			index = tmp;
		}
		fat[index] = 0;
		return 0;
	}
    else return (-1);
}

/* read from an open file */
int fs_read(int fd, void *buf, int n)
{	uint16_t beginBlock, endBlock, noOfBlocks, blockIndex, i, tmp;
	unsigned char blockBuf[BLOCKSIZE];
    int bytes_read = -1;

    // file not open or size = 0
	if ((fileDescriptors[fd].state==0) || (fileDescriptors[fd].fileSize==0))
		return bytes_read;	
	
	// file is open, size > 0
	beginBlock = fileDescriptors[fd].fp / BLOCKSIZE; 
	endBlock = (fileDescriptors[fd].fp + n - 1) / BLOCKSIZE; 
	noOfBlocks = (fileDescriptors[fd].fileSize-1) / BLOCKSIZE + 1;
	
	// just read till the last block of file
	if (endBlock > noOfBlocks - 1) endBlock = noOfBlocks - 1;  
	
	// copy content from the 1st block
	blockIndex = getFatIndex(fd, beginBlock);	// convert to block number
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
		blockIndex = fat[blockIndex];	// get next block
		readBlock(blockIndex, blockBuf);
		memcpy(buf, blockBuf, BLOCKSIZE);
		buf += BLOCKSIZE;
		bytes_read += BLOCKSIZE;
	}
	
	// copy content from the last block
	if (endBlock > beginBlock) {
		blockIndex = fat[blockIndex];	// get next block
		readBlock(blockIndex, blockBuf);
		memcpy(buf, blockBuf, (fileDescriptors[fd].fp + n) % BLOCKSIZE);
		bytes_read += (fileDescriptors[fd].fp + n) % BLOCKSIZE;
	}
	if (fileDescriptors[fd].fp + n <= fileDescriptors[fd].fileSize)
		fileDescriptors[fd].fp += n;
	else fileDescriptors[fd].fp = fileDescriptors[fd].fileSize;

    return (bytes_read);

}

/* write to an open file */
int fs_write(int fd, void *buf, int n)
{	uint16_t beginBlock, endBlock, noOfBlocks, blockIndex; 
	int i, tmp;
	unsigned char blockBuf[BLOCKSIZE];
    int bytes_written = -1;

    // write your code
	if (fileDescriptors[fd].state) { // make sure the file is open
		beginBlock = fileDescriptors[fd].fp / BLOCKSIZE; 
		endBlock = (fileDescriptors[fd].fp + n - 1) / BLOCKSIZE; 

		if (fileDescriptors[fd].fileSize == 0) noOfBlocks = 0;
		else noOfBlocks = (fileDescriptors[fd].fileSize - 1) / BLOCKSIZE + 1;
		// allocate new blank blocks as neccessary
		if (fs_diskFreeBlocks() < (endBlock + 1 - noOfBlocks)) {
			fprintf(stderr, "Out of disk space\n");
			return (-1); 
		}
		
		for (i=noOfBlocks; i<endBlock+1; i++) {
			tmp = allocateNewBlock(fd);
			if (tmp < 0) {
				fprintf(stderr, "Out of disk space.\n");
				return(-1);
			};
		}
		
		// write to the 1st block
		blockIndex = getFatIndex(fd, beginBlock);	// convert to block number
		readBlock(blockIndex, blockBuf);				// read block content
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
			blockIndex = fat[blockIndex];	// get next block
			writeBlock(blockIndex, blockBuf);
			buf += BLOCKSIZE;
			bytes_written += BLOCKSIZE;
		}
		
		// write to the last block
		if (endBlock > beginBlock) {
			blockIndex = fat[blockIndex];	// get next block
			readBlock(blockIndex, blockBuf);	// read block content
			memcpy(blockBuf, buf, (fileDescriptors[fd].fp + n) % BLOCKSIZE); // update changes
			writeBlock(blockIndex, blockBuf);	// write back to disk
			bytes_written += (fileDescriptors[fd].fp + n) % BLOCKSIZE;
		}
	}
	if (fileDescriptors[fd].fp + n > fileDescriptors[fd].fileSize) {
		fileDescriptors[fd].fileSize = fileDescriptors[fd].fp + n;
		fileDescriptors[fd].state = 2; // file size has been changed
	}
	fileDescriptors[fd].fp += n;

    return (bytes_written);
}


/* print root directory */
void fs_printDir ()
{	int n, i, count=0;
	dirRecord_t dirRecord;

    // write your code
	n = fileDescriptors[rootFd].fileSize / sizeof(dirRecord_t);
	fs_seek(rootFd, 0);
	printf("Root directory:\n");
	printf("File name       	Size  	FatIdx\n");
	printf("----------------	------	------\n");
	for (i=0; i<n; i++) {
		fs_read(rootFd, &dirRecord, sizeof(dirRecord_t));
		if (dirRecord.deleted!=1) {
			printf("%16s	%06d	%04Xh\n", dirRecord.fileName, dirRecord.fileSize, dirRecord.fatIndex);
			count++;
		}
	}
	printf("--------------------------------------\n");
	n = fs_diskFreeBlocks();
	if (n*BLOCKSIZE < 1024)
		printf("Total %d. Free space %dB\n", count, n*BLOCKSIZE);
	else
		printf("Total %d. Free space %dKB\n", count, n*BLOCKSIZE/1024);
}

/* print fat directory */
void fs_printFat()
{	int i;

    // write your code
	printf("File Allocation Table:\n    ");
	for (i=0; i< 16; i++) {
		printf("  %02X ", i);
	} 
	printf("\n    ");
	for (i=0; i< 16; i++) {
		printf("---- ");
	}
	for (i=0; i< disk_blockCount; i++) {
		if (i % 16 ==0) printf("\n%02X: ",i);
		printf("%04X ", fat[i]);
	}
	printf("\n\n");
	return;
}

int fs_diskFreeBlocks(void)
{	int i, count=0;
	for (i=superBlock.dataBlock; i<disk_blockCount; i++) {
		if (fat[i]==0) count++;
	}
	return count;
}


/* Supporting functions ---------------------------------------------------------------- */

/* search root directory for a given file name */
int searchDirByName(char *pFileName, dirRecord_t *pDirRecord)
{	int i, c;
		
    // write your code
	fs_seek(rootFd, 0);
	c = fileDescriptors[rootFd].fileSize / sizeof(dirRecord_t);
	for (i=0; i<c; i++) {
		fs_read(rootFd, pDirRecord, sizeof(dirRecord_t));
		if ((pDirRecord->deleted!=1)&&(strcmp(pFileName, pDirRecord->fileName) == 0))
			return(1); // found
	}
	return(0);	// not found
}


/* convert a file block sequence to fat node */
int getFatIndex(int fd, uint16_t blockSeq) 
{	uint16_t idx, i=0;
	
	idx = fileDescriptors[fd].fatIndex;	// get the 1st block index
	while ((idx != 0xFFFF) &&(i<blockSeq)) {
		i++;
		idx = fat[idx];
	}
	return(idx);
}

/* find a new block, add it to the file fat chain */
int allocateNewBlock(int fd)	
{	int i, found=0;
	uint16_t idx;

	// look for an unused block
	for (i=superBlock.dataBlock; i<disk_blockCount; i++) {
		if (fat[i] == 0) {
			found = 1;
			break;
		}
	}
	if (found) {
		if (fileDescriptors[fd].lastFatIndex == 0xFFFF) {
			// the 1st block of file
			fileDescriptors[fd].fatIndex = i;
			fileDescriptors[fd].lastFatIndex = i;
		}
		else {
			// attach to the end of file block chain
			idx = fileDescriptors[fd].lastFatIndex;
			fat[idx] = i;
			fileDescriptors[fd].lastFatIndex = i;
		}
		fat[i] = 0xFFFF;
		return(0);
	}
	else return(-1);
}



