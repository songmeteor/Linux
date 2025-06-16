#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int count = 0;

void count_up(int signo)
{
	count++;
	printf(" count = %d\n",count);
}

void count_down(int signo)
{
	count--;
	printf(" count = %d\n",count);
}

int main(int argc, char *argv[])
{
	signal(SIGRTMIN , count_up);
	signal(SIGRTMIN+1 , count_down);
	while(1)
	{
		sleep(1);
	}
	return 0;
}
