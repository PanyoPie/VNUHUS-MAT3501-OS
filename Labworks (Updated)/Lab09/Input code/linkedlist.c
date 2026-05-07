/* ----------------------------------------------------------------------------------------
	Memory allocation simulation using linked list.
	Course: MAT3501 - Principles of Operating Systems, MIM - HUS
------------------------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>

#define PAGESIZE	4096
#define NUMPAGES	256

typedef struct linkListNode {
	char processName;			//	a to z or A to Z
	char label;					//	P or H
	unsigned int startPage;
	unsigned int numPages;
	struct linkListNode *pNext;
} linkListNode_t;

typedef struct linkList {
	int numNodes;
	linkListNode_t *pFirstNode;
	linkListNode_t *pLastNode;
	linkListNode_t *pCurrentNode;
} linkList_t;

static linkList_t memList;

/* Link list functions ----------------------------------------------------------------*/

/* 
Add a node to the end of the list
Params: 
	pList - pointer to the link list
	name - process name, from 'a' to 'z' or 'A' to 'Z'
	label - 'P' for process, 'H' for hole
	start - start page of the memory area
	len - size of the memory area, in term of number of pages
Return:
	0  - successful
	-1 - unable to allocate memory for a new node
*/
int addNode(linkList_t *pList, char name, char label, unsigned int start, unsigned len)
{	linkListNode_t *pNode;
	
	pNode=malloc(sizeof(linkListNode_t));
	if (pNode==NULL) return (-1);	// fail to create node
	pNode->processName = name;
	pNode->label = label;
	pNode->startPage = start;
	pNode->numPages = len;
	pNode->pNext = NULL;
	
	if (pList->numNodes==0) {
		pList->pFirstNode=pNode;
		pList->pCurrentNode=pNode;
	}
	else pList->pLastNode->pNext = pNode;
	pList->pLastNode = pNode;
	pList->numNodes++;
	return 0;
}

/*
insert a new node before the current node
Params:
	pList - pointer to the link list
	name - process name, from 'a' to 'z' or 'A' to 'Z'
	label - 'P' for process, 'H' for hole
	start - start page of the memory area
	len - size of the memory area, in term of number of pages
Return:
	0  - successful
	-1 - unable to allocate memory for a new node
*/
int insertNode(linkList_t *pList, char name, char label, unsigned int start, unsigned len)
{	linkListNode_t *pNode;
	
	// make sure the current pointer has to point to an existing node
	if (pList->pCurrentNode == NULL) return (-1); 
	
	pNode=malloc(sizeof(linkListNode_t));
	if (pNode==NULL) return (-1);	// fail to create node
	
	// copy the current node to the new node
	pNode->processName = pList->pCurrentNode->processName;
	pNode->label = pList->pCurrentNode->label;
	pNode->startPage = pList->pCurrentNode->startPage;
	pNode->numPages = pList->pCurrentNode->numPages;
	pNode->pNext = pList->pCurrentNode->pNext;
	
	// overwrite current node with new content
	pList->pCurrentNode->processName = name;
	pList->pCurrentNode->label = label;
	pList->pCurrentNode->startPage = start;
	pList->pCurrentNode->numPages = len;
	pList->pCurrentNode->pNext = pNode;
	
	if (pList->pLastNode == pList->pCurrentNode) 
		pList->pLastNode = pList->pCurrentNode->pNext;
	pList->numNodes++;
	return 0;
}

/*
Get the current node
Params:
	pList - pointer to the link list
Return:
	pointer to the current node, NULL if the list has no nodes
*/
linkListNode_t* getCurrentNode(linkList_t *pList)
{
	if (pList == NULL) return NULL;
	if (pList->pCurrentNode == NULL) return NULL;
	return (pList->pCurrentNode);
}

/*
Move current node to the next node
Params:
	pList - pointer to the link list
Return:
	0  - sucessful
	-1 - if pList is NULL
*/
int nextNode(linkList_t *pList)
{
	if (pList == NULL) return (-1);
	if (pList->pCurrentNode == NULL) return (-1);
	pList->pCurrentNode = pList->pCurrentNode->pNext;
	return 0;
}

/*
write data to the current node
Params:
	pList - pointer to the link list
	name - process name, from 'a' to 'z' or 'A' to 'Z'
	label - 'P' for process, 'H' for hole
	start - start page of the memory area
	len - size of the memory area, in term of number of pages
Return:
	0  - successful
	-1 - either pList or current node is NULL
*/	

int updateCurrentNode(linkList_t *pList, char name, char label, unsigned int start, unsigned len)
{
	if (pList == NULL) return (-1);
	if (pList->pCurrentNode == NULL) return (-1);
	pList->pCurrentNode->processName = name;
	pList->pCurrentNode->label = label;
	pList->pCurrentNode->startPage = start;
	pList->pCurrentNode->numPages = len;
	return 0;
}

