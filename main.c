#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // 1. 서버 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 2. 포트 재사용 설정 (서버 재시작 시 포트 점유 에러 방지)
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 모든 IP로부터의 접속 허용
    address.sin_port = htons(PORT);

    // 3. 바인딩 및 리슨
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("바인드 실패");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("리슨 실패");
        exit(EXIT_FAILURE);
    }

    printf("🚀 서버 가동 중... (포트: %d)\n", PORT);
    printf("🌐 웹 브라우저 접속 주소: http://[WSL_IP]:%d\n", PORT);

    while(1) {
        printf("\n📱 연결 대기 중...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("접속 승인 실패");
            continue;
        }

        // 4. JSON 데이터 생성 (cJSON 사용)
        cJSON *root = cJSON_CreateObject();
        cJSON *books = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "books", books);

        // 도서 데이터 추가 (예시)
        cJSON *book1 = cJSON_CreateObject();
        cJSON_AddNumberToObject(book1, "id", 1);
        cJSON_AddStringToObject(book1, "title", "C Programming");
        cJSON_AddStringToObject(book1, "author", "Jeong Ho-young");
        cJSON_AddNumberToObject(book1, "available", 1);
        cJSON_AddItemToArray(books, book1);

        
        cJSON *book2 = cJSON_CreateObject();
        cJSON_AddNumberToObject(book2, "id", 2);
        cJSON_AddStringToObject(book2, "title", "Algorithm Study");
        cJSON_AddStringToObject(book2, "author", "Gemini");
        cJSON_AddNumberToObject(book2, "available", 0);
        cJSON_AddItemToArray(books, book2);

        char *json_string = cJSON_PrintUnformatted(root);
        
        // 5. 웹 브라우저 대응을 위한 HTTP 응답 포장
        char http_response[8192];
        memset(http_response, 0, sizeof(http_response));

        // HTTP 헤더 추가: 브라우저가 JSON으로 인식하고 보안 차단(CORS)을 하지 않도록 설정
        sprintf(http_response, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json; charset=utf-8\r\n"
            "Access-Control-Allow-Origin: *\r\n" 
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s", strlen(json_string), json_string);

        // 6. 데이터 전송
        send(new_socket, http_response, strlen(http_response), 0);
        printf("✅ 응답 전송 완료! (데이터 크기: %ld bytes)\n", strlen(http_response));

        // 7. 잠시 대기 후 소켓 닫기
        usleep(100000); 
        close(new_socket);

        // 메모리 해제
        free(json_string);
        cJSON_Delete(root);
    }

    
    return 0;
}