#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>   
#include <sys/wait.h>
#include <pthread.h>

void * server_function (void * args) {
	int bytes_recv;
	int sockfd = *((int*)args);
	char tx_buf[128], rx_buf[128];
	
	for(int i=1; ; i++) {
		memset(tx_buf, 0, sizeof(tx_buf));
		memset(rx_buf, 0, sizeof(rx_buf));
		if((bytes_recv = recv(sockfd, rx_buf, sizeof(rx_buf), 0)) == -1){
			perror("recv");
			exit(1);
		}
		if(bytes_recv == 0) break;	
		printf("Server Rx(%d) : %s", getpid(), rx_buf);
		sprintf(tx_buf, "Hi_%d, client(from %d)~~\n", i, getpid());
		if(send(sockfd, tx_buf, strlen(tx_buf)+1, 0) == -1) perror("send");	
	}
	printf("Server(%d): Client Connection Socket Closed!!\n", getpid());
	close(sockfd);
	return NULL;
}

int main(void){
	int server_sfd, client_sfd;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	int sock_size;
	int yes = 1;
	int i;
	pthread_t tid;

	if((server_sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket() error");
		exit(1);
	}

	if(setsockopt(server_sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		perror("setsockopt() error");
		exit(1);
	}
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(10000); //server port number setting
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(&(server_addr.sin_zero), '\0', 8);

	//server ip & port number setting
	if(bind(server_sfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
		perror("bind() error");
		exit(1);
	}

	//client backlog setting
	if(listen(server_sfd, 5) == -1){
		perror("listen() error");
		exit(1);
	}

	while(1){
		sock_size = sizeof(struct sockaddr_in);

		//wait for client request
		if((client_sfd = accept(server_sfd, (struct sockaddr *) &client_addr, &sock_size)) == -1){
			perror("accept() error");
			continue;
		}

		printf("server : got connection from %s \n", inet_ntoa(client_addr.sin_addr));
		pthread_create(&tid, NULL, server_function, &client_sfd);
	}
	return 0;
}

