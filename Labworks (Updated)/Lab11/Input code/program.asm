# ---------------------------------------------------------------------
# program.asm – Virtual Memory Simulator demo program
#
# This program deliberately accesses pages spread across the 12-bit
# virtual address space so that, with only 4 physical frames, several
# page faults and at least one page replacement must occur.
#
# Virtual address format:
#   0xPOO  where P  = virtual page number (0-F, one hex digit)
#                OO = byte offset within the page (00-FF)
#
# Instructions supported:
#   LOAD  <reg> <addr>   – read byte from virtual address into register
#   STORE <reg> <addr>   – write register value to virtual address
#   SET   <reg> <value>  – load immediate value into register
#   PRINT <reg>          – display register value
# ---------------------------------------------------------------------

# -- Phase 1: Cold-start – load 4 pages (fills all 4 frames) ----------

# Access virtual page 0 (address 0x000 = page 0, offset 0)
LOAD  R0  0x000

# Access virtual page 1 (address 0x100 = page 1, offset 0)
LOAD  R1  0x100

# Access virtual page 2 (address 0x200 = page 2, offset 0)
LOAD  R2  0x200

# Access virtual page 3 (address 0x300 = page 3, offset 0)
LOAD  R3  0x300

PRINT R0
PRINT R1
PRINT R2
PRINT R3

# -- Phase 2: Re-use pages already in memory (should be hits) ---------

LOAD  R0  0x301    # page 3, offset 1 – still in RAM
LOAD  R1  0x1FF    # page 1, offset 255 – still in RAM

# -- Phase 3: Write to a page (sets dirty bit) ------------------------

SET   R0  0x42     # R0 = 0x42 = 66 decimal
STORE R0  0x205    # write to page 2, offset 5

# Verify the write by reading it back
LOAD  R2  0x205
PRINT R2            # should print 66

# -- Phase 4: Trigger page replacement --------------------------------
# All 4 frames are occupied by pages 0-3.
# Accessing page 4 must evict one of them.

LOAD  R0  0x400    # page 4  – PAGE FAULT + REPLACEMENT

# Access two more new pages to force more replacements
LOAD  R1  0x500    # page 5  – PAGE FAULT + REPLACEMENT
LOAD  R2  0x600    # page 6  – PAGE FAULT + REPLACEMENT

# -- Phase 5: Access an evicted page (demonstrates reload) ------------
# Depending on the algorithm, page 0, 1, or 2 may have been evicted.
# Accessing page 0 again will cause a fault if it was evicted.

LOAD  R3  0x000    # page 0 – may fault again
PRINT R3

# -- Phase 6: Mixed workload -------------------------------------------

LOAD  R0  0x700    # page 7
LOAD  R1  0x800    # page 8
SET   R2  0xFF
STORE R2  0x7A0    # write to page 7, offset 0xA0
LOAD  R3  0x7A0    # read it back
PRINT R3            # should print 255
