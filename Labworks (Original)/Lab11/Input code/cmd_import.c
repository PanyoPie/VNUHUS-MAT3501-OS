/* ----------------------------------------------------------------------------------------
	Create a new FS file from an existing normal file.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Read an existing file from the normal source and create a new file in our FS.
------------------------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "fs.h"

extern superBlock_t superBlock;
extern fileDescriptor_t fileDescriptors[MAXOPENFILES];

int main(int argc, char *argv[])
{	char diskname[128];
	char filename[128];
	char buf[BLOCKSIZE];
	int fs_fd, size, freeSize, n, len; 
	FILE *fp;
		
	if (argc != 2) {
		printf ("usage: %s <sourcefile>\n",argv[0]); 
		exit (-1);
	}
	
    if (strlen(argv[1])>MAXFILENAME) {
		fprintf(stderr,"Sorry, SFS only support file name upto %d characters.\n", MAXFILENAME);
		exit(-1);
	}
	strcpy (diskname, DISKNAME);
	strcpy (filename, argv[1]);
	
	if (fs_mount (diskname) != 0) {
		printf ("Could not mount disk %s\n", diskname); 
		exit (1); 
	}
	else printf ("Filesystem %s mounted\n", diskname); 

	
	// copy from source file into our file system
	printf("filename = %s\n", filename);
	fp = fopen(filename, "r");
	if (fp==NULL) { fprintf(stderr,"Source file does not exist\n"); return(-2);}
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	freeSize = fs_diskFreeBlocks()*BLOCKSIZE;
	if (freeSize < size) {
		printf("Not enough disk space! Free %d, request %d\n", freeSize, size);
		fclose(fp);
		return (-1);
	} 
	fseek(fp, 0L, SEEK_SET);
		
	fs_fd = fs_create(filename);
	fs_fd = fs_open(filename);
	if (fs_fd<0) { fprintf(stderr,"Cannot open file %s in FS\n",filename); return(-2);}
	
	while (!feof(fp)) {
		n = fread(buf, BLOCKSIZE, 1, fp);
		if (n==1) len = BLOCKSIZE;
		else len = size % BLOCKSIZE;
		fs_write(fs_fd, buf, len);
	}
	
	if (fs_close(fs_fd)<0) printf("Fail to close fsf file\n");
	fclose(fp);
	
	fs_printDir();
	fs_printFat();
			
	fs_umount(); 
	printf ("Filesystem %s unmounted\n", diskname);

	return (0);		
}
