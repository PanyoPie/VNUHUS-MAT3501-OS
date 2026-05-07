File System (FS) using inodes
MAT3501 - Principles of Operating System, MIM - HUS

Compilation: run 'make'.
Run 'fsshell' and type 'help'.

What's new in version 2:
	Free blocks are no longer chained together as a link list. Free blocks are managed
		by a linked-list of blocks that keeps the list of free blocks (as in the lecture).
	New commands: badformat, checkdisk, undelete

Disk structure:
	Block 0: boot sector, left empty
	Block 1: super block (defined in fs.h)
	Block 2: inode table (defined in fs.h)
	Block 3: default 1st block of root dir

Super block:
	Inode 0 is reserved for root dir
	Root dir starts at block 3
	1st data block starts at block 4
	Free-block linked list: 
		freeList_First points to the 1st node of the linked list block
		freeList_Last points to the last node of the linked list block
	freeBlockCount counts the number of free blocks
	blockSize specifies the size of a block in bytes
	
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
	
Linked list of free blocks
	Each node contains a list of free block indices. The last index points to the next linked list node.
	1st and last nodes are loaded to RAM during the run
	Set an free block index to 0 to remove it from the free blocks list.
	
Features:
	This version implements checkdisk command to fix errors on data block consistency.
	Function fs_checkdisk is moved to checkdisk.c as a separate module for students to implement