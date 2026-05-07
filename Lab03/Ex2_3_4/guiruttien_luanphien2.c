/* Cha con luan phien voi tham so soTien */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DELAY	sleep(rand() % 4)

int account = 5;
int turn=0; // turn=0 luot Gui; =1 luot la Rut

void *guiTien(void* soTien) { 
	int x, soTienGui;
	soTienGui=*((int*) soTien);
	while (1) {
		while (turn!=0);
		x=account; DELAY;
		x=x+soTienGui; DELAY;
		account=x; DELAY;
		printf("Bo: Tao vua gui cho may %d\n",soTienGui);
		printf("Account = %d\n", account);
		turn=1;
		sleep(2);
	}
}

void *rutTien(void* soTien) { 
	int y, soTienRut;
	soTienRut=*((int*) soTien);
	while (1) {
		while (turn!=1);
		y=account; DELAY;
		y=y-soTienRut; DELAY;
		account=y; DELAY;
		printf("Con: vua rut %d\n",soTienRut);
		printf("Account = %d\n", account);
		turn=0;
		sleep(2);
	}
}
void main() {
   pthread_t tid1, tid2;
   int soTienRut=3;
   int soTienGui=4;
   pthread_create(&tid1, NULL, guiTien, &soTienGui);   
   sleep(1);
   pthread_create(&tid2, NULL, rutTien, &soTienRut);   
   
   pthread_join(tid1, NULL);  
   pthread_join(tid2, NULL);  
}
