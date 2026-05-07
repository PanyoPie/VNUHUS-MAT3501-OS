//producer consumer using concurrent processes
// gcc -pthread <program.c> -lrt
// remember to put -lrt at the end

#include <stdio.h>             
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define DELAY(S)	sleep(rand() % S)

void chaGuiTien(int amount);
void conRutTien(int amount);

// shared memory
struct mySharedMem {
	int account;
} *pMem;

int main() {
	pid_t pid,cid;
	int shmfd;
	int status;
		
	// Create unsized shared memory object;
	shm_unlink ("mySharedMemObject");	// remove the object, in case it exists
	shmfd = shm_open ("mySharedMemObject", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
	assert (shmfd != -1);
	
	// Resize the region to store 1 struct instance
	assert (ftruncate (shmfd, sizeof (struct mySharedMem)) != -1);
	
	// Map the object into memory so file operations aren't needed
	pMem = mmap (NULL, sizeof (struct mySharedMem),PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	assert (pMem != MAP_FAILED);
	
	// Initialize shared data
	pMem->account = 0;
		
	// create a child process to run chaGuiTien()
	pid = fork();
	if (pid == 0) { // child process
		chaGuiTien(3);
		munmap (pMem, sizeof (struct mySharedMem)); // unmap the object
		return 0;
	}
	else { // parent process
		cid = fork();   // create a child process to run conRutTien()
		if (cid == 0) { // child process
			conRutTien(3); DELAY(3);
			munmap (pMem, sizeof (struct mySharedMem)); // unmap the object
			return 0;
		}
	}
	
	// parent process
	waitpid(pid, &status, 0);
	waitpid(cid, &status, 0);
	printf("Well done. Let's go home\n");
	munmap (pMem, sizeof (struct mySharedMem)); // unmap the object
	shm_unlink ("mySharedMemObject");	// remove the object
	return 0;
}

void chaGuiTien(int amount) {  
	int x, i;
	for (i=0; i<5; i++) {
		x = pMem->account;			// get the account value to local variable x
		DELAY(2);
		x += amount;			// increase x by amount
		DELAY(2);
		pMem->account = x;			// write the value of x back to the account
		DELAY(2);
		printf("Tao da gui. Account=%d\n", pMem->account);
	}
}

void conRutTien(int amount) { 
	int y, i;
	for (i=0; i<5; i++) {
		y = pMem->account;			// get the account value to local variable y
		DELAY(2);
		y -= amount;			// decrease y by amount
		DELAY(2);
		if (y>=0) { 
			pMem->account = y;		// write the value of y back to the account
			printf("Con da rut. Account=%d\n", pMem->account);
		}
		else { printf("Khong du tien\n"); i--;}
		DELAY(2);
		
	}
}
