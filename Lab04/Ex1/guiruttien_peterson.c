#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#define TRUE 1
#define FALSE 0
#define DELAY	sleep(rand() % 4)

int account; // tai khoan ngan hang
int turn;
int interest[2];

void enter_region(int pid) {
	int other;
	other = 1 - pid;
	interest[pid] = TRUE;
	turn = pid;
	while ((turn == pid) && (interest[other] == TRUE));
}

void leave_region(int pid) {
	interest[pid] = FALSE;
}

void *chaGuiTien() {  // luot =0
	int x;
	while (TRUE) {
		enter_region(0);
		x = account;
		x += 2;
		account = x;
		leave_region(0);
		printf("Bo: Tao da gui. Account=%d\n", account);
	}
}

void *conRutTien() { // luot =1
	int y;
	while (TRUE) {
		enter_region(1); 
		y = account;
		y -= 3;
		if (y >= 0) {
			account = y;
			printf("Con: Con da rut. Account=%d\n", account);
		} else {
			printf("Con: Khong du tien\n");
		}
		leave_region(1);
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
