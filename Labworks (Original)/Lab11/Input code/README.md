A simple File System (FS) using FAT16
MAT3501 - Principles of Operating Systems - MIM HUS

Compilation: run 'make'.

Commands:
1) cmd_makedisk <power of 2 disk size>
	example: to create a disk of size 2^20B = 1MB, run "cmd_makedisk 20"
2) cmd_format
	format the disk with empty root dir and empty fat table
3) cmd_import <sourcefile>
	create a sfs file from <sourcefile>
3) cmd_cat <sfs file name>
	read content of a file in sfs
4) cmd_rm <sfs file name>
	delete a file in sfs
5) cmd_readblock <block number>
	read content of a sfs disk block
6) cmd_printfat
	print FAT table
7) cmd_dir
	print content of the root directory