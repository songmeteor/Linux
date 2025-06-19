#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <locale.h>

#define BUFFER_SZ 2048
#define NICKNAME_LEN 32
#define PORT 8888

int sock;

void *receive_handler(void *arg) {
    char buffer[BUFFER_SZ];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }
    return NULL;
}

int main() {
    setlocale(LC_ALL, "");
    struct sockaddr_in server_addr;
    pthread_t recv_thread;
    char message[BUFFER_SZ];
    char nickname[NICKNAME_LEN];

    printf("닉네임을 입력하세요: ");
    fgets(nickname, NICKNAME_LEN, stdin);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    send(sock, nickname, strlen(nickname), 0);

    pthread_create(&recv_thread, NULL, receive_handler, NULL);
    pthread_detach(recv_thread);

    while (fgets(message, BUFFER_SZ, stdin) != NULL) {
        send(sock, message, strlen(message), 0);
    }

    close(sock);
    return 0;
}

