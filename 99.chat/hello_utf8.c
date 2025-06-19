#include <stdio.h>
#include <locale.h>

int main() {
    setlocale(LC_ALL, "");
    printf("한글 출력 테스트입니다.\n");
    return 0;
}
