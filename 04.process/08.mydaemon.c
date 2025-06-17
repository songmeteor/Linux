#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <syslog.h>

int main(void) {
	pid_t pid, sid;
	int fd, fd0, fd1, fd2;
	time_t now;
	
	if((pid = fork()) != 0) {
		exit(0);																							//parent exit
	}
	sid=setsid();
	umask(0022); 			
	chdir("/");
	close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO);
	fd = open("/tmp/mydaemon.log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
	dup2(fd, 1); dup2(fd, 2);
	while(1){
		time(&now);
		fprintf(stdout, "Mydaemon alive at %s", ctime(&now));
		fflush(stdout); 																			/* flush the stream */
		sleep(5);
	}
	return 0;
}