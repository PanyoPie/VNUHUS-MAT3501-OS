/*
 * page_supervisor.c
 * =================
 * Handles page faults: locates a free (or victim) frame, simulates
 * loading the page from "disk" into physical memory, and updates the
 * page table accordingly.
 *
 * In this educational simulator, "disk" is simply a deterministic
 * function that fills a page with a known byte pattern so students
 * can verify that data is correctly preserved across evictions.
 */

#include "page_supervisor.h"
#include "replacement.h"

/* ---------------------------------------------
 * Free-frame pool
 * --------------------------------------------- */
static int freeFrames[NUM_FRAMES]; /* stack of unallocated frame numbers */
static int freeTop;                /* index of top of stack (-1 = empty) */

/* ---------------------------------------------
 * External storage for virtual pages
 * --------------------------------------------- */
 static unsigned char backingStore[VMEM_SIZE];  /* flat byte array  */

/*
 * Reverse map: frameToVpn[f] gives the virtual page number currently
 * occupying physical frame f, or -1 if the frame is free.
 *
 * Maintained by the Page Supervisor (the only module that allocates and
 * frees frames). Read by the Replacement module so CLOCK can look up
 * the VPN for any frame in O(1) instead of scanning the page table.
 */
int frameToVpn[NUM_FRAMES];


/* ---------------------------------------------
 * initPageSupervisor
 * --------------------------------------------- */
void initPageSupervisor(void)
{
    unsigned int i;

    /* Push all frames onto the free stack (frame 0 at bottom, NUM_FRAMES-1 at top) */
    for (i = 0; i < NUM_FRAMES; i++) {
        freeFrames[i]  = i;
        frameToVpn[i] = -1;   /* -1 = frame is currently free */
    }
    freeTop = NUM_FRAMES - 1;   /* top of stack index */

    /* Invalidate every page-table entry */
    for (i = 0; i < NUM_PAGES; i++) {
        pageTable[i].valid      = 0;
        pageTable[i].frame      = -1;
        pageTable[i].dirty      = 0;
        pageTable[i].referenced = 0;
    }

    if (verbose)
        printf("[SUP] Initialised. %d free frames available.\n",
               NUM_FRAMES);
}

/* ---------------------------------------------
 * simulateLoadFromDisk
 * Fill the frame with a recognisable pattern so that students can
 * see that page content survives eviction/reload cycles.
 * Pattern: each byte = (vpn * 16 + offset) & 0xFF
 * --------------------------------------------- */
static void simulateLoadFromDisk(unsigned int vpn, unsigned int frame)
{
    memcpy(ram+frame*PAGE_SIZE, backingStore+vpn*PAGE_SIZE, PAGE_SIZE);
    if (verbose)
        printf("[SUP] Page %d loaded from disk into frame %d\n", vpn, frame);
}

/* ---------------------------------------------
 * simulateSaveToDisk
 * In a real OS this would write the dirty page back to swap.
 * Here we just print a notice for educational purposes.
 * --------------------------------------------- */
static void simulateSaveToDisk(unsigned int vpn, unsigned int frame)
{
	memcpy(backingStore+vpn*PAGE_SIZE, ram+frame*PAGE_SIZE, PAGE_SIZE);
    if (verbose)
        printf("[SUP] Dirty page %d (frame %d) written back to"
               " disk.\n", vpn, frame);
}

/* ---------------------------------------------
 * handlePageFault(vpn)
 * Returns the physical frame where the page is now resident.
 * --------------------------------------------- */
unsigned int handlePageFault(unsigned int vpn)
{
    int frame;
    
	// It is a page fault event
	// ...

    if (verbose)
        printf("\n[SUP] PAGE FAULT for VPN %d!\n", vpn);

    /* -- Step 1: Is there a free frame? -------- */
    if (freeTop >= 0) {
        frame    = freeFrames[freeTop--];  /* pop from stack */
        if (verbose)
            printf("[SUP] Free frame %d available.\n", frame);

    } else {
        /* -- Step 2: All frames occupied – choose a victim -- */
        unsigned int victim_frame = chooseVictim();
		
		// Page replacement occurs, increment the page replacement counter
        // ...

        /* O(1) reverse lookup — no page-table scan needed */
        int victim_vpn = frameToVpn[victim_frame];

        if (verbose)
            printf("[SUP] Evicting VPN %d from frame %d.\n",
                   victim_vpn, victim_frame);

        /* Write back if the victim page is dirty */
        if (victim_vpn >= 0 && pageTable[victim_vpn].dirty)
            simulateSaveToDisk(victim_vpn, victim_frame);

        /* Invalidate the victim's page-table entry */
        if (victim_vpn >= 0) {
            pageTable[victim_vpn].valid      = 0;
            pageTable[victim_vpn].frame      = -1;
            pageTable[victim_vpn].dirty      = 0;
            pageTable[victim_vpn].referenced = 0;
        }

        /* The frame is no longer owned by any VPN */
        frameToVpn[victim_frame] = -1;

        frame = victim_frame;
    }

    /* -- Step 3: Load the requested page into 'frame' -- */
    simulateLoadFromDisk(vpn, frame);

    /* -- Step 4: Update the page table ---------- */
    pageTable[vpn].valid      = 1;
    pageTable[vpn].frame      = frame;
    pageTable[vpn].dirty      = 0;
    pageTable[vpn].referenced = 0;

    /* -- Step 5: Update the reverse map --------- */
    frameToVpn[frame] = vpn;

    /* -- Step 5: Tell the replacement module a new page was loaded -- */
    notifyLoad(frame);

    if (verbose) {
        printf("[SUP] VPN %d -> frame %d. Page fault resolved.\n\n",
               vpn, frame);
    }

    return frame;
}
