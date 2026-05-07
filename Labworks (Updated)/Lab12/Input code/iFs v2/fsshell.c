/* ----------------------------------------------------------------------------------------
	Command line shell for FS.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
	Summary: 
		Mount the file system.
		Read a command line, call the corresponding FS function.
		Unmount the file system.
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef FS_H
#include "fs.h"
#endif

#define MAXLEN		256
#define MAXPARAMS	10
#define NUMCOMMANDS	( sizeof(commandList)/sizeof(command_t) )

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#include "cmdline_editor.c"

typedef struct {
	char *pCommand;
	int (*fpFunction)(int, char **);
} command_t;

int cmd_cat(int argc, char* argv[]);
int cmd_checkdisk(int argc, char* argv[]);
int cmd_cd(int argc, char* argv[]);
int cmd_dir(int argc, char* argv[]);
int cmd_exit(int argc, char* argv[]);
int cmd_format(int argc, char* argv[]);
int cmd_badFormat(int argc, char* argv[]);
int cmd_help(int argc, char* argv[]);
int cmd_import(int argc, char* argv[]);
int cmd_mkdir(int argc, char* argv[]);
int cmd_readInode(int argc, char* argv[]);
int cmd_inodeTable(int argc, char* argv[]);
int cmd_ln(int argc, char* argv[]);
int cmd_bun_cha(int argc, char* argv[]);
int cmd_readBlock(int argc, char* argv[]);
int cmd_rm(int argc, char* argv[]);
int cmd_rmdir(int argc, char* argv[]);
int cmd_superBlock(int argc, char* argv[]);
int cmd_undelete(int argc, char* argv[]);

static void flattenPath(char *pPath);

int shutdown=0;			// shutdown the file system
int mounted=0;			// file system is mounted
char cmdLine[MAXLEN];	// buffer for command line input
char currentDir[MAXLEN];// current working dir

command_t commandList[]={
	{"cat", &cmd_cat},
	{"checkdisk", &cmd_checkdisk},
	{"cd", &cmd_cd},
	{"dir", &cmd_dir},
	{"exit", &cmd_exit},
	{"format", &cmd_format},
	{"badformat", &cmd_badFormat},
	{"help", &cmd_help},
	{"import", &cmd_import},
	{"readinode", &cmd_readInode},
	{"inodetable", &cmd_inodeTable},
	{"ln", &cmd_ln},
	{"ls", &cmd_dir},
	{"mkdir", &cmd_mkdir},
	{"bunchaobama", &cmd_bun_cha},
	{"readblock", &cmd_readBlock},
	{"rm", &cmd_rm},
	{"rmdir", &cmd_rmdir},
	{"superblock", &cmd_superBlock},
	{"undelete", &cmd_undelete}
};

int main(void)
{	int argc=0, i, ret;
	char* argv[MAXPARAMS];
	time_t seconds;
	struct tm* pCurrentTime;

	// Mount file system
	ret = fs_mount (DISKNAME);
	if (ret != 0) {
		if (ret==-2) printf ("Could not mount '%s'. Please reformat the disk.\n", DISKNAME); 
		else {
			printf ("Virtual disk '%s' not found. Run makedisk first.\n", DISKNAME); 
			exit(-1); 
		}
	}
	else {
		mounted = 1;
		printf ("Filesystem '%s' mounted\n", DISKNAME); 
	}
		
	// print welcome message
	seconds=time(NULL);
	pCurrentTime=localtime(&seconds); 
	
	printf("FS Shell MAT3501-%d. Type help for the list of commands.\n",pCurrentTime->tm_year + 1900);
	
	do {
		// read the command line
		printf(CYN "%s" RESET" $ ", currentDir);
		// fgets(cmdLine, MAXLEN, stdin);
		fflush(stdout); 
		readLine(cmdLine, MAXLEN);
		
		// split parameters
		i=0;
		argv[i] = strtok(cmdLine, " \t\r\n");
		if (argv[i]==NULL) continue;
		while ((argv[i]!=NULL)&&(i<MAXPARAMS-1)) {
			i++;
			argv[i] = strtok(NULL, " \t\r\n");
		}
		argc = i;
		// search for the command in commandList
		for (i=0; i<NUMCOMMANDS; i++) {
			if ( strcmp(argv[0], commandList[i].pCommand)==0 ) {
				// call the corresponding file system function
				(*commandList[i].fpFunction)(argc, argv);
				break;
			}
		}
		if (i==NUMCOMMANDS) fprintf(stderr, "Command not found\n");
		
	} while (!shutdown);
	
	// unmount file system
	fs_umount();
	mounted = 0;
	printf ("Filesystem '%s' unmounted\n", DISKNAME);
}


int cmd_cat(int argc, char* argv[])
{	unsigned char buf[BLOCKSIZE];
	dirRecord_t dirRecord;
	int fs_fd, dirFd, n, i, size, found=0; 
	char pathName[MAXLEN], *pFileName, fileName[MAXLEN];
	
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=2) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}

	if (argv[1][0]=='/') strcpy(pathName, argv[1]); // absolute path
	else {
		strcpy(pathName, currentDir);
		strcat(pathName,"/");
		strcat(pathName, argv[1]);
	}
	pFileName = splitPathAndName(pathName);
	dirFd = fs_open(pathName);
	if (dirFd<0) return(-2);

	// read file content, wildcards support
	while (fs_searchDirByName(dirFd, pFileName, &dirRecord, found)) {
		found = 1;
		strcpy(fileName, pathName);
		strcat(fileName, "/");
		strcat(fileName, dirRecord.fileName);
		fs_fd = fs_open(fileName);
		if (fs_fd<0) { fprintf(stderr,"Cannot open file. Error = %d\n", fs_fd); return(-3);}
		size = fs_fileSize(fs_fd);
		
		while (size>=BLOCKSIZE) {
			n = fs_read(fs_fd, buf, BLOCKSIZE); 
			size -= BLOCKSIZE;
			for (i=0; i<n; i++) printf("%c", buf[i]);
		}
		n = fs_read(fs_fd, buf, size); 
		for (i=0; i<n; i++) printf("%c", buf[i]);
		printf("\n");
			
		fs_close(fs_fd);
	}
	fs_close(dirFd);
	
	return 0;
}

int cmd_checkdisk(int argc, char* argv[])
{	
	fs_checkDisk();
	return 0;
}


int cmd_cd(int argc, char* argv[])
{	int ret;
	char tmpStr[MAXLEN];
		
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=2) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}

	if (argv[1][0]=='/') strcpy(tmpStr, argv[1]);	// absolute path
	else {
		strcpy(tmpStr, currentDir);
		strcat(tmpStr,"/");
		strcat(tmpStr, argv[1]);
	}
	flattenPath(tmpStr);

	ret = fs_setCurrentDir(tmpStr);
	if (ret==0) strcpy(currentDir, tmpStr);
	else {
		printf("Error = %d\n", ret);
		ret = fs_setCurrentDir(currentDir);
		return (-2);	// invalid path
	}		
	return 0;
}


int cmd_dir(int argc, char* argv[])
{
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	fs_printDir(argv[1]);
	return 0;
}

int cmd_exit(int argc, char* argv[])
{
	shutdown = 1;
	return 0;
}

int cmd_format(int argc, char* argv[])
{	FILE* fp;
	int size, ret; 
	
	/* // unmount file system
	fs_umount(); 
	mounted = 0;
	printf ("Filesystem %s unmounted\n", DISKNAME); */
	
	fp = fopen(DISKNAME, "r"); 
	if (fp==NULL) {
		printf("No disk. Please run createdisk first\n");
	}
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fclose(fp); 

	ret = fs_format(DISKNAME, size); 
	if (ret<0) {
		printf("Could not format disk. Error = %d\n", ret);
		return (-1);
	}
	printf("Format complete. Disk size = %dKB, %d blocks, block size = %dB\n", size/1024, size/BLOCKSIZE, BLOCKSIZE);	
	
	// Mount file system
	if (fs_mount (DISKNAME) != 0) {
		printf ("Could not mount disk %s\n", DISKNAME); 
		exit(-1); 
	}
	mounted = 1;
	printf ("Filesystem %s mounted\n", DISKNAME); 
	
	// reset current dir to roor
	currentDir[0]=0;
	
	return 0;
}

