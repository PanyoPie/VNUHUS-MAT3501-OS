File System (FS) using inodes
MAT3501 - Principles of Operating Systems, MIM - HUS

Compilation: run 'make'.
Run 'fsshell' and type 'help'.

Disk structure:
	Block 0: boot sector, left empty
	Block 1: super block (defined in fs.h)
	Block 2: inode table (defined in fs.h)
	Block 3: default 1st block of root dir

Super block:
	Inode 0 is reserved for root dir
	Root dir starts at data block 3
	Free blocks: chained together as a linked list, starting from block 4

Inodes:
	mode: 2 bytes, 4 most significant bits are used for file types (regular/directory/symlink)
	linkcount: count hardlinks in regular files, or number of entries in a directory/symlink
	file size: keeps file size
	direct pointer: points to data blocks; number of direct pointers is defined by DIRECTBLOCKS
	indirect pointers: use 1 extra data block allocated to the file when size grows of of direct block limit

Directory entry:
	file name
	inode
	deleted (to mark a file as deleted)