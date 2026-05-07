/* ----------------------------------------------------------------------------------------
	Module checkdisk.c
------------------------------------------------------------------------------------------- */

/* Explain in detail your algorithm here:
   Duyệt mảng nếu tổng của cột i( tức a[0][i] + a[1][i]) khác 1 thì xảy ra lỗi:
   + Với lỗi thất lạc khối đĩa(Lost blocks): add khối thất lạc vào list khối rỗi (lệnh addFreeBlock)
   + Với lỗi lặp khối đĩa rỗi (Duplicated inactive blocks) : xoá bớt khối lặp khỏi list khối rỗi
   + Với lỗi vừa rỗi vừa bận(Blocks marked as both statuses) : xoá khỏi list khối rỗi
*/
int i, j;
void fs_checkDisk()
{
	printf("Checking disk ...\n");
	// write your code here, add a comment after each line of code

	/* print file descriptor table: list toàn bộ thuộc tính định danh file trong bảng
		+ State: trạng thái
		+ File ptr : con trỏ file
		+ Inode no : inode đang dùng
		+ Indirect table: chứa block index mà file đó dùng
	*/
	printf("File descriptor table:\n");
	printf("%10s%10s%10s%16s\n", "State", "File ptr", "Inode no", "Indirect table");
	for (i = 0; i < MAXOPENFILES; i++)
	{
		if (fileDescriptors[i].fp == 0)
			continue;

		printf("%10d%10d%10d  ",
			fileDescriptors[i].state,	//
			fileDescriptors[i].fp, 		//fp: con trỏ tới địa chỉ của file cần ghi
			fileDescriptors[i].inodeNo);//inodeNo: index của inode mà file đang dùng
		for (j = 0; j < numInodes; j++)
		{
			int val = fileDescriptors[i].indirectTable[j];
			if (val > 0)
				printf("%d ", val);
		}
		printf("\n");
	}
	
	/* 
	- Tạo một con trỏ đến superBlock.freeBlockList (block rỗi đầu tiên)
 	- Duyệt khi con trỏ khác 0, đọc dữ liệu từ block và chỉ lấy 2 bytes đầu làm con trỏ đến block rỗi tiếp theo
	*/

	// Free block position, start with first free block from superblock
	int freeBlockPos = superBlock.freeBlockList;
	/*
	 * blockArr[0] = active
	 * blockArr[1] = inactive
	 */
	int blockArr[2][disk_blockCount];

	memset(blockArr[0], 0, disk_blockCount * sizeof(int));
	memset(blockArr[1], 0, disk_blockCount * sizeof(int));

	// First 3 blocks are active according to disk structure
	blockArr[0][0] = 1;
	blockArr[0][1] = 1;
	blockArr[0][2] = 1;
	blockArr[1][0] = 0;
	blockArr[1][1] = 0;
	blockArr[1][2] = 0;

	// Find active blocks
	for (j = 0; j < numInodes; j++)
	{
		uint16_t singleIndirectBlock = inodeTable[j].singleIndirect;
		uint16_t indirectBlocks[MAXINDIRECTBLOCKS];
		int k;

		if (singleIndirectBlock != 0)
			readBlock(singleIndirectBlock, (char*)indirectBlocks);

		for (k = 0; k < DIRECTBLOCKS; k++)
		{
			int block = inodeTable[j].direct[k];
			if (block > 2)
				blockArr[0][block]++;
		}

		if (singleIndirectBlock == 0)
			continue;

		blockArr[0][singleIndirectBlock]++;
		for (k = 0; k < MAXINDIRECTBLOCKS - 1; k++)
		{
			int block = indirectBlocks[k];
			if (block > 0)
				blockArr[0][block]++;
		}
	}

	// Find inactive blocks, if any
	if (fs_diskFreeBlocks() > 0) {
		do {
			// printBlock(freeBlockPos);
			blockArr[1][freeBlockPos]++;
			char buf[BLOCKSIZE];

			readBlock(freeBlockPos, buf);

			/*
			 * If duplicated inactive block, remove from list
			*/
			if (buf[0] < freeBlockPos) {
				char tmp[BLOCKSIZE];

				readBlock(buf[0], tmp);
				if (tmp[0] == freeBlockPos) {
					blockArr[1][buf[0]]++;
					buf[0] = 0;
					writeBlock(freeBlockPos, buf);
				}
			}

			freeBlockPos = buf[0];
		} while (freeBlockPos < disk_blockCount && freeBlockPos != 0);
	}

	// in ra blocks table
	printf("Blocks table:\n");
	for (i = 0; i < 2; i++) {
		for (j = 0; j < disk_blockCount; j++)
			printf("%d ", blockArr[i][j]);

		printf("\n");
	}

	// in ra các khối thất lạc
	printf("Lost blocks: ");
	for (i = 0; i < disk_blockCount; i++)
	{
		int activeCnt = blockArr[0][i];
		int inactiveCnt = blockArr[1][i];

		if (activeCnt + inactiveCnt == 0)
			printf("%d ", i);
	}
	printf("\n");

	// in ra khối đĩa trùng lặp cùng bận
	printf("Duplicated active blocks: ");
	for (i = 0; i < disk_blockCount; i++)
	{
		int activeCnt = blockArr[0][i];
		int inactiveCnt = blockArr[1][i];

		if (activeCnt > 1 && inactiveCnt == 0)
			printf("%d ", i);
	}
	printf("\n");

	// in ra khối đĩa trùng lặp cùng rỗi
	printf("Duplicated inactive blocks: ");
	for (i = 0; i < disk_blockCount; i++)
	{
		int activeCnt = blockArr[0][i];
		int inactiveCnt = blockArr[1][i];

		if (inactiveCnt > 1 && activeCnt == 0)
			printf("%d ", i);
	}
	printf("\n");

	// in khối đang ở cả 2 trạng thái rỗi và bận
	printf("Blocks marked as both statuses: ");
	for (i = 0; i < disk_blockCount; i++)
	{
		int activeCnt = blockArr[0][i];
		int inactiveCnt = blockArr[1][i];

		if (inactiveCnt > 0 && activeCnt > 0)
			printf("%d ", i);
	}
	printf("\n");

	// sửa lỗi
	printf("Fixing errors...\n");
	for (i = 0; i < disk_blockCount; i++)
	{
		int activeCnt = blockArr[0][i];
		int inactiveCnt = blockArr[1][i];

		if (activeCnt + inactiveCnt == 0) {
			addFreeBlock(i);
			continue;
		}
	}
}