/* ----------------------------------------------------------------------------------------
	Module checkdisk.c
------------------------------------------------------------------------------------------- */

/* Nhiệm vụ:
	- Quét bảng inode, lần theo các con trỏ direct và indirect để đếm các khối đĩa bận (active)
	- Quét dsmn lưu danh sách các khối rỗi
		Với mỗi node của dsmn, duyệt qua các con trỏ và đếm các khối rỗi
	- Phát hiện các lỗi khối đĩa 
		E1 khối đĩa thất lạc (lost block)
		E2 khối đĩa rỗi nhiều lần (duplicate free/inactive block)
		E3 khối đĩa bận nhiều lần (shared active block)
		E4 khối vừa bận vừa rỗi (both active and free/inactive)
	- Kết hợp khắc phục lỗi trong quá trình quét
	
	Chú thích:
	uint16_t blockArr[2][disk_blockCount]; là mảng thống kê trạng thái khối đĩa, dòng 0 bận, dòng 1 rỗi
*/


/*	Helper function: Clone a disk block to a new block then, 
		redirect the block pointer to the new block
	Argument: 
		blockArr stores the active/inactive block counters
		blockPtrAddr is the address of pointer that points to the block
*/
/* Uncomment this function to use it
static int cloneBlock(uint16_t blockArr[2][disk_blockCount], uint16_t *blockPtrAddr) {
	int newBlock, oldBlock;
	char buf[BLOCKSIZE];
	
	newBlock = getFreeBlock();	// get a free block
	if (newBlock<0) return -1;	// error: out of disk
	blockArr[0][newBlock]++;	// count the new block as busy (active)
	
	oldBlock = *blockPtrAddr;
	readBlock(oldBlock, buf);	// read the old block to buffer
	writeBlock(newBlock, buf);	// write the buffer to the new block
	
	*blockPtrAddr = newBlock;	// redirect to the new block
	
	printf("E3: Shared active block %d is copied to block %d.\n", oldBlock, newBlock);
	
	return newBlock;
}
*/

