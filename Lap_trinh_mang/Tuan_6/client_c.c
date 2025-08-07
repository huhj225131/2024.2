#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, recv_servaddr;
    char *server_ip = "127.0.0.1";
    fd_set read_fds;
    
    // Tạo socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &servaddr.sin_addr);
    
    // Gửi yêu cầu nhận key
    strcpy(buffer, "key");
    socklen_t len_ser = sizeof(servaddr);
    sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&servaddr, len_ser);
    
    // Nhận key từ server
    int valread = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&servaddr, &len_ser);
    buffer[valread] = '\0';
    int key = atoi(buffer);
    while (1) {
        printf("Message: \n");
        // fflush(stdout);

        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sockfd, &read_fds);
        int max_fd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;
        
        int result = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (result == -1) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            // Đọc input từ user và gửi đi
            fgets(buffer, sizeof(buffer), stdin);
            sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        }
        
        if (FD_ISSET(sockfd, &read_fds)) {
            // Nhận tin nhắn từ server
            socklen_t len = sizeof(recv_servaddr);
            valread = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&recv_servaddr, &len);
            
            if (valread > 0) {
                buffer[valread] = '\0';
                char plaintext[MAXLINE];
                
                for (int i = 0; i < valread; i++) {
                    plaintext[i] = buffer[i] ^ key;
                }
                plaintext[valread] = '\0';
                
                printf("Server cipher text from another client: %s\n", buffer);
                printf("Server plain text from another client: %s", plaintext);
            }
        }
    }
    
    close(sockfd);
    return 0;
}
