#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "network.h"

#define PORT     12345
#define BACKLOG  5

// 클라이언트 전용 스레드 함수
void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    printf("[Thread %lu] Client connected: fd=%d\n",
           pthread_self(), client_fd);

    // TODO: 여기서 recv/send 로 메시지 주고받기

    close(client_fd);
    printf("[Thread %lu] Client disconnected: fd=%d\n",
           pthread_self(), client_fd);
    return NULL;
}

int main() {
    int listen_fd = create_server_socket(PORT, BACKLOG);
    if (listen_fd < 0) return 1;
    printf("Server listening on port %d (fd=%d)\n", PORT, listen_fd);

    while (1) {
        // 1) 클라이언트 연결 수락
        int* client_fd = malloc(sizeof(int));
        if (!client_fd) {
            perror("malloc");
            continue;
        }
        *client_fd = accept(listen_fd, NULL, NULL);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        // 2) 새 스레드 생성
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, client_fd) != 0) {
            perror("pthread_create");
            close(*client_fd);
            free(client_fd);
            continue;
        }
        // 3) 스레드 분리(detach) → 리소스 자동 해제
        pthread_detach(tid);
    }

    close(listen_fd);
    return 0;
}
