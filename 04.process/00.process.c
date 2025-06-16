#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int a=10, a2=20;
int b, b2;

int f1(int x){
	return ++x;
}

int f2(int x){
	return --x;
}

int main(){
	int c=100, c2;
	char *d = "\e[31mThis is virtual memory layout of the process\e[00m";
	// char *d; 
	char *name;
	
	name = malloc(40);
	name = "cafe";
	
	printf("addr(TEXT)=> \tmain:%p, f1:%p, f2:%p\n", main, f1, f2);
	printf("addr(CONST)=> \td:%p : %s\n", *(&d), d);
	printf("addr(DATA)=> \ta:%p, a2:%p, a_value:%d, a2_value:%d\n", &a, &a2,*(&a), *(&a2));
	printf("addr(BSS)=> \tb:%p, b2:%p, b_value:%d, b2_value:%d\n", &b, &b2, *(&b), *(&b2));
	printf("addr(HEAP)=> \tname_var:%p, name_addr:%p, name:%s\n", &name, name, name );
	printf("addr(STACK)=> \tc:%p, c_value:%d\n", &c, *(&c));
	printf("addr(STACK)=> \td:%p, d_value:%p\n", &d, *(&d));
	// c=200; c2=300;
	// printf("addr(STACK)=> c:%p, c_value:%d, c2:%p, c2_value:%d\n", &c, *(&c), &c2, *(&c2));
	sleep(1);
	strcpy(d, "hello world");
	printf("addr(CONST)=> \td:%p : %s\n", d, d);
	return 0;
}




