/*
 * replacement.c
 * =============
 * Implements three classic page-replacement algorithms:
 *
 *   FIFO  – evict the frame that was loaded first (circular queue)
 *   LRU   – evict the frame that was least-recently used
 *   CLOCK – "second-chance" approximation of LRU using a reference bit
 *
 * The rest of the simulator calls only the four public functions
 * declared in replacement.h; algorithm details are hidden here.
 */

#include "replacement.h"

/* ---------------------------------------------
 * Internal state shared by all algorithms
 * --------------------------------------------- */

static ReplacementAlgo currentAlgo;

/* FIFO: circular queue of frame numbers in load order */
static int fifoQueue[NUM_FRAMES];   /* queue slots                     */
static int fifoHead;                /* index of oldest frame (next out) */
static int fifoTail;                /* index where next frame goes in  */
static int fifoCount;               /* how many frames are occupied     */

/* LRU: timestamp counter – higher value = more recently used */
static int lruTimestamp[NUM_FRAMES];
static int lruClockCounter;        /* global "time" tick. Increment by 1 (++)
										after every page loading or access */
/* CLOCK: a circular pointer ("hand") sweeps through frames */
static int clockHand;               /* current position of the hand    */
/* NOTE: the reference bit for CLOCK is stored in pageTable[].referenced
   so the MMU can update it on every access without knowing which algo
   is active. */

/* ---------------------------------------------
 * initReplacement – reset all internal state
 * --------------------------------------------- */
void initReplacement(ReplacementAlgo a)
{
    int i;
    currentAlgo = a;

    /* Zero out everything so we start clean */
    for (i = 0; i < NUM_FRAMES; i++) {
        fifoQueue[i]    = -1;
        lruTimestamp[i] = 0;
    }
    fifoHead        = 0;
    fifoTail        = 0;
    fifoCount       = 0;
    lruClockCounter = 0;
    clockHand       = 0;

    if (verbose) {
        printf("[REP] Initialised with %s algorithm.\n", algoNames[a]);
    }
}

/* ---------------------------------------------
 * notifyLoad – called when a page is loaded into 'frame'
 * --------------------------------------------- */
void notifyLoad(unsigned int frame)
{
    if (currentAlgo == ALGO_FIFO) {
        /* Add this frame to the back of the FIFO queue */
        fifoQueue[fifoTail] = frame;
        fifoTail = (fifoTail + 1) % NUM_FRAMES;
        if (fifoCount < NUM_FRAMES) fifoCount++;

        if (verbose)
            printf("[FIFO] Frame %d added to queue (head=%d tail=%d).\n",
                   frame, fifoHead, fifoTail);

    } else if (currentAlgo == ALGO_LRU) {
        /* Record load time as the most-recent timestamp */
        lruTimestamp[frame] = ++lruClockCounter;

        if (verbose)
            printf("[LRU] Frame %d loaded, timestamp=%d.\n",
                   frame, lruTimestamp[frame]);

    } else { // if (currentAlgo == ALGO_CLOCK) { /* CLOCK */
        /* Reference bit is already cleared when we evict;
           just note the load for debugging */
        if (verbose)
            printf("[CLOCK] Frame %d loaded (hand at %d).\n",
                   frame, clockHand);
    } // else { /* WSCLOCK update goes here */
		// copy code from CLOCK branch and modify
		// increment lruClockCounter and assign it to the frame time label
		// ...
}

/* ---------------------------------------------
 * notifyAccess – called on every read/write to 'frame'
 * --------------------------------------------- */
void notifyAccess(unsigned int frame)
{
    if (currentAlgo == ALGO_LRU) {
        /* Update the timestamp to "now" */
        lruTimestamp[frame] = ++lruClockCounter;

        if (verbose)
            printf("[LRU] Frame %d accessed, timestamp updated to %d.\n",
                   frame, lruTimestamp[frame]);

    } else if (currentAlgo == ALGO_CLOCK) {
        /* Set the reference bit — the page gets a "second chance".
         * frameToVpn gives us the owning VPN in O(1). */
        int vpn = frameToVpn[frame];
        if (vpn >= 0) {
            pageTable[vpn].referenced = 1;
            if (verbose)
                printf("[CLOCK] Set ref bit R=1 for frame %d (VPN %d).\n",
                       frame, vpn);
        }
    } // else if (currentAlgo == ALGO_WSCLOCK) {
        // copy code from CLOCK branch here
		// ...
		// increment lruClockCounter
		// ...
		
    /* FIFO does not change on access – order is determined by load time only */
}

