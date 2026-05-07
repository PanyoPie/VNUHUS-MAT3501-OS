all: diskemu.o fs.o cmd_makedisk cmd_format cmd_import cmd_rm cmd_cat cmd_readblock cmd_dir cmd_printfat

diskemu.o:	diskemu.c diskemu.h
	gcc -Wall -c diskemu.c

fs.o:	fs.c fs.h diskemu.h
	gcc -Wall -c fs.c

cmd_makedisk: cmd_makedisk.c diskemu.o
	gcc -Wall -o cmd_makedisk cmd_makedisk.c diskemu.o

cmd_format: cmd_format.c fs.o diskemu.o
	gcc -Wall -o cmd_format cmd_format.c diskemu.o fs.o

cmd_import: cmd_import.c fs.o diskemu.o
	gcc -Wall -o cmd_import cmd_import.c diskemu.o fs.o

cmd_rm: cmd_rm.c fs.o diskemu.o
	gcc -Wall -o cmd_rm cmd_rm.c diskemu.o fs.o

cmd_cat: cmd_cat.c fs.o diskemu.o
	gcc -Wall -o cmd_cat cmd_cat.c diskemu.o fs.o

cmd_readblock: cmd_readblock.c fs.o diskemu.o
	gcc -Wall -o cmd_readblock cmd_readblock.c diskemu.o fs.o

cmd_dir: cmd_dir.c fs.o
	gcc -Wall -o cmd_dir cmd_dir.c diskemu.o fs.o

cmd_printfat: cmd_printfat.c fs.o diskemu.o
	gcc -Wall -o cmd_printfat cmd_printfat.c diskemu.o fs.o

