#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static void handler (int signo){
	printf ("[%d] signal is caughted\n", signo);
}

int main (void){
	signal(SIGRTMIN, handler);
	signal(SIGRTMIN+1, handler);
	for (;;){
		for(int i=0; i<5; i++){
			printf("I am running  --- %d\n", i);
			sleep(1);
		}
		printf("Now I am waiting for a signal --- ");
		pause();
	}
	return 0;
}
