#define VIRTUALMEM_H

int vm_setByte(int virtualAddr, char byteVal);
int vm_getByte(int virtualAddr, char *pByte);
int vm_initMem();
void vm_printPageTable();