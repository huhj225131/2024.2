#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024
#define MAXCLIENT 10

int main() {
    int sockfd;
    int key[MAXCLIENT] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    char buffer[MAXLINE];
    char cipher[MAXLINE];
    struct sockaddr_in servaddr, cliaddr, cliaddr_list[MAXCLIENT];
    int client_number = 0;

    // Tạo socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Thiết lập thông tin server
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket với địa chỉ server
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    socklen_t len = sizeof(cliaddr);
    printf("Server is running on port %d\n", PORT);

    while (1) {
        // Nhận tin nhắn từ client
        int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        // Kiểm tra nếu client đã được lưu trong danh sách
        int client_index = -1;
        for (int i = 0; i < client_number; i++) {
            if (memcmp(&cliaddr.sin_addr, &cliaddr_list[i].sin_addr, sizeof(cliaddr.sin_addr)) == 0 &&
                memcmp(&cliaddr.sin_port, &cliaddr_list[i].sin_port, sizeof(cliaddr.sin_port)) == 0) {
                client_index = i;
                break;
            }
        }
        
        // Nếu là client mới, thêm vào danh sách
        if (client_index == -1 && client_number < MAXCLIENT) {
            cliaddr_list[client_number] = cliaddr;
            client_index = client_number;
            client_number++;
        }

        // Xử lý yêu cầu "key"
        if (strcmp(buffer, "key") == 0) {
            char key_str[10];
            sprintf(key_str, "%d", key[client_index]);
            sendto(sockfd, key_str, strlen(key_str), 0, (const struct sockaddr *)&cliaddr, len);
        }
        
        else {
            // Mã hóa XOR với key của client trước khi gửi lại
            // for (int i = 0; i < n; i++) {
            //     cipher[i] = buffer[i] ^ key[client_index];
            // }
            // cipher[n] = '\0';

            // sendto(sockfd, cipher, n, 0, (const struct sockaddr *)&cliaddr, len);
            for(int i = 0 ; i  < client_number;i++){
                if (memcmp(&cliaddr.sin_addr, &cliaddr_list[i].sin_addr, sizeof(cliaddr.sin_addr)) != 0 ||
                memcmp(&cliaddr.sin_port, &cliaddr_list[i].sin_port, sizeof(cliaddr.sin_port)) != 0){
                for (int j = 0 ; j < n; j++){
                    cipher[j] =  buffer[j] ^ key[i];
                }
                cipher[n] = '\0';
                sendto(sockfd, cipher, n, 0, (const struct sockaddr *)& cliaddr_list[i], sizeof(cliaddr_list[i]));
            }
            }
        }
    }

    close(sockfd);
    return 0;
}
