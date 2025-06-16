#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#define BUFSIZE	4096

int main(int argc, char *argv[]){
	int fd, ret, maxread;
	char buf[BUFSIZE];
	off_t filesize;
	struct stat fileinfo;
	off_t pos;
	
	if(argc < 2){
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}
	
	fd = open(argv[1], O_RDONLY);
	if(fd == -1){
		perror("open()");
		return 1;
	}
	
	// fstat(fd, &fileinfo);
	stat(argv[1], &fileinfo);
	filesize = fileinfo.st_size;
	
	if(filesize >= BUFSIZE)
		maxread = BUFSIZE - 1;
	else
		maxread = filesize;

	printf("BUF-PTR : %p\n", &buf);
	printf("FILE-POS : %lx\n", lseek(fd, 0, SEEK_CUR));
	pos=lseek(fd, 0, SEEK_CUR);
	while(1) {
		memset(buf, 0, sizeof(buf));
		ret=read(fd, buf, maxread);
		if(ret == 0)
			break;
		printf("%s\n", buf);
		pos=lseek(fd, 0, SEEK_CUR);
		printf("BUF-PTR : %p\n", &buf[pos]);
		printf("FILE-POS : %lx\n", lseek(fd, 0, SEEK_CUR));
	}
	close(fd);
	return 0;
}
