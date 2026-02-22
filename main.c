#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>      // 네트워크 통신 도구
#include <arpa/inet.h>   // 인터넷 주소 도구
#include <cjson/cJSON.h>

#define PORT 8080        // 안드로이드가 접속할 문 번호 (포트)

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 1. 서버 소켓(입구) 만들기
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 2. 포트 8080번을 사용할 수 있게 설정
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 아무나 접속 허용
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    // 3. 손님 기다리기 시작
    listen(server_fd, 3);
    printf("--- 도서 관리 서버가 가동되었습니다 (포트: %d) ---\n", PORT);
    printf("안드로이드 앱의 접속을 기다리는 중...\n");

    while(1) {
        // 4. 손님이 오면 연결 승인 (Accept)
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        printf("안드로이드 앱이 접속했습니다!\n");

        // 5. 보낼 데이터(JSON) 준비
        cJSON *json = cJSON_CreateObject();
        cJSON_AddNumberToObject(json, "id", 1);
        cJSON_AddStringToObject(json, "title", "C Programming Master");
        cJSON_AddStringToObject(json, "loan_date", "2026-02-21");
        cJSON_AddStringToObject(json, "status", "대출중");

        char *json_string = cJSON_Print(json);

        // 6. 데이터 전송 (Send)
        send(new_socket, json_string, strlen(json_string), 0);
        printf("JSON 데이터를 전송했습니다.\n");

        // 7. 정리 후 다음 손님 대기
        cJSON_Delete(json);
        free(json_string);
        close(new_socket);
        printf("연결을 종료하고 다시 대기합니다...\n\n");
    }

    return 0;
}