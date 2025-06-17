#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

int main(void) {
	int pd[2], read_fd, write_fd;
	pid_t pid;
	time_t timer1, timer2;
	char tx_buf[100], rx_buf[100];
	
	if ( pipe(pd) == -1 ) {
		perror("pipe");    
		exit(1);    
	}
	
	read_fd = pd[0];    
	write_fd = pd[1];
  
	switch(pid=fork()) {
		case 0: 
			close(read_fd);
			for(int i=0; i<11; i++){
				// for(timer1=time(NULL); time(NULL)<timer1 + 1;)
				// continue;
				// strcpy(tx_buf, "\e[31mHello Parent. I am child.");
				sprintf(tx_buf, "\e[31mHello Parent. I am child ---%d\n", i);
				write(write_fd, tx_buf, strlen(tx_buf)+1);
				for(timer1=time(NULL); time(NULL)<timer1 + 1;)
				continue;
				// read(read_fd, rx_buf, sizeof(rx_buf));
				// printf("\e[00m--------> CHILD: %s\n", rx_buf);
			}
			exit(0);
		default:   
#if 1
			close(write_fd);
			for(int i=0; i<10; i++){
				// for(timer2=time(NULL); time(NULL)<timer2 + 2;)
					// continue;
				// memset(&rx_buf[0], 0, sizeof(rx_buf));
				read(read_fd, rx_buf, sizeof(rx_buf));
				printf("\e[00mPARENT: %s\n", rx_buf);
				// for(timer2=time(NULL); time(NULL)<timer2 + 2;)
					// continue;
				// strcpy(tx_buf, "\e[00mHello Child. I am Parent");
				// sprintf(tx_buf, "\e[00mWeleome Child. I am Parent--%d\n", i);
				// write(write_fd, tx_buf, strlen(tx_buf)+1);
			}
#else
			for(int i=0; i<10; i++){
				for(timer2=time(NULL); time(NULL)<timer2 + 2;)
					continue;
				strcpy(tx_buf, "\e[00mHello Child. I am Parent");
				write(write_fd, tx_buf, strlen(tx_buf)+1);
				read(read_fd, rx_buf, sizeof(rx_buf));
				printf("\e[00mPARENT: %s\n", rx_buf);
			}
#endif
			exit(0);
	}
}