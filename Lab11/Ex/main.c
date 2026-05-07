/* MAT3501 - Principles of Operating System, MIM - HUS
 * ======
 * Virtual Memory Simulator – entry point.
 *
 * Usage:
 *   vmsim [options] <program.asm>
 *
 * Options:
 *   -v              Verbose mode (step-by-step explanation)
 *   -a fifo|lru|clock
 *                   Page-replacement algorithm (default: fifo)
 *   -h              Print help and exit
 *
 * Example:
 *   ./vmsim -v -a lru program.asm
 *
 * Modules overview:
 *   CPU              – fetches and executes LOAD/STORE instructions
 *   MMU              – translates virtual → physical addresses
 *   Page Supervisor  – handles page faults, manages frames
 *   Replacement      – FIFO / LRU / CLOCK eviction policy
 */

#include "config.h"
#include "cpu.h"
#include "mmu.h"
#include "page_supervisor.h"
#include "replacement.h"

/* ---------------------------------------------
 * Global state definitions (declared extern in config.h)
 * --------------------------------------------- */
unsigned char ram[RAM_SIZE];
int verbose = 0;
PageTableEntry pageTable[NUM_PAGES];
ReplacementAlgo algo = ALGO_FIFO;
const char *algoNames[] = {"FIFO", "LRU", "CLOCK", "WSCLOCK"};

int statMemoryAccesses  = 0;
int statPageFaults      = 0;
int statPageReplacements = 0;

/* ---------------------------------------------
 * printUsage
 * --------------------------------------------- */
static void printUsage(const char *prog)
{
    printf("Virtual Memory Simulator\n");
    printf("Usage: %s [options] <program.asm>\n\n", prog);
    printf("Options:\n");
    printf("  -v              Verbose: print step-by-step execution detail\n");
    printf("  -a <algo>       Replacement algorithm: fifo | lru | clock\n");
    printf("                  (default: fifo)\n");
    printf("  -h              Show this help message\n\n");
    printf("System parameters:\n");
    printf("  Virtual address space : %d bits (%d pages x %d bytes)\n",
           VIRTUAL_ADDR_BITS, NUM_PAGES, PAGE_SIZE);
    printf("  Physical memory       : %d frames x %d bytes = %d bytes\n",
           NUM_FRAMES, PAGE_SIZE, RAM_SIZE);
}

/* ---------------------------------------------
 * printStats – final simulation statistics
 * --------------------------------------------- */
static void printStats(void)
{
    printf("\n╔══════════════════════════════════════╗\n");
    printf("║       SIMULATION STATISTICS          ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  Memory accesses  : %-16d ║\n", statMemoryAccesses);
    printf("║  Page faults      : %-16d ║\n", statPageFaults);
    printf("║  Page replacements: %-16d ║\n", statPageReplacements);
    if (statMemoryAccesses > 0)
        printf("║  Fault rate       : %-15.1f%% ║\n",
               100.0 * statPageFaults / statMemoryAccesses);
    printf("╚══════════════════════════════════════╝\n");
}

/* ---------------------------------------------
 * main
 * --------------------------------------------- */
int main(int argc, char *argv[])
{
    int   i;
    const char *programFile = NULL;

    /* -- Parse command-line arguments -------- */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;

        } else if (strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return EXIT_SUCCESS;

        } else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: -a requires an argument.\n");
                return EXIT_FAILURE;
            }
            i++;
            if (strcmp(argv[i], "fifo") == 0)       algo = ALGO_FIFO;
            else if (strcmp(argv[i], "lru") == 0)   algo = ALGO_LRU;
            else if (strcmp(argv[i], "clock") == 0) algo = ALGO_CLOCK;
			else if (strcmp(argv[i], "wsclock") == 0) algo = ALGO_WSCLOCK;
            else {
                fprintf(stderr, "ERROR: Unknown algorithm '%s'."
                        " Choose fifo, lru, or clock.\n", argv[i]);
                return EXIT_FAILURE;
            }

        } else if (argv[i][0] != '-') {
            programFile = argv[i];

        } else {
            fprintf(stderr, "ERROR: Unknown option '%s'.\n", argv[i]);
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (!programFile) {
        fprintf(stderr, "ERROR: No program file specified.\n\n");
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    /* -- Banner ------------------------------- */
    printf("╔══════════════════════════════════════╗\n");
    printf("║    VIRTUAL MEMORY SIMULATOR v1.0     ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║  Program   : %-23s ║\n", programFile);
    printf("║  Algorithm : %-23s ║\n", algoNames[algo]);
    printf("║  Verbose   : %-23s ║\n", verbose ? "ON" : "OFF");
    printf("║  Frames    : %-23d ║\n", NUM_FRAMES);
    printf("║  Pages     : %-23d ║\n", NUM_PAGES);
    printf("╚══════════════════════════════════════╝\n\n");

    /* -- Initialise all modules --------------- */
    initReplacement(algo);
    initPageSupervisor();
    initMmu();
    initCpu();

    /* -- Run the program ---------------------- */
    cpuRun(programFile);

    /* -- Post-run diagnostics ----------------- */
    printPageTable(0);	// print all entries
	printReplacementInfo();
    printStats();

    return EXIT_SUCCESS;
}
