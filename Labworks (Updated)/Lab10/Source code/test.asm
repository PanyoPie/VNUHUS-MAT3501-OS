# -- Phase 1: Cold-start – load 4 pages (fills all 4 frames) ----------

LOAD  R0  0x000
LOAD  R1  0x100
LOAD  R2  0x200
LOAD  R3  0x300

# -- Phase 2: Re-use pages already in memory (should be hits) ---------

LOAD  R0  0x001    # page 0, offset 1 – still in RAM
LOAD  R1  0x1FF    # page 1, offset 255 – still in RAM

# -- Phase 3: Write to a page (sets dirty bit) ------------------------

SET   R0  0x42     # R0 = 0x42 = 66 decimal
STORE R0  0x205    # write to page 2, offset 5

# -- Phasse 4: Trigger page replacement --------------------------------
# All 4 frames are occupied by pages 0-3.
# Accessing page 4 must evict one of them.

LOAD  R0  0x400    # page 4  – PAGE FAULT + REPLACEMENT
#LOAD  R1  0x500    # page 5  – PAGE FAULT + REPLACEMENT
