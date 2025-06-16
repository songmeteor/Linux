#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]){
	char data[512]="hello world good morning\n";
	FILE *fp;
	int i;
	if((fp=fopen(argv[1], "w"))==NULL) {
		perror("fopen");
		exit(1);
	}
	printf("FILE-POS-IND: %ld\n", ftell(fp));
	if(fwrite(data, sizeof(char), strlen(data), fp) == -1) {
		perror("fwrite");
		fclose(fp);
		exit(2);
	}
	printf("FILE-POS-IND: %ld\n", ftell(fp));
	fclose(fp);
	return 0;
}