int cmd_badFormat(int argc, char* argv[])
{	FILE* fp;
	int size, ret; 
	
	/* // unmount file system
	fs_umount(); 
	mounted = 0;
	printf ("Filesystem %s unmounted\n", DISKNAME); */
	
	fp = fopen(DISKNAME, "r"); 
	if (fp==NULL) {
		printf("No disk. Please run makedisk first\n");
	}
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	fclose(fp); 

	ret = fs_badFormat(DISKNAME, size); 
	if (ret<0) {
		printf("Could not format disk. Error = %d\n", ret);
		return (-1);
	}
	printf("Bad format complete. Disk size = %dKB, %d blocks, block size = %dB\n", size/1024, size/BLOCKSIZE, BLOCKSIZE);	
	
	// Mount file system
	if (fs_mount (DISKNAME) != 0) {
		printf ("Could not mount disk %s\n", DISKNAME); 
		exit(-1); 
	}
	mounted = 1;
	printf ("Filesystem %s mounted\n", DISKNAME); 
	
	// reset current dir to root
	currentDir[0]=0;
	
	return 0;
}

int cmd_help(int argc, char* argv[])
{
	printf("List of FS commands:\n");
	printf("badformat		format the file system with errors on data blocks\n");
	printf("cat <file>		display content of a file\n");
	printf("cd <folder>		change working directory\n");
	printf("checkdisk		check and fix errors on data blocks\n");
	printf("dir [wildcards]		display content of the root directory\n");
	printf("exit			shutdown the file system and quit\n");
	printf("format			format the file system, all content will be lost\n");
	printf("import <file>		import an existing file into our file system\n");
	printf("readinode <no>		display content of inode <no>\n");
	printf("inodetable		display the inode table\n");
	printf("ln <source> <target>	create hardlink target from source file\n");
	printf("ln -s <source> <target>	create symbolic link target from source file\n");
	printf("ls [wildcards]		display content of the root directory\n");
	printf("mkdir <folder>		create a new directory\n");
	printf("readblock <no>		display content of block no\n");
	printf("rm <file>		remove a file\n");
	printf("rmdir <folder>		remove an empty directory\n");
	printf("superblock		display content of super block\n");
	printf("undelete		restore deleted files in current directory\n");
	return 0;
}

