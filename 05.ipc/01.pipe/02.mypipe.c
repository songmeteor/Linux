#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#define MSGSIZE	64

int main(void){
	int pd1[2], pd2[3];
	pid_t ret;
	time_t timer1;
	char sbuf[MSGSIZE];
	char rbuf[MSGSIZE];
	int i, len;
	
	if(pipe(pd1) == -1){
		perror("pipe1");
		exit(1);
	}
	
	if(pipe(pd2) == -1){
		perror("pipe2");
		exit(1);
	}
	
	
	switch(fork()){
		case 0:					//child
			close(pd1[0]);
			close(pd2[1]);
			for(i=0; i<5; i++) {
				sprintf(&sbuf[0], "Hello, Parent #%d", i+1);
				write(pd1[1], sbuf, strlen(sbuf));
				
				memset(&rbuf[0], 0, MSGSIZE);
				len = read(pd2[0], rbuf, MSGSIZE);
				if(len == 0){
					break;
				}
				printf("CHILD: %s\n", rbuf);
				for(timer1=time(NULL); time(NULL)<timer1 + 1;)
					continue;
			}
			exit(0);
		default:				//parent
			close(pd1[1]);
			close(pd2[0]);
			for(i=0; ; i++) {
				memset(&rbuf[0], 0, MSGSIZE);
				len = read(pd1[0], rbuf, MSGSIZE);
				if(len == 0){
					break;
				}
				printf("PARENT: %s\n", rbuf);
				for(timer1=time(NULL); time(NULL)<timer1 + 1;)
					continue;
				sprintf(&sbuf[0], "Welcome, Child #%d", i+1);
				write(pd2[1], sbuf, strlen(sbuf));
			}
	}
	return 0;
}