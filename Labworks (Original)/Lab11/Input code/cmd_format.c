/* ----------------------------------------------------------------------------------------
	Format a FS disk.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Prepare boot, super, fat blocks to make FS ready to be used.
		FS disk structure:
		Block 0: boot block - left blank
		Block 1: super block - contains SFS meta data
		Block 2: FAT block - 16 bit FAT entries
		Block 3: start of root directory (sub-directories are not supported)
		Block 4 onwards: data blocks.
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "fs.h"

int main (void)
{	FILE* fp;
	char vdiskname [128]; 
	unsigned int size; 
	int ret;
	
	strcpy (vdiskname, DISKNAME); 
	fp = fopen(vdiskname, "r"); 
	if (fp==NULL) {
		printf("No disk. Please run createdisk first\n");
	}
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fclose(fp); 

	ret = fs_format(vdiskname, size); 
	if (ret<0) {
		printf("Could not format disk. Error = %d\n", ret);
		return (-1);
	}
	printf("Format complete. Disk size = %dKB, %d blocks, block size = %dB\n", size/1024, size/BLOCKSIZE, BLOCKSIZE);	
	return (0); 
}
