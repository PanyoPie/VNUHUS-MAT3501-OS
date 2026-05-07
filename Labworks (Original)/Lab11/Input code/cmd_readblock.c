/* ----------------------------------------------------------------------------------------
	Read a FS disk block.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Call to readBlock() and display the block content in both hexa and text format.
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

#define LINEWIDTH	16

int main(int argc, char *argv[])
{
	char diskname[128]; 
	unsigned char buf[BLOCKSIZE], text[LINEWIDTH+1];
	int i, n, tmp; 
		
	if (argc != 2) {
		printf ("usage: %s <block>\n", argv[0]); 
		exit (-1);
	}
	
       
	strcpy (diskname, DISKNAME);
	n = atoi(argv[1]);
		
	if (fs_mount (diskname) != 0) {
		printf ("Could not mount disk %s\n", diskname); 
		exit (-2); 
	}
	else printf ("Filesystem %s mounted\n", diskname); 

	// read block content
	tmp = readBlock(n, buf);
	if (tmp<0) { fprintf(stderr, "Could not read block\n"); return (-3);}
	memset(text,0,LINEWIDTH); // set the string empty
	for (i=0; i<=LINEWIDTH; i++) text[i]=0;
	for (i=0; i<BLOCKSIZE; i++) {
		if (i%LINEWIDTH==0) { printf("\t%s\n",text); printf("%04X\t",i);}
		printf("%02X ", buf[i]);
		if (buf[i]<32) text[i%LINEWIDTH]='.'; else text[i%LINEWIDTH]=buf[i];
	}
	printf("\t%s\n", text);
		
	fs_umount();
	printf ("Filesystem %s unmounted\n", diskname);	

	return (0);		
}
