#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "signalprint.h"

void handler(int sig) {
    sigset_t sigset;
    sigprocmask(SIG_SETMASK, NULL, &sigset);
    printf("\n[HANDLER] signal %d received\n", sig);
    printf("blocked signal set : ");
    print_sigset_t(&sigset);

    for (int i = 1; i <= 3; i++) {
        printf("handler_%d!!\n", i);
        sleep(1);
    }
}

int main(void) {
    struct sigaction act;
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGQUIT);  // 핸들러 실행 중 SIGQUIT 차단
    act.sa_flags = SA_RESTART;

    // SIGINT, SIGQUIT만 핸들러 지정
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);

    // 나머지 시그널은 모두 무시
    struct sigaction ign;
    ign.sa_handler = SIG_IGN;
    sigemptyset(&ign.sa_mask);
    ign.sa_flags = 0;

    for (int i = 1; i < NSIG; i++) {
        if (i == SIGINT || i == SIGQUIT) continue;
        sigaction(i, &ign, NULL);
    }

    while (1) {
        printf("signal test --- running...\n");
        sleep(2);
    }

    return 0;
}

