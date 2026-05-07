/*
 * mmu.c
 * =====
 * Memory Management Unit implementation.
 *
 * Responsibilities:
 *   1. Extract VPN and offset from a virtual address.
 *   2. Look up the page table.
 *   3. On a miss, call the Page Supervisor to handle the page fault.
 *   4. Compute the physical address and perform the read or write.
 *   5. Notify the replacement module about the access.
 */

#include "mmu.h"
#include "page_supervisor.h"
#include "replacement.h"

/* TLB ----------------------------------------- 
#define TLB_SIZE	3	//	#TLB entries
typedef struct {
    unsigned int vpn;	// virtual page
    unsigned int frame; // physical frame
    int valid;			// entry validity
} TlbEntry_t;

static TlbEntry_t tlb[TLB_SIZE];
static int tlbNext = 0;  // TLB FIFO replacement
*/

/* ---------------------------------------------
 * initMmu – placeholder for future TLB support
 * --------------------------------------------- */
void initMmu(void)
{
	// Initialise TLB here if available
	// ...
    if (verbose)
        printf("[MMU] Initialised.\n");
}

/* ---------------------------------------------
 * Internal helper: translate virtual -> physical
 * Returns the resolved physical address (byte index into ram[]).
 * Also updates reference bit and dirty flag as appropriate.
 * 'isWrite' = 1 for STORE operations, 0 for LOAD.
 * --------------------------------------------- */
static unsigned int translate(unsigned int virtual_addr, int isWrite)
{
    /* -- Extract VPN and offset --------------- */
    unsigned int vpn    = (virtual_addr >> PAGE_OFFSET_BITS) & 0xF;  /* bits [11:8] */
    unsigned int offset =  virtual_addr & (PAGE_SIZE - 1);           /* bits [ 7:0] */
    unsigned int frame, physical_addr;

    // It is a memory access
	// ...

    if (verbose) {
        printf("[MMU] Virtual addr 0x%03X -> VPN=%d, offset=0x%02X  (%s)\n",
               virtual_addr, vpn, offset, isWrite ? "WRITE" : "READ");
    }
	
	/* -- TLB lookup here ---------------------- */
	// if not found, proceed to Page Table lookup
	// ...

    /* -- Page-table lookup -------------------- */
    if (!pageTable[vpn].valid) {
        /* Page fault: delegate to the supervisor */
        frame = handlePageFault(vpn);
    } else {
        frame = pageTable[vpn].frame;
        if (verbose)
            printf("[MMU] Page table hit: VPN %d -> frame %d.\n", vpn, frame);
    }

    /* -- Set reference bit (used by CLOCK) --- */
    pageTable[vpn].referenced = 1;

    /* -- Mark dirty on writes ----------------- */
    if (isWrite)
        pageTable[vpn].dirty = 1;

    /* -- Notify replacement module ------------ */
    notifyAccess(frame);
	
	/* -- Update new entry to TLB --------------------------- */
	// ...

    /* -- Compute physical address ------------- */
    physical_addr = frame * PAGE_SIZE + offset;

    if (verbose)
        printf("[MMU] Physical addr = 0x%03X (frame %d, offset 0x%02X).\n",
               physical_addr, frame, offset);

    return physical_addr;
}

/* ---------------------------------------------
 * mmuRead
 * --------------------------------------------- */
unsigned char mmuRead(unsigned int virtual_addr)
{
    unsigned int phys  = translate(virtual_addr, 0);
    unsigned char value = ram[phys];

    if (verbose)
        printf("[MMU] READ  VA=0x%03X => PA=0x%03X => value = %d (0x%02X)\n\n",
               virtual_addr, phys, value, value);

    return value;
}

/* ---------------------------------------------
 * mmuWrite
 * --------------------------------------------- */
void mmuWrite(unsigned int virtual_addr, unsigned char value)
{
    unsigned int phys = translate(virtual_addr, 1);
    ram[phys] = value;

    if (verbose)
        printf("[MMU] WRITE VA=0x%03X => PA=0x%03X <= value = %d (0x%02X)\n\n",
               virtual_addr, phys, value, value);
}

/* ---------------------------------------------
 * printPageTable – diagnostic output
 * --------------------------------------------- */
void printPageTable(int valid_entry_only)
{
    int i;
    printf("\n╔══════════════════════════════════════╗\n");
	if (valid_entry_only)
		printf("║     PAGE TABLE - valid entries       ║\n");
	else 
		printf("║           PAGE TABLE STATE           ║\n");
    printf("╠═══════╦═══════╦═══════╦═══════╦══════╣\n");
    printf("║  VPN  ║ Valid ║ Frame ║ Dirty ║ Ref  ║\n");
    printf("╠═══════╬═══════╬═══════╬═══════╬══════╣\n");
    for (i = 0; i < NUM_PAGES; i++) {
		if (valid_entry_only && !pageTable[i].valid) continue; // skip printing
        printf("║  %2d   ║   %d   ║  %3s  ║   %d   ║  %d   ║\n",
               i,
               pageTable[i].valid,
               pageTable[i].valid ? (char[]){' ', '0' + pageTable[i].frame, '\0'} : " -",
               pageTable[i].dirty,
               pageTable[i].referenced);
    }
    printf("╚═══════╩═══════╩═══════╩═══════╩══════╝\n\n");
}

/* ---------------------------------------------
 * printPhysicalMemory – diagnostic output
 * Prints first 8 bytes of each frame to keep output manageable.
 * --------------------------------------------- */
void printPhysicalMemory(void)
{
    int f, b;
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║       PHYSICAL MEMORY (first 8B/frame)   ║\n");
    printf("╠═══════╦══════════════════════════════════╣\n");
    printf("║ Frame ║ Bytes (hex)                      ║\n");
    printf("╠═══════╬══════════════════════════════════╣\n");
    for (f = 0; f < NUM_FRAMES; f++) {
        printf("║   %d   ║ ", f);
        for (b = 0; b < 8; b++)
            printf("%02X ", ram[f*PAGE_SIZE + b]);
        printf("         ║\n");
    }
    printf("╚═══════╩══════════════════════════════════╝\n\n");
}