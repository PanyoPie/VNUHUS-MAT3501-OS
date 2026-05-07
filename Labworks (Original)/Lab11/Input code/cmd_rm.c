/* ----------------------------------------------------------------------------------------
	Delete a file from FS disk.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Given a SFS file name, delete it from our FS.
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

int main(int argc, char *argv[])
{
	char diskname[128]; 
	char filename[128]; 
		
	if (argc != 2) {
		printf ("usage: %s <filename>\n", argv[0]); 
		exit (-1);
	}
	
       
	strcpy (diskname, DISKNAME);
	strcpy (filename, argv[1]);
		
	if (fs_mount (diskname) != 0) {
		printf ("Could not mount disk %s\n", diskname); 
		exit (-2); 
	}
	else printf ("Filesystem %s mounted\n", diskname); 

	fs_delete(filename);
	
	fs_printDir();
	fs_printFat();
	
	fs_umount(); 
	printf ("Filesystem %s unmounted\n", diskname);

	return (0);		
}
