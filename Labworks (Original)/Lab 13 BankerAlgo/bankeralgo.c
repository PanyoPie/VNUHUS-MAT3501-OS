/* Banker's algorithm - MAT3501 - Principles of Operating System, MIM - HUS
	gcc -o bankeralgo bankeralgo.cabs
	cat input1.txt | ./bankeralgo
*/
#include <stdio.h> 

#define MAXPROCESSES	5
#define MAXRESOURCES	5
#define MAXLEN			128

int numProcesses, numResources;
int allocation[MAXPROCESSES][MAXRESOURCES];
int max[MAXPROCESSES][MAXRESOURCES];
int available[MAXRESOURCES];
int need[MAXPROCESSES][MAXRESOURCES]; 
int satisfied[MAXPROCESSES], solution[MAXPROCESSES]; 

void printBankState(int);
void printRow(int *pArray);
	
int main(void) 
{	int i, j, k, l, found, count=0; 
	int oldAvailable[MAXRESOURCES];
	char str[MAXLEN];
    
	// reading from standard input
	
	fgets(str, MAXLEN-1, stdin);
	fscanf(stdin, "%d %d\n", &numProcesses, &numResources);
	if (numProcesses>MAXPROCESSES || numResources>MAXRESOURCES) {
		printf("Error: max processes > %d or max resources > %d\n", MAXPROCESSES, MAXRESOURCES);
		return(-1);
	}
	
	fgets(str, MAXLEN-1, stdin);
	for (i=0; i<numProcesses; i++) 
		for (j=0; j<numResources; j++) 
			fscanf(stdin, "%d ", &allocation[i][j]);
	
    fgets(str, MAXLEN-1, stdin);
	for (i=0; i<numProcesses; i++) 
		for (j=0; j<numResources; j++) 
			fscanf(stdin, "%d ", &max[i][j]);

	fgets(str, MAXLEN-1, stdin);
	for (j=0; j<numResources; j++) 
		fscanf(stdin, "%d ", &available[j]);
	
    for (i = 0; i < numProcesses; i++) { 
        for (j = 0; j < numResources; j++) 
            need[i][j] = max[i][j] - allocation[i][j]; 
    } 
	printf("Initial state\n");
	printBankState(MAXPROCESSES);
	
	// run banker's algo on the initial state
    do { // repeat until all processes are marked satisfied or no more allocation is possible
		found = 0;
        for (i = 0; i < numProcesses; i++) { 
            if (satisfied[i] == 0) { 
                for (j = 0; j < numResources; j++) { 
                    if (need[i][j] > available[j]) break;
                } 
  
                if (j==numResources) { // i.e need <= available
					found = 1;
                    solution[count++] = i; 
					// get back resources allocated to process i
                    for (k = 0; k < numResources; k++) {
						oldAvailable[k] = available[k];
                        available[k] += allocation[i][k]; 
					}
                    satisfied[i] = 1; 
					
					printf("-----------------------------------------------------------------\n");
					printf("For i=%d:\tNeed[%d] <= Available\n\t\t", i, i); 
					printRow(need[i]); printf(" <= "); printRow(oldAvailable); printf("\n");
					printf("\t\tAvailable += Allocation[%d]\n\t\t", i);
					printRow(available); printf(" = "); printRow(oldAvailable); printf(" + "); printRow(allocation[i]); printf("\n");
					printBankState(i);
					
					break;
                } 
            } 
        } 
    } while (count<numProcesses && found);
  
    if (found) {
		printf("The current state is safe. A possible allocation sequence is\n"); 
		for (i = 0; i < numProcesses; i++) 
			printf(" P%d ->", solution[i]); 
		printf("\b\b  \n"); 
	}
	else printf("The current state is not safe\n");
	return 0;
} 


void printBankState(int row)
{	int i, j;
	printf(" Process\tAllocation\t\tNeed\t\tSatisfied\n");
	printf(" -------\t----------\t\t----\t\t---------\n");
	for (i=0; i<numProcesses; i++) {
		if (i==row) printf("*"); else printf(" ");
		printf("%2d \t\t",i);
		for (j=0; j<numResources; j++)
			if (satisfied[i]) printf(" - ");
			else printf("%2d ", allocation[i][j]);
		printf("\t\t");
		for (j=0; j<numResources; j++)
			if (satisfied[i]) printf(" - ");
			else printf("%2d ", need[i][j]);
		printf("\t\t");
		if (satisfied[i]) printf("T\n");
		else printf("F\n");
	}
	printf("Available = (");
	for (j=0; j<numResources; j++) printf("%2d ", available[j]);
	printf(")\n\n");
}

void printRow(int *pArray)
{	int i;
	printf("(");
	for (i=0; i<numResources; i++)
		printf("%2d ", pArray[i]);
	printf(")");
}
