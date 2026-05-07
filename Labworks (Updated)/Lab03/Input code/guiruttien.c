// Gửi và rút tiền với race condition
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DELAY	sleep(rand() % 4)

int account; // tai khoan ngan hang

void *chaGuiTien() {  
	int x, i;
	for (i=0; i<5; i++) {
		...; DELAY;
		...; DELAY;
		...; DELAY;
		printf("Tao da gui. Account=%d\n", account);
	}
}

void *conRutTien() { 
	int y, i;
	for (i=0; i<5; i++) {
		...; DELAY;
		...; DELAY;
		if (y>=0) { 
			...; 
			printf("Con da rut. Account=%d\n", account);
		}
		else { printf("Khong du tien\n"); i--;}
		DELAY;
		
	}
}

void main() {
   pthread_t tid1, tid2;
   
   printf("Account=%d\n",account);
   pthread_create(...);   
   pthread_create(...);   
   
   pthread_join(...);  
   pthread_join(...);  
}
