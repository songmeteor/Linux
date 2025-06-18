#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char* argv[]){
	int sockfd;
	struct sockaddr_in sockaddr;
	char message[500];
	int bytes_recv;
	socklen_t len;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sockfd=socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("sockfdet() error!!");
		exit(1);
	}
	
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family=AF_INET;
	sockaddr.sin_addr.s_addr=inet_addr(argv[1]);
	sockaddr.sin_port=htons(atoi(argv[2]));
		
	if(connect(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr))==-1){
		perror("connect() error!!");
		exit(1);
	}

#if 1			//It is to see the peer information in terms of name and port #
	len = sizeof(sockaddr);
	getpeername(sockfd, (struct sockaddr*)&sockaddr, &len);
	printf("Peer IP address: %s\n", inet_ntoa(sockaddr.sin_addr));
	printf("Peer port      : %d\n", ntohs(sockaddr.sin_port));
#endif
	bzero(&message, sizeof(message));
	bytes_recv=recv(sockfd, message, sizeof(message), 0);
	//bytes_recv=read(sockfd, message, sizeof(message));
	if(bytes_recv==-1){
		perror("recv() error!!");
		exit(1);
	}
	printf("Message from server: %s (%d)\n", message, bytes_recv); 
	
	printf("Press Enter to close the socket!!!");
	getchar();

	close(sockfd);
	return 0;
}