/*
	Check disk for errors on data blocks
*/
void fs_checkDisk()
{	uint16_t blockArr[2][disk_blockCount];	// block state counter array
	// char buf[BLOCKSIZE], strMsg[256];
	// uint16_t *pFreeList;
	// int i, j, blockNo, newBlock; //prevBlockNo
	int ret, errorCount=0;
	
	// unmount and mount file system to sync content in buffer and disk
	fs_umount();
	ret = fs_mount (DISKNAME);
	if (ret != 0) {
		if (ret==-2) printf ("Could not mount '%s'. Please reformat the disk.\n", DISKNAME); 
		else printf ("Virtual disk '%s' not found. Run makedisk first.\n", DISKNAME); 
		exit(-1); 
	}
	
	printf("Checking disk ...\n");

	/*
	 * blockArr[0] = active / busy
	 * blockArr[1] = inactive / free
	*/
	memset(blockArr, 0, disk_blockCount * sizeof(uint16_t)*2);

	// First 3 blocks are active according to disk structure
	blockArr[0][0] = 1;	// boot block
	blockArr[1][0] = 0;	
	blockArr[0][1] = 1;	// super block
	blockArr[1][1] = 0;
	blockArr[0][2] = 1;	// inode table
	blockArr[1][2] = 0;
 
	/* Count active blocks */
	/* Thuật toán:
		Duyệt qua từng inode trong bảng inodeTable[] có linkCount>0 (đang dùng)
		
			Duyệt qua các con trỏ direct trong mỗi inode
				Tăng số đếm khối active tương ứng lên 1
				Nếu số đếm > 1 thì gặp lỗi có 2 inodes trỏ tới cùng 1 khối dữ liệu
					=> Xử lý: gọi hàm cloneBlock sao chép ra 1 khối mới, cho inode trỏ tới khối mới
		
			Tải khối đĩa trỏ bởi con trỏ singleIndirect trong inode
			Đếm khối này là active
			Kiểm tra xem nếu số đếm > 1 thì gặp lỗi có 2 inodes trỏ tới cùng 1 khối dữ liệu
				=> Xử lý: gọi hàm cloneBlock sao chép ra 1 khối mới, cho inode trỏ tới khối mới
			
			Duyệt qua từng con trỏ trong khối đĩa mà singleIndirect trỏ đến
				Tăng số đếm khối active tương ứng lên 1
					Nếu số đếm > 1 thì gặp lỗi có 2 inodes trỏ tới cùng 1 khối dữ liệu
					=> Xử lý: gọi hàm cloneBlock sao chép ra 1 khối mới, cho inode trỏ tới khối mới
					sau đó ghi khối đĩa singleIndirect chứa các con trỏ trở lại đĩa
	*/
	
	// Scan the inode table
	// insert your code here
	for (int i = 0; i < superBlock.inodeCount; i++) {
		if (inodeTable[i].linkCount > 0) {
			for (int j = 0; j < 10; j++) {
				blockNo = inodeTable[i].direct[j];
				if (blockNo != 0) {
					blockArr[0][blockNo]++;
					if (blockArr[0][blockNo] > 1) {
						cloneBlock(blockArr, &inodeTable[i].direct[j]);
						errorCount++;
					}
				}
			}

			if (inodeTable[i].singleIndirect != 0) {
				blockNo = inodeTable[i].singleIndirect;
				blockArr[0][blockNo]++;
				if (blockArr[0][blockNo] > 1) {
					cloneBlock(blockArr, &inodeTable[i].singleIndirect);
					blockNo = inodeTable[i].singleIndirect;
					errorCount++;
				}

				uint16_t indexBuf[BLOCKSIZE / sizeof(uint16_t)];
				readBlock(blockNo, (char*)indexBuf);
				int modified = 0;
				for (int j = 0; j < (BLOCKSIZE / sizeof(uint16_t)); j++) {
					if (indexBuf[j] != 0) {
						blockArr[0][indexBuf[j]]++;
						if (blockArr[0][indexBuf[j]] > 1) {
							cloneBlock(blockArr, &indexBuf[j]);
							modified = 1;
							errorCount++;
						}
					}
				}
				if (modified) writeBlock(blockNo, (char*)indexBuf);
			}
		}
	}
	

	/* Count free blocks including inactive blocks and active blocks acting as linked freelist nodes */
	
	/* Thuật toán
		Đầu tiên cần ghi lại khối node đầu và cuối của dsmn khối rỗi đang lưu trong bộ nhớ xuống đĩa
		Tải vào lần lượt từng khối đĩa của dsmn chứa các chỉ số khối rỗi (freelist node)
			Mỗi khối này là 1 khối active nên cũng phải đếm là khối bận và kiểm tra lỗi và xử lý lỗi như trên
			Duyệt qua từng chỉ số trong node hiện tại của dsmn khối rỗi
				Tăng số đêm inactive cho khối tương ứng
				Nếu khối đĩa xuất hiện nhiều lần trong dsmn thì xóa khỏi bảng của dsmn
				Nếu khối đĩa vừa active, vừa inactive thì cũng xóa khổi bảng của dsmn
				Lưu ý: để xóa khỏi node hiện tại của dsmn, chỉ cần gán giá trị chỉ số đó bằng 0
	*/
	
	// Before checking, flush in-memory buffers of 1st and last freelist node back to disk
	writeBlock(superBlock.freeList_First, freeBlocks.pFirstBlock);
	writeBlock(superBlock.freeList_Last, freeBlocks.pLastBlock);
	// blockNo = superBlock.freeList_First; 
	
	// Scan each freelist node. The 1st node is already stored in RAM
	// insert your code here
	int currentFreeNode = superBlock.freeList_First;
	uint16_t nodeBuf[BLOCKSIZE / sizeof(uint16_t)];
	int entriesPerBlock = (BLOCKSIZE / sizeof(uint16_t)) - 1;

	while (currentFreeNode != 0) {
		blockArr[0][currentFreeNode]++;
		if (blockArr[0][currentFreeNode] > 1) {
			errorCount++;
		}

		readBlock(currentFreeNode, (char*) nodeBuf);
		int nodeModified = 0;

		for (int i = 0; i < entriesPerBlock; i++) {
			uint16_t fBlock = nodeBuf[i];
			if (fBlock != 0) {
				blockArr[1][fBlock]++;
				if (blockArr[1][fBlock] > 1 || blockArr[0][fBlock] > 0) {
					if (blockArr[1][fBlock] > 1) printf("E2: Duplicate free block %d\n", fBlock);
					if (blockArr[0][fBlock] > 0) printf("E4: Both active and free block %d\n", fBlock);
					nodeBuf[i] = 0;
					nodeModified = 1;
					errorCount++;
				}
			}
		}

		if (nodeModified) writeBlock(currentFreeNode, (char*)nodeBuf);
		currentFreeNode = nodeBuf[entriesPerBlock];
	}

	
	// If the block is lost => add it to the freelist
	// scan the block state array blockArr
	// insert your code here
	for (int i = 3; i < disk_blockCount; i++) {
		if (blockArr[0][i] == 0 && blockArr[1][i] == 0) {
			printf("E1: Lost block %d added to freelist.\n", i);
			addFreeBlock(i);
			blockArr[1][i] = 1;
			errorCount++;
		}
	}

	
	// statistic
	printf("Checkdisk is not yet implemented\n");
	printf ("%d errors found and fixed.\n", errorCount);
	
	// unmount and mount file system to sync content in buffer and disk
	fs_umount();
	ret = fs_mount (DISKNAME);
	if (ret != 0) {
		if (ret==-2) printf ("Could not mount '%s'. Please reformat the disk.\n", DISKNAME); 
		else printf ("Virtual disk '%s' not found. Run makedisk first.\n", DISKNAME); 
		exit(-1); 
	}
}