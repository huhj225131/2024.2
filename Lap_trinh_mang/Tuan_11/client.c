#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// lệnh gửi đến server ftp
void send_command(int sockfd, const char *cmd){
    printf(">> Send command: %s", cmd);
    write(sockfd, cmd, strlen(cmd));
}

// hàm nhận phản hồi từ server
void receive_response(int sockfd, char* buffer){
    int n = read(sockfd, buffer, 1023);
    buffer[n] = '\0';
    printf("<< Server response: %s", buffer);
}

// hàm phân tích phản hồi PASV để lấy địa chỉ IP và cổng
void extract_pasv_ip_port(char* response, char* ip, int* port){
    int h1, h2, h3, h4, p1, p2;
    sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
         &h1, &h2, &h3, &h4, &p1, &p2);
        sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
        *port = p1 * 256 + p2;
}

int main() { 
    // tạo socket điều khiển
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // tạo địa chỉ server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(21);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // kết nối tới server
    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    char buffer[1024];
    receive_response(sockfd, buffer);
    

    send_command(sockfd, "USER user\r\n");
    receive_response(sockfd, buffer);
    

    send_command(sockfd, "PASS pass\r\n");
    receive_response(sockfd, buffer);
    

    send_command(sockfd, "PASV\r\n");
    receive_response(sockfd, buffer);
    

    char data_ip[64];
    int data_port;
    extract_pasv_ip_port(buffer, data_ip, &data_port);
    printf("Data IP: %s, Data Port: %d\n", data_ip, data_port);

    // tạo socket dữ liệu 
    int data_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    inet_pton(AF_INET, data_ip, &data_addr.sin_addr);
    connect(data_sockfd, (struct sockaddr*)&data_addr, sizeof(data_addr));

    //gui lenh list
    send_command(sockfd, "LIST\r\n");
    receive_response(sockfd, buffer);
    

    //nhan du lieu tu server
    printf("Receiving data...\n");
    while (1) {
        int n = read(data_sockfd, buffer, sizeof(buffer) - 1);
        if (n <= 0) break; // Break on error or connection close
        buffer[n] = '\0'; // Null-terminate the string
        printf("%s", buffer); // Print the received data
    }
    

    close(data_sockfd);
    receive_response(sockfd, buffer);

    //gui lenh quit
    send_command(sockfd, "QUIT\r\n");
    receive_response(sockfd, buffer);
    
    return 0;
}