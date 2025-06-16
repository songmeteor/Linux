#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void){
	pid_t pid;
	printf("My PID is %d\n", getpid());
   
	pid = fork();
	if(pid == -1){
		perror("fork failed");
		return 1;
	}
	printf("Hello World from PID = %d\n", getpid());
	sleep(120);
	return 0;
}