/*
delete the current node
Params:
	pList - pointer to the link list
Return:
	0  - sucessful
	-1 - either pList or current node is NULL
*/
int deleteNode(linkList_t *pList)
{	linkListNode_t* pOldCurrent;

	if (pList == NULL) return (-1);
	if (pList->pCurrentNode == NULL) return (-1);
	
	pOldCurrent = pList->pCurrentNode;
	if (pList->pCurrentNode == pList->pFirstNode) {	// delete 1st node
		pList->pFirstNode = pList->pCurrentNode->pNext;
		if (pList->pCurrentNode == pList->pLastNode)// also is the last node
		pList->pLastNode = pList->pCurrentNode->pNext;
	}
	else {		
		pList->pCurrentNode = pList->pFirstNode;
		while (pList->pCurrentNode != NULL) {
			if (pList->pCurrentNode->pNext == pOldCurrent) break;
			pList->pCurrentNode = pList->pCurrentNode->pNext;
		}
		// current node is the previous node of the old current node 
		pList->pCurrentNode->pNext = pOldCurrent->pNext;
		if (pList->pLastNode == pOldCurrent) // delete the last node
			pList->pLastNode = pList->pCurrentNode;
	}
	pList->numNodes--;
	free(pOldCurrent);	// remove the old current node
}

/*
search list for a given process name
Params:
	pList - pointer to the link list
	name - process name 
Return:
	0  - sucessful
	-1 - pList is NULL
	-2 - not found
*/
int searchListByProcessName(linkList_t *pList, char name)
{	
	if (pList == NULL) return (-1);

	pList->pCurrentNode = pList->pFirstNode;
	while (pList->pCurrentNode != NULL) {
		if (pList->pCurrentNode->processName == name) break;
		pList->pCurrentNode = pList->pCurrentNode->pNext;
	}
	if (pList->pCurrentNode == NULL) return (-2);
	else return 0;
}

/*
search list for free memory pages, using first fit algo
Params:
	pList - pointer to the link list
	pages - number of free pages requested
Return:
	0  - sucessful, pCurrent wil point to the found node
	-1 - pList is NULL
	-2 - memory is full, the request can not be fulfilled
*/
int firstFitSearch(linkList_t *pList, int pages)
{
	// write your code here
	// ...
}

/*
search list for free memory pages, using next fit algo
Params:
	pList - pointer to the link list
	pages - number of free pages requested
Return:
	0  - sucessful, pCurrent wil point to the found node
	-1 - pList is NULL
	-2 - memory is full, the request can not be fulfilled
*/
int nextFitSearch(linkList_t *pList, int pages)
{	linkListNode_t* pCurrent; 

	// write your code here
	if (pList == NULL) return (-1);
	pCurrent = pList->pCurrentNode; // remember current node

	while (pList->pCurrentNode != NULL) {
		if ( (pList->pCurrentNode->label=='H')&&
			(pList->pCurrentNode->numPages >= pages)) break;
		pList->pCurrentNode = pList->pCurrentNode->pNext;
	}
	if (pList->pCurrentNode != NULL) return 0; // found

	// if not found, resume searching from the beginning
	pList->pCurrentNode = pList->pFirstNode; 
	while (pList->pCurrentNode!=pCurrent) {		// search till the current position
		if ( (pList->pCurrentNode->label=='H')&&
			(pList->pCurrentNode->numPages >= pages)) break;
		pList->pCurrentNode = pList->pCurrentNode->pNext;
	}
	if (pList->pCurrentNode != pCurrent) return 0; // found
	
	return (-2); // memory is full
}

/*
search list for free memory pages, using best fit algo
Params:
	pList - pointer to the link list
	pages - number of free pages requested
Return:
	0  - sucessful, pCurrent wil point to the found node
	-1 - pList is NULL
	-2 - memory is full, the request can not be fulfilled
*/
int bestFitSearch(linkList_t *pList, int pages)
{	linkListNode_t* pNode=NULL;
	int minPages = NUMPAGES + 1;

	// write your code here
	if (pList == NULL) return (-1);
	pList->pCurrentNode = pList->pFirstNode;
	while (pList->pCurrentNode != NULL) {
		if ( (pList->pCurrentNode->label=='H')&&
			(pList->pCurrentNode->numPages >= pages)&&
			(pList->pCurrentNode->numPages < minPages) ) {
			minPages = pList->pCurrentNode->numPages;
			pNode = pList->pCurrentNode;	// remember the position found
		}
		pList->pCurrentNode = pList->pCurrentNode->pNext;
	}
	if (pNode != NULL) {
		pList->pCurrentNode = pNode;  // set current node to the min node found
		return 0;
	}
	else return (-2); // memory is full
}

