#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int get_ppid(pid_t pid) {
    char path[256], buffer[1024];
    FILE *status_file;
    int ppid = -1;

    snprintf(path, sizeof(path), "/proc/%d/status", pid);

    status_file = fopen(path, "r");
    if (!status_file) {
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), status_file)) {
        if (strncmp(buffer, "PPid:", 5) == 0) {
            sscanf(buffer + 5, "%d", &ppid);
            break;
        }
    }

    fclose(status_file);
    return ppid;
}

int main() {
    pid_t pid = getpid();
    printf("01.current : pid %d", pid);

    while (1) {
        int ppid = get_ppid(pid);
        if (ppid <= 1) {
            printf(" → ppid %d\n", ppid);
            break;
        }
        printf(" → ppid %d", ppid);
        pid = ppid;
    }

    return 0;
}