/* ---------------------------------------------
 * chooseVictim – return the frame number to evict
 * --------------------------------------------- */
unsigned int chooseVictim(void)
{   int victim = -1;
	// int threshold = 5; /* Working set threshold for WSCLOCK algo */

    /* -- FIFO ----------------------------------- */
    if (currentAlgo == ALGO_FIFO) {
        /* The oldest loaded frame is at the head of the queue */
        victim = fifoQueue[fifoHead];
        fifoHead = (fifoHead + 1) % NUM_FRAMES;

        if (verbose) {
			printReplacementInfo();
            printf("[FIFO] Victim = frame %d (oldest in queue).\n", victim);
		}

    /* -- LRU ------------------------------------ */
    } else if (currentAlgo == ALGO_LRU) {
        int i, min_ts = lruTimestamp[0], min_frame = 0;

        /* Scan all frames; pick the one with the smallest timestamp */
        for (i = 1; i < NUM_FRAMES; i++) {
            if (lruTimestamp[i] < min_ts) {
                min_ts    = lruTimestamp[i];
                min_frame = i;
            }
        }
        victim = min_frame;

        if (verbose) {
			printReplacementInfo();
            printf("[LRU] Victim = frame %d (least recently used, ts=%d).\n",
                   victim, min_ts);
		}

    /* -- CLOCK ---------------------------------- */
    } else { // if (currentAlgo == ALGO_CLOCK) {
        /*
         * Walk frames in a circle starting from clockHand.
         * If the page in the current frame has reference=1, clear it
         * and give it a second chance. If reference=0, evict it.
         * Guaranteed to terminate because at worst we circle twice:
         * first pass clears all bits, second pass finds reference=0.
         */
        int steps = 0;
        while (steps < NUM_FRAMES * 2) {	// scan the frames only 2 rounds (2 chances only)
            /* O(1) lookup — no page-table scan needed */
            int vpn = frameToVpn[clockHand];

            if (vpn >= 0) {
                if (pageTable[vpn].referenced == 0) {
                    if (verbose)
                        printf("[CLOCK] Victim = frame %d (VPN %d, R=0).\n",
                               victim, vpn);
					break; // found the victim, pointed to by clock hand
                } else {
                    /* Give it a second chance: clear the reference bit
                     * and let the hand move on */
                    pageTable[vpn].referenced = 0;
                    if (verbose)
                        printf("[CLOCK] Frame %d (VPN %d) gets second chance,"
                               " clear ref bit R=0.\n", clockHand, vpn);
                }
            }

            clockHand = (clockHand + 1) % NUM_FRAMES;
            steps++;
        }
        // *** /* Fallback (should not happen in a well-formed simulation) */
		/* Case 1: found a victim with R==0. Case 2: no victim found after 2 rounds 
		   In both cases, replace the frame pointed to by clock hand */
        victim = clockHand;
        clockHand = (clockHand + 1) % NUM_FRAMES;
    } // else {	// WSCLOCK branch goes here
		// copy CLOCK code and modify it ...
		// ...
		// if (pageTable[vpn].referenced == 0 && age >= threshold) => evic the frame
		// ...
	
    return victim;
}

/* ---------------------------------------------
 * print replacement attributes
 * --------------------------------------------- */
void printReplacementInfo() {
	switch (currentAlgo) {
	case ALGO_FIFO:
		printf("Frames in fifo queue:\t");
		for (int i=0; i<fifoCount; i++) // print fifo queue
			printf("%d\t", fifoQueue[(fifoHead+i)%NUM_FRAMES]);
		printf("\n");
		break;
	case ALGO_LRU:
		printf("LRU (Frame,Timestamp):\t");
		for (int i = 0; i < NUM_FRAMES; i++) 
			printf("(%d, %d)\t", i, lruTimestamp[i]);
		printf("\n"); 
		break;
	case ALGO_CLOCK:
		printf("Clock hand ==> frame %d\n", clockHand);
		printf("Ref bit (Frame, Ref bit):\t");
		for (int i=0; i<NUM_FRAMES; i++) {
			int vpn = frameToVpn[i];
			printf("(%d, R=%d)\t", i, pageTable[vpn].referenced);
		}
		printf("\n");
		break;
	case ALGO_WSCLOCK:
		// ...
		break;
	}
}
