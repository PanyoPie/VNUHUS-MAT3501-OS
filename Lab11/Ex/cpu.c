/*
 * cpu.c
 * =====
 * Simulates a tiny CPU that fetches, decodes, and executes instructions
 * from a plain-text assembly file.  Each instruction triggers memory
 * accesses through the MMU, which in turn may cause page faults handled
 * by the Page Supervisor.
 *
 * This module is intentionally simple so students can focus on the
 * virtual-memory machinery rather than CPU internals.
 */

#include "config.h"
#include "cpu.h"
#include "mmu.h"
#include "replacement.h"

/* ---------------------------------------------
 * Register file: R0 .. R3
 * --------------------------------------------- */
#define NUM_REGS 4
static unsigned char regs[NUM_REGS];   /* general-purpose registers */
static unsigned int pc;               /* program counter (instruction index) */

/* ---------------------------------------------
 * initCpu
 * --------------------------------------------- */
void initCpu(void)
{
    int i;
    for (i = 0; i < NUM_REGS; i++) regs[i] = 0;
    pc = 0;

    if (verbose)
        printf("[CPU] Initialised. Registers cleared (R0-R3 = 0).\n");
}

/* ---------------------------------------------
 * parseReg – convert "R0".."R3" to index 0..3
 * Returns -1 on error.
 * --------------------------------------------- */
static int parseReg(const char *s)
{
    if ((s[0] == 'R' || s[0] == 'r') &&
        s[1] >= '0' && s[1] <= '3' && s[2] == '\0')
        return s[1] - '0';
    return -1;
}

/* ---------------------------------------------
 * parseAddr – accept decimal or 0x… hex strings
 * --------------------------------------------- */
static unsigned int parseAddr(const char *s)
{
    int val;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        sscanf(s + 2, "%x", &val);
    else
        val = atoi(s);
    return val;
}

/* ---------------------------------------------
 * printRegisters
 * --------------------------------------------- */
void printRegisters(void)
{
    int i;
    printf("  Registers: ");
    for (i = 0; i < NUM_REGS; i++)
        printf("R%d=%d  ", i, regs[i]);
    printf("\n");
}

/* ---------------------------------------------
 * cpuRun – load file and execute line by line
 * --------------------------------------------- */
void cpuRun(const char *filename)
{
    FILE *fp;
    char  line[256];
    char  op[32], arg1[32], arg2[32];
    int   line_num = 0;

    fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "[CPU] ERROR: Cannot open program file '%s'.\n",
                filename);
        exit(EXIT_FAILURE);
    }

    if (verbose)
        printf("[CPU] Loaded program from '%s'. Beginning execution.\n\n", filename);

    /* -- Main fetch-decode-execute loop -------- */
    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        /* Strip trailing newline */
        line[strcspn(line, "\n")] = '\0';

        /* Skip blank lines and comments */
        if (line[0] == '\0' || line[0] == '#') continue;

        /* Parse: up to three whitespace-separated tokens */
        op[0] = arg1[0] = arg2[0] = '\0';
        int n = sscanf(line, "%31s %31s %31s", op, arg1, arg2);

        if (n < 1) continue;

        /* Convert opcode to uppercase for case-insensitive matching */
        {
            char *p;
            for (p = op; *p; p++)
                if (*p >= 'a' && *p <= 'z') *p -= 32;
        }

        if (verbose) {
            printf("------------------------------------------\n");
            printf("[CPU] PC=%d  |  %s\n", pc, line);
        }

        /* -- LOAD <reg> <addr> ---------------- */
        if (strcmp(op, "LOAD") == 0) {
            int reg  = parseReg(arg1);
            int addr = parseAddr(arg2);

            if (reg < 0) {
                fprintf(stderr, "[CPU] Line %d: Unknown register '%s'.\n",
                        line_num, arg1);
                continue;
            }
            if (addr < 0 || addr >= (NUM_PAGES * PAGE_SIZE)) {
                fprintf(stderr, "[CPU] Line %d: Virtual address 0x%X out of"
                        " range.\n", line_num, addr);
                continue;
            }

            regs[reg] = mmuRead(addr);

            if (verbose)
                printf("[CPU] LOAD: R%d = mem[0x%03X] = %d\n",
                       reg, addr, regs[reg]);

        /* -- STORE <reg> <addr> --------------- */
        } else if (strcmp(op, "STORE") == 0) {
            int reg  = parseReg(arg1);
            int addr = parseAddr(arg2);

            if (reg < 0) {
                fprintf(stderr, "[CPU] Line %d: Unknown register '%s'.\n",
                        line_num, arg1);
                continue;
            }
            if (addr < 0 || addr >= (NUM_PAGES * PAGE_SIZE)) {
                fprintf(stderr, "[CPU] Line %d: Virtual address 0x%X out of"
                        " range.\n", line_num, addr);
                continue;
            }

            mmuWrite(addr, regs[reg]);

            if (verbose)
                printf("[CPU] STORE: mem[0x%03X] = R%d = %d\n",
                       addr, reg, regs[reg]);

        /* -- SET <reg> <value> ---------------- */
        } else if (strcmp(op, "SET") == 0) {
            int reg = parseReg(arg1);
            int val = parseAddr(arg2);   /* reuse hex-aware parser */

            if (reg < 0) {
                fprintf(stderr, "[CPU] Line %d: Unknown register '%s'.\n",
                        line_num, arg1);
                continue;
            }

            regs[reg] = val;

            if (verbose)
                printf("[CPU] SET: R%d = %d\n", reg, val);

        /* -- PRINT <reg> ---------------------- */
        } else if (strcmp(op, "PRINT") == 0) {
            int reg = parseReg(arg1);

            if (reg < 0) {
                fprintf(stderr, "[CPU] Line %d: Unknown register '%s'.\n",
                        line_num, arg1);
                continue;
            }
            printf("[CPU] PRINT: R%d = %d\n", reg, regs[reg]);

        } else {
            fprintf(stderr, "[CPU] Line %d: Unknown instruction '%s'.\n",
                    line_num, op);
        }

        pc++;

        if (verbose) {
			printRegisters();
			printPageTable(1); // print valid entries only
			printReplacementInfo();
		}
    }

    fclose(fp);

    if (verbose) {
        printf("------------------------------------------\n");
        printf("[CPU] Program finished. %d instructions executed.\n\n", pc);
    }
}
