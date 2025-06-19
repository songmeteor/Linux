#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <locale.h>
#include <time.h>
#include <errno.h>

#define MAX_CLIENTS       100
#define REQUIRED_PLAYERS  5         // <<< 게임 시작에 필요한 인원 수
#define BUFFER_SZ         2048
#define NICKNAME_LEN      32
#define FULL_MSG_SZ       (BUFFER_SZ + NICKNAME_LEN + 16)
#define PORT              8888

typedef struct {
    int  socket;
    char nickname[NICKNAME_LEN];
} Client;

Client clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int game_started = 0;

void broadcast_message(const char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        int csock = clients[i].socket;
        if (csock <= 0) continue;
        if (sender_sock >= 0 && csock == sender_sock) continue;
        if (send(csock, message, strlen(message), 0) < 0) {
            perror("send 실패");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_sock = *((int*)arg);
    free(arg);

    char buffer[BUFFER_SZ];
    char nickname[NICKNAME_LEN];
    int bytes_received;

    // 닉네임 수신
    bytes_received = recv(client_sock, nickname, NICKNAME_LEN, 0);
    if (bytes_received <= 0) {
        perror("recv(nickname) 실패 또는 연결 종료");
        close(client_sock);
        return NULL;
    }
    nickname[bytes_received - 1] = '\0';

    // 클라이언트 리스트에 등록 및 현재 접속자 수 계산
    int current_count = 0;  // <<< 수정된 부분: 입장 시 현재 인원 세기
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == 0) {
            clients[i].socket = client_sock;
            strncpy(clients[i].nickname, nickname, NICKNAME_LEN);
            break;
        }
    }
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket > 0) current_count++;
    }
    pthread_mutex_unlock(&clients_mutex);

    // <<< 수정된 부분: 입장 메시지에 (현재/5) 추가 >>>
    snprintf(buffer, sizeof(buffer),
             "[%s] 님이 입장했습니다. (%d/%d)\n\n",
             nickname, current_count, REQUIRED_PLAYERS);
    broadcast_message(buffer, client_sock);

    // 역할 배정 체크
    int do_broadcast = 0;
    pthread_mutex_lock(&clients_mutex);
    if (!game_started) {
        if (current_count == REQUIRED_PLAYERS) {
            game_started = 1;
            do_broadcast = 1;
            // 역할 셔플 및 개별 전송
            const char *roles[REQUIRED_PLAYERS] = {"마피아","시민","시민","의사","경찰"};
            srand((unsigned)time(NULL));
            for (int i = REQUIRED_PLAYERS - 1; i > 0; --i) {
                int j = rand() % (i + 1);
                const char *tmp = roles[i];
                roles[i] = roles[j];
                roles[j] = tmp;
            }
            int r = 0;
            for (int i = 0; i < MAX_CLIENTS && r < REQUIRED_PLAYERS; ++i) {
                if (clients[i].socket > 0) {
                    char role_msg[BUFFER_SZ];
                    snprintf(role_msg, sizeof(role_msg),
                             "[역할] %s 님은 \"%s\"입니다.\n",
                             clients[i].nickname, roles[r++]);
                    if (send(clients[i].socket, role_msg, strlen(role_msg), 0) < 0)
                        perror("send(역할) 실패");
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    // 역할 배정 후 전체 알림
    if (do_broadcast) {
        broadcast_message("[게임 시작] 역할이 배정되었습니다!\n\n", -1);
    }

    // 채팅 루프
    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';
        // 메시지 끝 개행 제거
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') buffer[--len] = '\0';
        if (len > 0 && buffer[len-1] == '\r') buffer[--len] = '\0';
        // 두 줄 개행 추가
        char full_msg[FULL_MSG_SZ];
        snprintf(full_msg, sizeof(full_msg), "[%s] %s\n\n", nickname, buffer);
        broadcast_message(full_msg, client_sock);
    }
    if (bytes_received < 0) {
        perror("recv(메시지) 실패");
    }

    // 퇴장 처리
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == client_sock) {
            clients[i].socket = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    snprintf(buffer, sizeof(buffer), "[%s] 님이 퇴장했습니다.\n", nickname);
    broadcast_message(buffer, client_sock);

    close(client_sock);
    return NULL;
}

int main() {
    setlocale(LC_ALL, "");

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) { perror("socket 생성 실패"); exit(EXIT_FAILURE); }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind 실패"); close(server_sock); exit(EXIT_FAILURE);
    }
    if (listen(server_sock, 10) < 0) {
        perror("listen 실패"); close(server_sock); exit(EXIT_FAILURE);
    }
    printf("서버 시작됨. 포트 %d에서 대기 중...\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        int *pclient = malloc(sizeof(int));
        if (!pclient) { fprintf(stderr, "메모리 할당 실패\n"); continue; }

        *pclient = accept(server_sock,
                          (struct sockaddr*)&client_addr,
                          &addr_len);
        if (*pclient < 0) {
            perror("accept 실패");
            free(pclient);
            continue;
        }

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, pclient) != 0) {
            perror("pthread_create 실패");
            close(*pclient);
            free(pclient);
            continue;
        }
        pthread_detach(tid);
    }

    close(server_sock);
    return 0;
}
