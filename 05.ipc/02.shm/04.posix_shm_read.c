#include <stdio.h>
#include <unistd.h>  
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define  MEM_SIZE    128

int main() {
	int fd;
	void *shm_addr;
	char buf[MEM_SIZE];

	fd = shm_open("/mydata", O_RDONLY, 0666);
	if(fd == -1){
		perror("shm_open");
		exit(1);
	}
	
	
	shm_addr = mmap(0, MEM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	if(shm_addr == (void *)-1){
		perror("mmap error");
		return EXIT_FAILURE;
	}
#if 1	
	memset(buf, 0, MEM_SIZE);
	read(fd, buf, MEM_SIZE);
	printf("%s\n", buf);
#else
	memcpy(buf, shm_addr, sizeof(buf));
	printf("Map addr is %p\n", shm_addr);
	printf("Read message: %s\n", buf);
#endif
	printf("Press enter to munmap .....");
	getchar();
	munmap(shm_addr, MEM_SIZE);
	printf("Press enter to remove shared memory file");
	getchar();
	shm_unlink("/mydata");
	// close(fd);
	return 0;
}