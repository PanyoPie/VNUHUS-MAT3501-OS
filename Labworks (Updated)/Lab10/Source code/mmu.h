#ifndef MMU_H
#define MMU_H

#include "config.h"

/*
 * mmu.h
 * =====
 * The Memory Management Unit (MMU) translates 12-bit virtual addresses
 * to physical addresses and performs the actual read/write to RAM.
 *
 * Virtual address layout (12 bits total):
 *   [11:8]  VPN    (4-bit virtual page number)
 *   [ 7:0]  offset (8-bit byte offset within the page)
 *
 * Physical address layout:
 *   [11:8]  frame  (4-bit physical frame number)
 *   [ 7:0]  offset (same offset)
 */

/* Initialise the MMU (currently a no-op, reserved for TLB extension) */
void initMmu(void);

/*
 * mmuRead(virtual_addr)
 * ----------------------
 * Translate the address and return the byte value at that location.
 * Triggers a page fault via the page supervisor if the page is absent.
 */
unsigned char mmuRead(unsigned int virtual_addr);

/*
 * mmuWrite(virtual_addr, value)
 * --------------------------------
 * Translate the address and write 'value' to physical memory.
 * Marks the page as dirty (needed for write-back on eviction).
 */
void mmuWrite(unsigned int virtual_addr, unsigned char value);

/* Pretty-print the current state of the page table */
void printPageTable(int valid_entry_only);

/* Pretty-print the contents of physical memory (one frame per row) */
void printPhysicalMemory(void);

#endif /* MMU_H */