int cmd_import(int argc, char* argv[])
{	char buf[BLOCKSIZE];
	int fs_fd, size, freeSize, n, len; 
	FILE *fp;

	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=2) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}
	 if (strlen(argv[1])>MAXFILENAME) {
		fprintf(stderr,"Sorry, FS only support file name upto %d characters.\n", MAXFILENAME);
		return(-1);
	}
	
	// import an existing file into our file system
	fp = fopen(argv[1], "r");	// open existing file
	if (fp==NULL) { 
		fprintf(stderr,"Source file does not exist\n"); 
		return(-2);
	}
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	freeSize = fs_diskFreeBlocks()*BLOCKSIZE;
	if (freeSize < size) {
		printf("Not enough disk space! Free %d, request %d\n", freeSize, size);
		fclose(fp);
		return (-1);
	} 
	fseek(fp, 0L, SEEK_SET);
		
	fs_fd = fs_create(argv[1]);
	if (fs_fd<0) {
		switch (fs_fd) {
		case -1: fprintf(stderr, "File already exists\n"); break;
		case -2: fprintf(stderr, "Invalid path\n"); break;
		case -3: fprintf(stderr, "Path does not exists\n"); break;
		case -4: fprintf(stderr, "Inode table full\n"); break;
		}
		fprintf(stderr, "Could not create file. Error = %d\n", fs_fd);
		return(-1);
	}
	fs_fd = fs_open(argv[1]);
	if (fs_fd<0) { 
		fprintf(stderr,"Cannot open file %s in FS. Error = %d\n",argv[1], fs_fd); 
		return(-2);
	}
	
	while (!feof(fp)) {
		n = fread(buf, BLOCKSIZE, 1, fp);
		if (n==1) len = BLOCKSIZE;
		else len = size % BLOCKSIZE;
		n = fs_write(fs_fd, buf, len);
		if (n<0) {
			switch (n) {
			case -1: fprintf(stderr, "Out of disk space\n"); break;
			case -2: fprintf(stderr, "File exceeds max size\n"); break;
			}
			fs_close(fs_fd);
			return (-1);
		}
	}
	
	if (fs_close(fs_fd)<0) printf("Fail to close fsf file\n");
	fclose(fp);
	
	//fs_printDir(NULL);
	
	return 0;
}

