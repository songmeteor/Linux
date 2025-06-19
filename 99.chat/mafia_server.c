
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>

#define MAX_CLIENTS       100
#define REQUIRED_PLAYERS  5
#define BUFFER_SZ         2048
#define NICKNAME_LEN      32
#define FULL_MSG_SZ       (BUFFER_SZ + NICKNAME_LEN + 16)
#define PORT              8888

// 클라이언트 정보
typedef struct {
    int  socket;
    char nickname[NICKNAME_LEN];
    char role[NICKNAME_LEN];
    int  alive;
} Client;

// 게임 단계
typedef enum { PHASE_DAY, PHASE_VOTE1, PHASE_VOTE2_CHAT, PHASE_VOTE2_VOTING, PHASE_NIGHT } Phase;

// 전역 변수
Client          clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
Phase           phase         = PHASE_DAY;
int             game_started  = 0;

// 투표 관련
int  player_list[REQUIRED_PLAYERS];
char *player_names[REQUIRED_PLAYERS];
int  vote_counts[REQUIRED_PLAYERS];
int  votes_received;
int  nominated_index;

// 밤 행동 관련
int mafia_target = -1, doctor_target = -1, police_target = -1;
int mafia_done = 0, doctor_done = 0, police_done = 0;

// 타이머 스레드
pthread_t day_timer_tid, vote2_timer_tid;

// 함수 선언
void broadcast_message(const char *message, int sender_sock);
void send_private(int sock, const char *message);
void list_alive_players(int sock);
void start_vote1();
void start_vote2();
void start_night();
void process_night();
void *day_timer(void *arg);
void *vote2_timer(void *arg);

// 살아있는 플레이어 목록 전송
void list_alive_players(int sock) {
    pthread_mutex_lock(&clients_mutex);
    char buf[BUFFER_SZ] = "[플레이어 목록]\n";
    int idx = 1;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket > 0 && clients[i].alive) {
            char line[64];
            snprintf(line, sizeof(line), "%d: %s\n", idx++, clients[i].nickname);
            strcat(buf, line);
        }
    }
    strcat(buf, "\n");
    pthread_mutex_unlock(&clients_mutex);
    send(sock, buf, strlen(buf), 0);
}

// 낮 타이머
void *day_timer(void *arg) {
    sleep(60);
    broadcast_message("[주의] 낮 시간이 1분 남았습니다.\n\n", -1);
    sleep(30);
    broadcast_message("[주의] 낮 시간이 30초 남았습니다.\n\n", -1);
    sleep(30);

    pthread_mutex_lock(&clients_mutex);
    phase = PHASE_VOTE1;
    pthread_mutex_unlock(&clients_mutex);

    broadcast_message("[낮 종료] 투표1을 시작합니다.\n\n", -1);
    start_vote1();
    return NULL;
}

// 투표1 시작
void start_vote1() {
    pthread_mutex_lock(&clients_mutex);
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS && count < REQUIRED_PLAYERS; ++i) {
        if (clients[i].socket > 0 && clients[i].alive) {
            player_list[count] = i;
            player_names[count] = clients[i].nickname;
            count++;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    char msg[BUFFER_SZ] = "[투표1] 투표할 사람 번호를 입력하세요:\n";
    for (int i = 0; i < count; ++i) {
        char line[64];
        snprintf(line, sizeof(line), "%d. %s  ", i+1, player_names[i]);
        strcat(msg, line);
    }
    strcat(msg, "\n\n");
    broadcast_message(msg, -1);

    memset(vote_counts, 0, sizeof(vote_counts));
    votes_received = 0;
}

// 투표2 토론 타이머
void *vote2_timer(void *arg) {
    sleep(30);
    pthread_mutex_lock(&clients_mutex);
    phase = PHASE_VOTE2_VOTING;
    pthread_mutex_unlock(&clients_mutex);
    start_vote2();
    return NULL;
}

// 투표2 시작
void start_vote2() {
    char msg[BUFFER_SZ];
    snprintf(msg, sizeof(msg), "[투표2] %s: 처형(1) 구제(2) 입력하세요:\n\n", player_names[nominated_index]);
    broadcast_message(msg, player_list[nominated_index]);

    memset(vote_counts, 0, sizeof(vote_counts));
    votes_received = 0;
}

// 밤 단계 시작
void start_night() {
    pthread_mutex_lock(&clients_mutex);
    phase = PHASE_NIGHT;
    mafia_done = doctor_done = police_done = 0;
    mafia_target = doctor_target = police_target = -1;
    pthread_mutex_unlock(&clients_mutex);

    broadcast_message("[밤] 밤이 시작되었습니다. 행동하세요.\n", -1);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket <= 0 || !clients[i].alive) continue;
        int sock = clients[i].socket;
        list_alive_players(sock);
        if (strcmp(clients[i].role, "마피아") == 0) {
            send_private(sock, "[밤] 죽일 번호 입력:\n");
        } else if (strcmp(clients[i].role, "의사") == 0) {
            send_private(sock, "[밤] 살릴 번호 입력:\n");
        } else if (strcmp(clients[i].role, "경찰") == 0) {
            send_private(sock, "[밤] 조사할 번호 입력:\n");
        } else {
            send_private(sock, "[밤] 시민님, 기다려주세요.\n");
        }
    }
}

