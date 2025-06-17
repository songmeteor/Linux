#include <semaphore.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include <stdlib.h>
#include <stdio.h>

#define KEY_NUM     0x2222
#define	MEM_SIZE		1024

int main(int argc, char **argv) {
  sem_t *mysem;
	int shm_id;
	void *shm_addr;

	printf("Started --------------------\n");
	if((shm_id = shmget((key_t)KEY_NUM,MEM_SIZE,IPC_CREAT|0666)) == -1) {
		perror("shmget");
		exit(1);
	}

	if((shm_addr = shmat(shm_id, NULL, 0)) == (void *)-1) {
		perror("shmat");
		exit(1);
	}
	
	printf("Named semaphore ---------------\n");
  if((mysem = sem_open("/mysemaphore", O_CREAT, 0777, 1)) == NULL) {
    perror("Sem Open Error");
    return 1;
  }

  for(int i=0; i<500; i++){
#ifdef SEM
    sem_wait(mysem);
#endif
		sprintf((char *)shm_addr, "%d", getpid());
		for(int j=0; j<5000000; j++){}
		if(getpid() == atoi(shm_addr))
			putchar('0');
		else
			putchar('X');
		fflush(stdout);
#ifdef SEM
		sem_post(mysem);
#endif
    }
	if(shmdt(shm_addr) !=0){
		perror("shmdt");
		exit(2);
	}
	sem_close(mysem);
	sem_unlink("/mysemaphore");
	if(shmctl(shm_id, IPC_RMID, NULL) == -1){
		perror("shmctl");
		exit(2);
	}
}

// sem.mysem file for mysem semaphore is located in /dev/shm
// To compile the program, -lrt & -lpthread is required
