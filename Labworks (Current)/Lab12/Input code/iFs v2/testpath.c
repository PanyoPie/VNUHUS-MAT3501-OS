#include <stdio.h>
#include <string.h>

#define MAXLEN 256

char currentDir[MAXLEN];

/* trim off '..' and '.' out of a path */
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

void main(int agrc, char* argv[])
{	char cmdLine[MAXLEN];

	strcpy(currentDir,"/");
	strcpy(cmdLine, argv[1]);
	flattenPath(cmdLine);
	printf("shorten path = %s\n", cmdLine);
}