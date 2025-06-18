#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>

//#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <sys/syscall.h>

void *thread_function(void *arg);

int main() {
	int status;
	pthread_t tid;
	void *return_value;
	int i;

	
	status = pthread_create(&tid, NULL, thread_function, "hello thread\n");
	if(status !=0){
		perror("pthread_create");
		exit(1);
	}
	
	for(i=1; i<=5; i++){
		printf("Parent thread %d!!\n", i);
		sleep(1);
	}

	//
	status = pthread_join(tid, &return_value);
	if(status != 0){
		perror("pthread_join");
		exit(1);
	}
	printf("Thread joined, it returned %s\n", (char *)return_value); 
	// printf("Thread joined, it returned %ld\n", (uintptr_t)return_value); 
	
	return 0;
}

void *thread_function(void *arg){
	int i;
	pid_t * tpid;
	pthread_t * thread_id;
	
	tpid=malloc(sizeof(pid_t));
	thread_id=malloc(sizeof(pthread_t));

	*tpid = syscall(SYS_gettid);
	printf("Thread LWP: %d, Thread PID: %d\n", *tpid, getpid());
	
	*thread_id = pthread_self();
	printf("Thread ID: %lu\n", *thread_id);
	
	for(i=1; i<=10; i++){
		printf("\t\tChild thread %d\n", i);
		sleep(1);
	}
	// pthread_exit("Good Bye");
	// return (void *)1;
	pthread_exit((void *)0);
}
