all: ramEmu.o virtualMem.o fifo.o clock.o lru.o main
ramEmu.o:	config.h ramEmu.h ramEmu.c
	gcc -c ramEmu.c
virtualMem.o:	config.h ramEmu.h virtualMem.h virtualMem.c
	gcc -c virtualMem.c
fifo.o:	config.h ramEmu.h fifo.h fifo.c
	gcc -c fifo.c
clock.o:	config.h ramEmu.h clock.h clock.c
	gcc -c clock.c
lru.o:	config.h ramEmu.h lru.h lru.c
	gcc -c lru.c
main:	config.h ramEmu.h virtualMem.h main.c ramEmu.o virtualMem.o fifo.o clock.o lru.o
	gcc -o main main.c ramEmu.o virtualMem.o fifo.o clock.o lru.o
	