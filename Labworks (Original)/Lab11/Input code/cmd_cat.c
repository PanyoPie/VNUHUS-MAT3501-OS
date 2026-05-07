/* ----------------------------------------------------------------------------------------
	Read content of an FS file.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: Use FS file functions to read content of a given file name.
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
	unsigned char buf[BLOCKSIZE];
	int fs_fd, n, i; 
		
	if (argc != 2) {
		printf ("usage: %s <filename>\n", argv[0]); 
		exit (-1);
	}
	
	strcpy (diskname, DISKNAME);
	strcpy (filename, argv[1]);
		
	if (fs_mount(diskname) != 0) {
		printf ("Could not mount disk %s\n", diskname); 
		exit (-2); 
	}
	// printf ("Filesystem %s mounted\n", diskname); 

		
	// 1. Mở tệp filename dùng fs_open(), nếu lỗi thì trả lại -3
	fs_fd = fs_open(filename);
	if (fs_fd<0) {
		printf("Lỗi mở tệp\n"); return(-3);
	}

	// 2. Lặp: đọc từng khối kích thước BLOCKSIZE vào buf[], dùng fs_read() và fs_eof()
	//		Lặp in ra từng ký tự trong buf[i];
	while (!fs_eof(fs_fd)) {
		n = fs_read(fs_fd, buf, BLOCKSIZE);
		for (i=0; i<n; i++) printf("%c",buf[i]);
	}

	printf("\n");	// print an extra newline at the end of file

	// 3. Đóng tệp
	fs_close(fs_fd);
	
	fs_umount(); 
	// printf ("Filesystem %s unmounted\n", diskname);

	return (0);		
}
