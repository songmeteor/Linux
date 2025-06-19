#include <stdio.h>
#include "network.h"

#define SERVER_IP "127.0.0.1"
#define PORT 12345

int main() {
    int sockfd = create_client_socket(SERVER_IP, PORT);
    if (sockfd < 0) return 1;
    printf("Connected to server (fd=%d)\n", sockfd);
    // 나중에 입출력 로직 추가 예정
    return 0;
}