int cmd_readInode(int argc, char* argv[])
{	int inodeNo;

	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=2) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}
	
	inodeNo = atoi(argv[1]);
	fs_printInode(inodeNo);
	
	return 0;
}

int cmd_inodeTable(int argc, char* argv[])
{
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	fs_printInodeTable();
	return 0;
}

int cmd_ln(int argc, char* argv[])
{	char sourceName[128], targetName[128];
	int ret;
	
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	
	switch (argc) {
	case 3: // ln <source> <target>
		if ((strlen(argv[1])>MAXFILENAME)||(strlen(argv[2])>MAXFILENAME)) {
			fprintf(stderr,"Sorry, SFS only support file name upto %d characters.\n", MAXFILENAME);
			return(-1);
		}
		strcpy (sourceName, argv[1]);
		strcpy (targetName, argv[2]);
		
		if ( (strstr(sourceName, "*")!=NULL)||
			 (strstr(sourceName, "?")!=NULL)||
			 (strstr(targetName, "*")!=NULL)||
			 (strstr(targetName, "?")!=NULL) ) break; // wildcards not allowed
		
		ret = fs_createHardLink(sourceName, targetName);
		if (ret<0) fprintf(stderr, "Could not create hard link\n");
		
		//fs_printDir(NULL);
		return 0;
	case 4:
		// ln -s <source> <target>
		if (strcmp(argv[1], "-s")==0) {
			 if ((strlen(argv[2])>MAXFILENAME)||(strlen(argv[3])>MAXFILENAME)) {
				fprintf(stderr,"Sorry, SFS only support file name upto %d characters.\n", MAXFILENAME);
				return(-1);
			}
			strcpy (sourceName, argv[2]);
			strcpy (targetName, argv[3]);
			
			if ( (strstr(sourceName, "*")!=NULL)||
			 (strstr(sourceName, "?")!=NULL)||
			 (strstr(targetName, "*")!=NULL)||
			 (strstr(targetName, "?")!=NULL) ) break; // wildcards not allowed
			
			ret = fs_createSymLink(sourceName, targetName);
			if (ret<0) fprintf(stderr, "Could not create symbolic link. Error=%d\n", ret);
			
			// fs_printDir(NULL);
			return 0;
		}
	}
	fprintf(stderr, "Incorrect arguments\n");
	return (-1);
}


/* int cmd_makeDisk(int argc, char* argv[])
{	int power, size ,ret, err=0;
	
	if (argc!=2) {
		printf("usage: %s <power2size>\n", argv[0]);
		return(-1);
	}
	
	power = atoi(argv[1]); 
	
	ret = createDisk(DISKNAME, power);
	switch (ret) {
	case  0: 
		size = 1 << power; 
		printf ("Created virtual disk '%s' of size=%dKB, %d blocks, block size = %dB\n", 
			DISKNAME, size/1024, size/BLOCKSIZE, BLOCKSIZE); 
		err = 0;
		break;
	case -1: 
		printf("<power2size> must be between %d and %d\n", MINPOWER2SIZE, MAXPOWER2SIZE); 
		err = -1;
		break;
	case -2:
		printf ("Could not create virtual disk\n"); 
		err = -1; 
		break;
	case -3:
		printf ("Could not write blank blocks to virtual disk\n"); 
		err = -1;
		break;
	}
	
	return(err);
} */

int cmd_mkdir(int argc, char* argv[])
{	int i;
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=2) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}
	
	i = fs_makeDir(argv[1]);
	if (i<0) {
		switch (i) {
		case -1: fprintf(stderr, "Folder already exists\n"); break;
		case -2: fprintf(stderr, "Invalid path\n"); break;
		case -3: fprintf(stderr, "Path does not exists\n"); break;
		case -4: fprintf(stderr, "Inode table full\n"); break;
		}
		fprintf(stderr, "Could not create file. Error = %d\n", i);
		return(-1);
	}
	return 0;
}

