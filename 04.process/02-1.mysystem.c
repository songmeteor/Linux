#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

void handler(int signo) {
	pid_t exit_pid;
	printf("handler_pid [%d], handler_ppid [%d]\n", getpid(), getppid());
	exit_pid = waitpid(-1, NULL, WNOHANG);
	if(exit_pid == -1)
		printf("waitpid error with ERRNO=%d meaning %s\n", errno,  strerror(errno));
	else
		printf("Terminated child PID = %d\n", exit_pid);
}

int main() {
	signal(SIGCHLD, handler);
	system("echo Hello World");
	system("ps -lf");
}


