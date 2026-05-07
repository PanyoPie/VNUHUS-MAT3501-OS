#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------
 * System-wide configuration constants
 * --------------------------------------------- */
#define PAGE_SIZE        256   /* bytes per page (and per frame)          */
#define NUM_PAGES        16    /* number of virtual pages (4-bit VPN)     */
#define NUM_FRAMES       4     /* physical frames available in RAM         */
#define RAM_SIZE      (NUM_FRAMES * PAGE_SIZE)
#define VMEM_SIZE     (NUM_PAGES * PAGE_SIZE)

// 12-bit virtual addr, 10-bit physical addr, 8-bit offset
#define VIRTUAL_ADDR_BITS 12   /* 12-bit virtual address space            */
#define PAGE_OFFSET_BITS  8    /* lower 8 bits = offset within a page     */
#define VPN_BITS          4    /* upper 4 bits = virtual page number      */

/* ---------------------------------------------
 * Page-table entry
 * --------------------------------------------- */
typedef struct {
    int  valid;        /* 1 = page is in physical memory               */
    int  frame;        /* physical frame number (valid when valid==1)  */
    int  dirty;        /* 1 = page has been written to (STORE)         */
    int  referenced;   /* used by CLOCK algorithm                      */
} PageTableEntry;

/* ---------------------------------------------
 * Replacement algorithm identifiers
 * --------------------------------------------- */
typedef enum {
    ALGO_FIFO  = 0,
    ALGO_LRU   = 1,
    ALGO_CLOCK = 2,
	ALGO_WSCLOCK = 3
} ReplacementAlgo;

extern const char *algoNames[];

/* ---------------------------------------------
 * Global simulation state (defined in main.c)
 * --------------------------------------------- */
extern unsigned char ram[RAM_SIZE];  /* flat byte array  */
extern int verbose;                        /* verbose flag     */
extern PageTableEntry pageTable[NUM_PAGES];
extern ReplacementAlgo algo;                          /* chosen algorithm */

/*
 * Reverse map: frameToVpn[f] gives the virtual page number currently
 * occupying physical frame f, or -1 if the frame is free.
 *
 * Maintained by the Page Supervisor (the only module that allocates and
 * frees frames). Read by the Replacement module so CLOCK can look up
 * the VPN for any frame in O(1) instead of scanning the page table.
 */
extern int frameToVpn[NUM_FRAMES];

/* ---------------------------------------------
 * Statistics counters
 * --------------------------------------------- */
extern int statMemoryAccesses;
extern int statPageFaults;
extern int statPageReplacements;

#endif /* CONFIG_H */
