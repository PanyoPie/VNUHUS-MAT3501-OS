#ifndef CPU_H
#define CPU_H

#include "config.h"

/*
 * cpu.h
 * =====
 * The CPU module reads and executes a simple assembly program.
 *
 * Supported instructions:
 *
 *   LOAD  <reg> <virtual_addr>
 *       Load the byte at <virtual_addr> into register <reg>.
 *       <reg> is R0 .. R3 (four general-purpose 32-bit registers).
 *
 *   STORE <reg> <virtual_addr>
 *       Store the byte value of <reg> into memory at <virtual_addr>.
 *
 *   SET   <reg> <value>
 *       Load an immediate integer value directly into <reg>.
 *       (Useful for preparing STORE values without a prior LOAD.)
 *
 *   PRINT <reg>
 *       Print the current value of <reg> (useful for tracing).
 *
 * Comments: lines starting with '#' are ignored.
 * Blank lines are ignored.
 *
 * Virtual addresses must be in the range 0x000 – 0xFFF (12-bit).
 * They may be written as decimal (e.g. 256) or hex (e.g. 0x100).
 */

/* Initialise the CPU (clears registers) */
void initCpu(void);

/* Load program from file, then execute it instruction by instruction */
void cpuRun(const char *filename);

/* Print current register values */
void printRegisters(void);

#endif /* CPU_H */
