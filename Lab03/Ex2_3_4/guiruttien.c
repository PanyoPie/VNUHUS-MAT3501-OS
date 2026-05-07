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
		x = account; DELAY;
		x += 3; DELAY;
		account = x; DELAY;
		printf("Tao da gui. Account=%d\n", account);
	}
}

void *conRutTien() { 
	int y, i;
	for (i=0; i<5; i++) {
		y = account; DELAY;
		y -= 2; DELAY;
		if (y>=0) { 
			account = y; 
			printf("Con da rut. Account=%d\n", account);
		}
		else { printf("Khong du tien\n"); i--;}
		DELAY;
		
	}
}

void main() {
   pthread_t tid1, tid2;
   
   printf("Account=%d\n",account);
   pthread_create(&tid1, NULL, chaGuiTien, NULL);   
   pthread_create(&tid2, NULL, conRutTien, NULL);   
   
   pthread_join(tid1, NULL);  
   pthread_join(tid2, NULL);
}