// 밤 처리 및 낮 전환
void process_night() {
    pthread_mutex_lock(&clients_mutex);
    if (mafia_target == doctor_target && mafia_target >= 0) {
        char buf[BUFFER_SZ];
        snprintf(buf, sizeof(buf), "[밤 결과] 의사가 %s님을 살렸습니다.\n\n", clients[mafia_target].nickname);
        broadcast_message(buf, -1);
    } else if (mafia_target >= 0) {
        clients[mafia_target].alive = 0;
        char buf[BUFFER_SZ];
        snprintf(buf, sizeof(buf), "[밤 결과] %s님이 마피아에게 죽었습니다.\n\n", clients[mafia_target].nickname);
        broadcast_message(buf, -1);
    }
    pthread_mutex_unlock(&clients_mutex);

    pthread_mutex_lock(&clients_mutex);
    phase = PHASE_DAY;
    pthread_mutex_unlock(&clients_mutex);
    broadcast_message("[낮] 낮이 시작되었습니다. 토론하세요.\n\n", -1);
    pthread_create(&day_timer_tid, NULL, day_timer, NULL);
    pthread_detach(day_timer_tid);
}

// 전체 브로드캐스트
void broadcast_message(const char *message, int sender_sock) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        int s = clients[i].socket;
        if (s > 0 && s != sender_sock) send(s, message, strlen(message), 0);
    }
    pthread_mutex_unlock(&clients_mutex);
}

// 개인 메시지
void send_private(int sock, const char *message) {
    send(sock, message, strlen(message), 0);
}

