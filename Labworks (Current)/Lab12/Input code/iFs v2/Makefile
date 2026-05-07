all: diskemu.o fs.o makedisk fsshell

diskemu.o:	diskemu.c diskemu.h
	gcc -Wall -c diskemu.c

fs.o:	fs.c fs.h diskemu.h checkdisk.c
	gcc -Wall -c fs.c

makedisk: makedisk.c diskemu.o
	gcc -Wall -o makedisk makedisk.c diskemu.o

fsshell: fsshell.c fs.o diskemu.o
	gcc -Wall -o fsshell fsshell.c diskemu.o fs.o
