#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *isprime(void *arg);
void *progress(void *arg);

int main(int argc, char *argv[]){
  long long num1;
  long long num2;
  pthread_t tid1, tid2, tid3;
  pthread_attr_t attr;
  if (argc != 3) {
    fprintf(stderr, "Please supply two numbers.\n" "Example: %s 9 7\n", argv[0]);
    return 1;
  }
  num1 = atoll(argv[1]);
  num2 = atoll(argv[2]);
  
  pthread_attr_init(&attr);
  
  pthread_create(&tid3, &attr, progress, NULL);
  pthread_detach(tid3);
  
  pthread_create(&tid1, &attr, isprime, &num1);
  pthread_create(&tid2, &attr, isprime, &num2);
  
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  
  pthread_attr_destroy(&attr);
  if (pthread_cancel(tid3) != 0)
     fprintf(stderr, "Couldn't cancel progress thread\n");
  printf("Done!\n");
	sleep(3);
  return 0;
}

void *isprime(void *arg){
   long long int number = *((long long*)arg);
   long long int j;
   int prime = 1;
    
   for(j=2; j<number; j++) {
      if(number%j == 0){
         prime = 0;
      }
   }
   if(prime == 1){
      printf("\n%lld is a prime number\n", number);
      return NULL;
   }else{
      printf("\n%lld is not a prime number\n", number);
      return NULL;
   }
}

void *progress(void *arg){
   while(1) {
      sleep(1);
      printf(".");
      fflush(stdout);
   }
   return NULL;
}