// 클라이언트 처리
void *handle_client(void *arg) {
    int client_sock = *((int*)arg); free(arg);
    char buffer[BUFFER_SZ], nickname[NICKNAME_LEN];
    int bytes_received, idx = -1;

    if ((bytes_received = recv(client_sock, nickname, NICKNAME_LEN, 0)) <= 0) {
        close(client_sock); return NULL;
    }
    nickname[bytes_received-1] = '\0';

    pthread_mutex_lock(&clients_mutex);
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i].socket) { clients[i].socket = client_sock; strncpy(clients[i].nickname, nickname, NICKNAME_LEN); clients[i].alive = 1; idx = i; break; }
        if (clients[i].socket) count++;
    }
    pthread_mutex_unlock(&clients_mutex);

    // 역할 할당 및 낮 시작
    char *roles[REQUIRED_PLAYERS] = {"마피아","시민","시민","의사","경찰"};
    if (!game_started && count+1 == REQUIRED_PLAYERS) {
        game_started = 1; srand(time(NULL));
        for (int i = REQUIRED_PLAYERS-1; i > 0; --i) { int j = rand() % (i+1); char *t = roles[i]; roles[i] = roles[j]; roles[j] = t; }
        int r = 0;
        for (int i = 0; i < MAX_CLIENTS; ++i) if (clients[i].socket) {
            strncpy(clients[i].role, roles[r], NICKNAME_LEN);
            char m[BUFFER_SZ]; snprintf(m, sizeof(m), "[역할] %s: %s\n", clients[i].nickname, roles[r]);
            send_private(clients[i].socket, m);
            r++;
        }
        broadcast_message("[게임 시작] 역할 배정 완료!\n\n", -1);
        pthread_create(&day_timer_tid, NULL, day_timer, NULL); pthread_detach(day_timer_tid);
    }

    // 메시지 루프
    while ((bytes_received = recv(client_sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received-1] = '\0';
        pthread_mutex_lock(&clients_mutex); Phase cur = phase; int alive = clients[idx].alive; pthread_mutex_unlock(&clients_mutex);

        if (cur == PHASE_DAY && alive) {
            char m[FULL_MSG_SZ]; snprintf(m, sizeof(m), "[%s] %s\n", nickname, buffer); broadcast_message(m, client_sock);
        }
        else if (cur == PHASE_VOTE1 && alive) {
            int c = atoi(buffer)-1; if (c>=0 && c<REQUIRED_PLAYERS) { vote_counts[c]++; votes_received++; }
            if (votes_received == REQUIRED_PLAYERS) {
                int maxv=0; for(int i=0;i<REQUIRED_PLAYERS;i++) if(vote_counts[i]>maxv){maxv=vote_counts[i]; nominated_index=i;}
                int ties=0; for(int i=0;i<REQUIRED_PLAYERS;i++) if(vote_counts[i]==maxv) ties++;
                if (ties>1) { broadcast_message("[투표1] 동률, 밤으로 이동\n\n", -1); start_night(); }
                else { pthread_mutex_lock(&clients_mutex); phase=PHASE_VOTE2_CHAT; pthread_mutex_unlock(&clients_mutex);
                       char b2[BUFFER_SZ]; snprintf(b2,sizeof(b2),"[투표2 토론] %s\n\n", player_names[nominated_index]); broadcast_message(b2,-1);
                       pthread_create(&vote2_timer_tid,NULL,vote2_timer,NULL); pthread_detach(vote2_timer_tid);
                }
            }
        }
        else if (cur == PHASE_VOTE2_CHAT && alive && idx==player_list[nominated_index]) {
            char c[FULL_MSG_SZ]; snprintf(c,sizeof(c),"[%s] %s\n",nickname,buffer); broadcast_message(c,client_sock);
        }
        else if (cur == PHASE_VOTE2_VOTING && alive && idx!=player_list[nominated_index]) {
            int v=atoi(buffer); if(v==1||v==2){vote_counts[v-1]++;votes_received++;}
            if (votes_received==REQUIRED_PLAYERS-1) {
                if(vote_counts[0]>vote_counts[1]) { clients[player_list[nominated_index]].alive=0;
                    char o[BUFFER_SZ]; snprintf(o,sizeof(o),"[투표2 결과] %s 처형\n\n",player_names[nominated_index]); broadcast_message(o,-1);
                } else { char o[BUFFER_SZ]; snprintf(o,sizeof(o),"[투표2 결과] %s 구제\n\n",player_names[nominated_index]); broadcast_message(o,-1);}                
                start_night();
            }
        }
        else if (cur == PHASE_NIGHT && alive) {
            if (!mafia_done && strcmp(clients[idx].role,"마피아")==0) { mafia_target=atoi(buffer)-1;mafia_done=1; send_private(client_sock,"[확인] 마피아\n"); }
            else if(!doctor_done && strcmp(clients[idx].role,"의사")==0) { doctor_target=atoi(buffer)-1;doctor_done=1; send_private(client_sock,"[확인] 의사\n"); }
            else if(!police_done && strcmp(clients[idx].role,"경찰")==0) {
                police_target=atoi(buffer)-1;police_done=1; send_private(client_sock,"[조사] ");
                if(strcmp(clients[police_target].role,"마피아")==0) send_private(client_sock,"마피아\n"); else send_private(client_sock,"아님\n");
            }
            if(mafia_done&&doctor_done&&police_done) process_night();
        }
    }

    // 퇴장 처리
    pthread_mutex_lock(&clients_mutex);
    clients[idx].socket=0;
    pthread_mutex_unlock(&clients_mutex);
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock = socket(AF_INET,SOCK_STREAM,0);
    if(server_sock<0){perror("socket 실패");exit(1);} int opt=1; setsockopt(server_sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    struct sockaddr_in addr={.sin_family=AF_INET, .sin_port=htons(PORT), .sin_addr.s_addr=INADDR_ANY};
    if(bind(server_sock,(struct sockaddr*)&addr,sizeof(addr))<0){perror("bind 실패");exit(1);} if(listen(server_sock,10)<0){perror("listen 실패");exit(1);} printf("서버 대기중 포트 %d\n",PORT);
    while(1){ struct sockaddr_in caddr; socklen_t len=sizeof(caddr); int*p=malloc(sizeof(int)); *p=accept(server_sock,(struct sockaddr*)&caddr,&len); if(*p<0){free(p);continue;} pthread_t t; pthread_create(&t,NULL,handle_client,p); pthread_detach(t);}    
    close(server_sock);
    return 0;
}

