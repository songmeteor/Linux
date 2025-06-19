#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <locale.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 2048
#define NICKNAME_LEN 32
#define PORT 8888

typedef struct {
    int socket;
    char nickname[NICKNAME_LEN];
} Client;

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket != 0 && clients[i].socket != sender_sock) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_sock = *((int *)arg);
    char buffer[BUFFER_SZ];
    char nickname[NICKNAME_LEN];
    int bytes_received;

    // 닉네임 수신
    if ((bytes_received = recv(client_sock, nickname, NICKNAME_LEN, 0)) <= 0) {
        close(client_sock);
        return NULL;
    }
    nickname[bytes_received - 1] = '\0'; // 개행 제거

    // 클라이언트 정보 저장
    pthread_mutex_lock(&clients_mutex);
    int index = -1;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == 0) {
            clients[i].socket = client_sock;
            strncpy(clients[i].nickname, nickname, NICKNAME_LEN);
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    // 입장 메시지
    snprintf(buffer, BUFFER_SZ, "[%s] 님이 입장했습니다.\n", nickname);
    broadcast_message(buffer, client_sock);

    // 메시지 루프
    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';
        char full_msg[BUFFER_SZ + NICKNAME_LEN];
        snprintf(full_msg, sizeof(full_msg), "[%s] %s", nickname, buffer);
        broadcast_message(full_msg, client_sock);
    }

    // 퇴장 처리
    pthread_mutex_lock(&clients_mutex);
    clients[index].socket = 0;
    pthread_mutex_unlock(&clients_mutex);

    snprintf(buffer, BUFFER_SZ, "[%s] 님이 퇴장했습니다.\n", nickname);
    broadcast_message(buffer, client_sock);
    close(client_sock);
    return NULL;
}

int main() {
    setlocale(LC_ALL, "");
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;
    socklen_t addr_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    listen(server_sock, 10);
    printf("서버 시작됨. 포트 %d에서 대기 중...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
        pthread_create(&tid, NULL, handle_client, (void*)&client_sock);
        pthread_detach(tid);
    }

    return 0;
}

