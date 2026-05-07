# --------------------------------------------------
# Makefile – Virtual Memory Simulator
# Usage:
#   make          build the simulator
#   make run      run with default demo program (FIFO, verbose)
#   make run_lru  run with LRU replacement
#   make run_clk  run with CLOCK replacement
#   make clean    remove build artefacts
# --------------------------------------------------

CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c99

TARGET  = vmsim
SRCS    = main.c cpu.c mmu.c page_supervisor.c replacement.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all run run_lru run_clk clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Dependencies (header changes trigger recompile)
main.o:            main.c config.h cpu.h mmu.h page_supervisor.h replacement.h
cpu.o:             cpu.c  cpu.h  config.h mmu.h
mmu.o:             mmu.c  mmu.h  config.h page_supervisor.h replacement.h
page_supervisor.o: page_supervisor.c page_supervisor.h config.h replacement.h
replacement.o:     replacement.c replacement.h config.h

run: $(TARGET)
	./$(TARGET) -v -a fifo program.asm

run_lru: $(TARGET)
	./$(TARGET) -v -a lru program.asm

run_clk: $(TARGET)
	./$(TARGET) -v -a clock program.asm

clean:
	rm -f $(OBJS) $(TARGET)
