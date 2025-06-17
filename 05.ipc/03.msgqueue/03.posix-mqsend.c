#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#define MAX_MSG_SIZE 2048

int main(int argc, char *argv[]){
	int mqd; /* msg queue descriptor */
	/* attributes for the message queue */
	struct mq_attr msgattr;
	msgattr.mq_maxmsg = 10;
	msgattr.mq_msgsize = MAX_MSG_SIZE;
	char msg[128];

	mqd = mq_open("/myqueue", O_CREAT|O_RDWR, 0644, &msgattr); 
	if(mqd == -1){
	  perror("Creating message queue");
	  return 1;
	}
	for(int i=0; i<10; i++){
		sprintf(&msg[0], "Hello, message queue #%d", i+1);
		if((mq_send(mqd, msg, strlen(msg), 1)) == -1){
			perror("Message queue send");
			return 1;
		}
	}
	printf("Press Enter to remove message queue....");
	getchar();
	
	mq_close(mqd);
	mq_unlink("/myqueue");
	return 0;
}