#define LINEWIDTH	16
int cmd_readBlock(int argc, char* argv[])
{	unsigned char buf[BLOCKSIZE], text[LINEWIDTH+1];
	int i, n, tmp; 
	
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=2) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}
	
	// read block content
	n = atoi(argv[1]);
	tmp = readBlock(n, buf);
	if (tmp<0) { fprintf(stderr, "Could not read block\n"); return (-3);}
	memset(text,0,LINEWIDTH); // set the string empty
	for (i=0; i<=LINEWIDTH; i++) text[i]=0;
	for (i=0; i<BLOCKSIZE; i++) {
		if (i%LINEWIDTH==0) { printf("\t%s\n",text); printf("%04X\t",i);}
		printf("%02X ", buf[i]);
		if (buf[i]<32) text[i%LINEWIDTH]='.'; else text[i%LINEWIDTH]=buf[i];
	}
	printf("\t%s\n", text);
	
	return 0;
}

int cmd_rm(int argc, char* argv[])
{	int ret;

	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=2) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}
	if (fs_isDir(argv[1])) {
		fprintf(stderr, "Cannot remove a directory. Use rmdir instead.\n");
		return(-1);
	}

	ret = fs_delete(argv[1]);
	if (ret<0) 
		fprintf(stderr,"Cannot remove file(s). Error = %d\n", ret);
	else printf("File is removed.\n");
	//fs_printDir(NULL);

	return 0;
}

int cmd_rmdir(int argc, char* argv[])
{	int ret;

	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=2) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}
	
	ret = fs_isDir(argv[1]);
	if (ret>=2) {
		if (ret==2) {
			ret = fs_delete(argv[1]);
			if (ret==0) return 0;
			else {
				fprintf(stderr, "Could not remove file. Error = %d\n", ret);
				return (-3);
			}
		}
		else fprintf(stderr, "Directory is not empty\n");
	}
	else fprintf(stderr, "Not a directory\n");
	
	return (-1);
}

int cmd_superBlock(int argc, char* argv[])
{
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=1) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}
	
	fs_printSuperBlock();
	return 0;
}

int cmd_undelete(int argc, char* argv[]) 
{	int ret;
	if (!mounted) {
		fprintf(stderr, "File system is not mounted. Please format the disk.\n");
		return (-2);
	}
	if (argc!=1) {
		fprintf(stderr, "Incorrect arguments\n");
		return(-1);
	}
	
	ret = fs_undelete();
	if (ret<0) {
		fprintf(stderr, "Undelete fails\n");
		return ret;
	}
	return 0;
}

int cmd_bun_cha(int argc, char* argv[])
{
	printf("Mon nguyên lý hệ điều hành có ngon không?\n");
	return 0;
}


/* trim off '..' and '.' out of a full path from root */
static void flattenPath(char *pPath)
{	char pathName[MAXLEN];
	char* argv[MAXLEN];
	int i, j, argc=0;

	if (pPath[0]=='/') strcpy(pathName, pPath);
	else {
		strcpy(pathName, currentDir);
		strcat(pathName,"/");
		strcat(pathName, pPath);
	}
	
	i=0;
	argv[i] = strtok(pathName, "/\r\n");
	if (argv[i]==NULL) {
		pPath[0] = 0;
		return;
	}
	while ((argv[i]!=NULL)&&(i<MAXLEN-1)) {
		i++;
		argv[i] = strtok(NULL, "/\r\n");
	}
	argc = i;
	
	i=1;
	while (i<argc) {
		if (strcmp(argv[i],"..")==0) {
			for (j=i+1; j<argc; j++) argv[j-2] = argv[j];
			argc-=2;
		}

		if (strcmp(argv[i-1],".")==0) {
			for (j=i; j<argc; j++) argv[j-1] = argv[j];
			argc--;
		}
		i++;
	}

	if (strcmp(argv[0],"..")==0) {
		for (j=1; j<argc; j++) argv[j-1] = argv[j];
			argc--;
	}

	if (argc>=2 && strcmp(argv[argc-1],"..")==0) argc-=2;
	if (argc>=1 && strcmp(argv[argc-1],".")==0) argc--;
		
	pPath[0] = 0;

	for (i=0; i<argc; i++) {
		strcat(pPath, "/");
		strcat(pPath, argv[i]);
	}
}