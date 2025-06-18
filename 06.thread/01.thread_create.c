#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>

void *thread_function(void *arg);

int main() {
	int status;
	pthread_t tid;
	pthread_attr_t attr;
	void *thread_result;
	int i;
	

	pthread_attr_init(&attr);
	status = pthread_create(&tid, &attr, thread_function, "hello thread");
	// status = pthread_create(&tid, NULL, thread_function,(void *) NULL);
	if(status !=0){
		perror("pthread_create");
		exit(1);
	}
	printf("Created Thread ID = %u\n", (unsigned int)tid);
	
	for(i=1; i<=5; i++){
		printf("Parent thread %d!!\n", i);
		sleep(1);
	}
	return 0;
}

void *thread_function(void *arg){
	int i;
	pid_t tpid, pid;
	
	pthread_t thread_id;
	thread_id = pthread_self();
	printf("Thread ID: %u\n", (unsigned int)thread_id);

	for(i=1; i<=10; i++){
		printf("\t\tChild thread[%d] - %s\n", i, (char *)arg);
		sleep(1);
	}
	return NULL;
}
