/* ----------------------------------------------------------------------------------------
	Print FS File Allocation Table.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Call to fs_printFat() to print FAT entries.
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"

int main(void) 
{	char diskname[128]; 
	
	strcpy (diskname, DISKNAME);
	if (fs_mount (diskname) != 0) {
		printf ("Could not mount disk %s\n", diskname); 
		exit(1); 
	}
	else printf ("Filesystem %s mounted\n", diskname); 
	
	fs_printFat();
	
	fs_umount(); 
	printf ("Filesystem %s unmounted\n", diskname); 
	exit(0);
}