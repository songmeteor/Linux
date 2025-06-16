#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
	struct stat fileinfo;
	
	if (argc == 1){
		printf("Usage : %s <filename>\n", argv[0]);
		exit (1);
	}
	
	if((access (argv[1], F_OK)) != 0){
		printf("not exist\n");
		exit(1);
	}
	
	stat(argv[1], &fileinfo);
	printf("File mode of %s is %o\n : ", argv[1], fileinfo.st_mode);
	
	if((access(argv[1], R_OK)) != 0){
		chmod(argv[1], fileinfo.st_mode | S_IRUSR);
	}else
		printf("readable, ");
	
	if((access(argv[1], W_OK)) != 0)
		chmod(argv[1], fileinfo.st_mode | S_IWUSR);
	else 
		printf("writable, ");
	
	if((access(argv[1], X_OK)) == 0)
		printf("executable, ");
	else 
		printf("not executable, ");
	printf("\n");
}
