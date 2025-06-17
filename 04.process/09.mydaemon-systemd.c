#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

void sigHandler(int sig);

int fd;
const char pidfile[] = "/run/mydaemon.pid";
const char daemonlog[]="/tmp/mydaemon.log";

int main(int argc, char * argv[]) {
	pid_t pid, sid;
	time_t now;
	struct sigaction action;
		
	if((pid = fork()) != 0) { 
		exit(0);									//parent exit and child continues
	}
	
	if((pid = fork()) !=0 ) {		//double forking
		exit(0);					
	}
	
	sid=setsid();
	if ((fd = open(pidfile, O_RDWR | O_CREAT)) == -1){
			perror("Can't open file for writing");
			return 1;
	}
	dprintf(fd, "%d\n", getpid());
	
	umask(0); 			
	if(chdir("/") !=0 ) {
		perror("chdir");
		exit(1);
	}
	
	close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO);
	
	/* prepare for sigaction */
	action.sa_handler = sigHandler;
	sigfillset(&action.sa_mask);
	action.sa_flags = SA_RESTART;
	/* register the signals we want to handle */
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGABRT, &action, NULL);
	
	if((access(daemonlog, F_OK)) == 0){
	   unlink(daemonlog);
   }
	
	if ((fd = open(daemonlog, O_CREAT | O_RDWR | O_TRUNC, 0644)) == -1){
		perror("Can't open daemonlog");
		return 1;
	}
	while(1){
		time(&now);
		dprintf(fd, "Mydaemon alive at %s", ctime(&now));
		sleep(5);
	}
	return 0;
}

void sigHandler(int sig){
    int status = 0;
    if ( sig == SIGTERM || sig == SIGINT || sig == SIGQUIT || sig == SIGABRT ){
        if ((unlink(pidfile)) == -1)			/* remove the pid-file */
            status = 1;
        if ((close(fd)) == -1)
            status = 1;
		if ((unlink(daemonlog)) == -1)			/* remove the daemonlog */
            status = 1;
        exit(status); 							/* exit with the status set*/
    }else{										/* some other signal */
        exit(1);
    }
}