/*
search list for free memory pages, using worst fit algo
Params:
	pList - pointer to the link list
	pages - number of free pages requested
Return:
	0  - sucessful, pCurrent wil point to the found node
	-1 - pList is NULL
	-2 - memory is full, the request can not be fulfilled
*/
int worstFitSearch(linkList_t *pList, int pages)
{	linkListNode_t* pNode=NULL;
	int maxPages = 0;

	// write your code here
	// ...
}


/*
print content of all list nodes
Params:
	pList - pointer to the link list
*/
void printList(linkList_t *pList)
{	linkListNode_t *pNode;

	if (pList==NULL) return;
	printf("|");
	
	// remember current node
	pNode = pList->pCurrentNode;
	
	pList->pCurrentNode = pList->pFirstNode;
	while (pList->pCurrentNode!=NULL) {
		printf(" %c %c %3d %2d |", pList->pCurrentNode->processName, pList->pCurrentNode->label, pList->pCurrentNode->startPage, pList->pCurrentNode->numPages);
		pList->pCurrentNode = pList->pCurrentNode->pNext;
	}
	printf("\n");
	
	// restore current node
	pList->pCurrentNode = pNode;
}


/* Process management functions -------------------------------------------------------------------------------*/

/*
allocate free space and load a process into memory
Params:
	name  - process name
	pages - number of free pages requested by the process
Return:
	0  - successful
	-1 - the process is already in memory, no action
	-2 - unable to load, memory is full
	-3 - fail to create a new link list node
*/
int loadProcess(char name, unsigned int pages)
{	linkListNode_t *pNode;
	int ret;
	
	pNode = memList.pCurrentNode;
	ret = searchListByProcessName(&memList, name);
	memList.pCurrentNode = pNode;	// restore the current position after searching
	if (ret==0) return (-1);		// already loaded
	
	// use first/next/best/worst fit algo here
	ret = bestFitSearch(&memList, pages);
	
	if (ret<0) return(-2);					// memory is full
	pNode = getCurrentNode(&memList);
	if (pNode->numPages > pages) {	// split the node
		ret = insertNode(&memList, name, 'P', pNode->startPage, pages);
		if (ret<0) return (-3);				// fail to insert node
		// move to the original node and update it
		nextNode(&memList);
		pNode = getCurrentNode(&memList);
		updateCurrentNode(&memList, ' ', pNode->label, pNode->startPage + pages, pNode->numPages - pages);
	} else {
		updateCurrentNode(&memList, name, 'P', pNode->startPage, pNode->numPages);
	}

	return 0;
}

/*
unload a process from memory
Params:
	name  - process name
Return:
	0  - successful
	-1 - no such process name
*/
int unloadProcess(char name)
{	linkListNode_t* pNode;
	int i, tmp;

	tmp = searchListByProcessName(&memList,name);
	if (tmp<0) return (-1);		// not found
	
	pNode = getCurrentNode(&memList);
	updateCurrentNode(&memList, ' ', 'H', pNode->startPage, pNode->numPages);
	
	// reunite neighboring hole nodes
	memList.pCurrentNode = memList.pFirstNode;
	while ( (memList.pCurrentNode != NULL) && (memList.pCurrentNode->pNext != NULL) ) {
		if ((memList.pCurrentNode->label == 'H')&&(memList.pCurrentNode->pNext->label=='H')) {
			// unite the 2 neighboring nodes
			memList.pCurrentNode->numPages += memList.pCurrentNode->pNext->numPages;
			pNode = memList.pCurrentNode->pNext;
			memList.pCurrentNode->pNext = memList.pCurrentNode->pNext->pNext;
			memList.numNodes--;
			if (memList.pLastNode == pNode) memList.pLastNode = memList.pCurrentNode;
			free(pNode);	// delete the unused node
		}
		else memList.pCurrentNode = memList.pCurrentNode->pNext;
	}
	return 0;
}


// Memory allocation simulation

void main(void) 
{	int i, pages, err;
	char name;
	
	// Initiate 1st link list node
	addNode(&memList, ' ', 'H', 0, NUMPAGES);
	printList(&memList);
	
	printf("Instruction: input a letter for process name and number of pages, e.g a 10. Enter '!' to terminate\n");
	
	while (1) {
		scanf("%c", &name);
		if (name=='!') break;
		scanf("%d", &pages);
		
		if (((name >= 'a')&&(name<='z')) ||
			((name >= 'A')&&(name<='Z')) ) {
			if (pages>0) {
				err = loadProcess(name, pages);
				switch (err) {
				case -1: fprintf(stderr, "Process %d is already in memory\n", name); break;
				case -2: fprintf(stderr, "Memory is full\n"); break;
				case -3: fprintf(stderr, "Unable to create link list node\n"); break;
				}
			}
			else {
				err = unloadProcess(name);
				if (err<0) fprintf(stderr, "Unable to terminate process %c\n", name);
			}
			printList(&memList);
		}
	}
}