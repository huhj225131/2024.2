#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Send FTP command over the control socket
void send_command(int sockfd, const char *cmd) {
    write(sockfd, cmd, strlen(cmd));
    printf(">> Sent command: %s", cmd);
}

// Receive and print response from FTP server
void receive_response(int sockfd) {
    char buffer[1024];
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        buffer[n] = '\0';
        printf("<< Server response: %s", buffer);
    }
}

// Parse PASV response to extract IP and port for data connection
void parse_pasv_response(char *response, char *ip, int *port) {
    char *start = strchr(response, '(');  // Tìm dấu mở ngoặc
    char *end = strchr(response, ')');    // Tìm dấu đóng ngoặc
    if (!start || !end || start >= end) {
        printf("Invalid PASV response format\n");
        exit(1);
    }

    // Cắt đoạn giữa ngoặc ()
    char numbers[100];
    strncpy(numbers, start + 1, end - start - 1);
    numbers[end - start - 1] = '\0';

    int h1, h2, h3, h4, p1, p2;
    if (sscanf(numbers, "%d,%d,%d,%d,%d,%d", &h1, &h2, &h3, &h4, &p1, &p2) == 6) {
        sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
        *port = p1 * 256 + p2;
    } else {
        printf("Failed to extract IP and port\n");
        exit(1);
    }
}



void ftp_retr(int control_sock, const char* filename) {
    char buffer[1024], command[256];

    // 1. Gửi lệnh PASV để lấy địa chỉ IP và cổng dữ liệu
    send_command(control_sock, "PASV\r\n");

    // 2. Nhận phản hồi từ server
    memset(buffer, 0, sizeof(buffer));
    int total_n = 0;
    while (1) {
        int n = read(control_sock, buffer + total_n, sizeof(buffer) - total_n - 1);
        if (n <= 0) break;
        total_n += n;
        buffer[total_n] = '\0';
        if (strstr(buffer, "\r\n")) break;
    }

    if (total_n <= 0) {
        printf("Failed to receive PASV response\n");
        return;
    }

    // 3. Phân tích PASV response để lấy IP và port
    char data_ip[32];
    int data_port;
    parse_pasv_response(buffer, data_ip, &data_port);
    printf(">> Data connection IP: %s, Port: %d\n", data_ip, data_port);

    // 4. Tạo kết nối dữ liệu
    int data_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    inet_pton(AF_INET, data_ip, &data_addr.sin_addr);

    if (connect(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
        perror("Data connection failed");
        close(data_sock);
        return;
    }

    // 5. Gửi lệnh RETR <filename>\r\n qua control socket
    snprintf(command, sizeof(command), "RETR %s\r\n", filename);
    send_command(control_sock, command);
    receive_response(control_sock); // Có thể là "150 Opening data..."

    // 6. Mở file local để ghi
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to open local file for writing");
        close(data_sock);
        return;
    }

    // 7. Nhận dữ liệu từ data socket và ghi ra file
    int n;
    while ((n = read(data_sock, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, n, fp);
    }

    fclose(fp);
    close(data_sock);
    receive_response(control_sock); // Có thể là "226 Transfer complete"

    printf(">> File \"%s\" has been downloaded successfully.\n", filename);
}

void ftp_stor(int control_sock, const char* filename){
    char buffer[1024], command[256];

    // 1. Gửi lệnh PASV để lấy địa chỉ IP và cổng dữ liệu
    send_command(control_sock, "PASV\r\n");

    // 2. Nhận phản hồi từ server
    memset(buffer, 0, sizeof(buffer));
    int total_n = 0;
    while (1) {
        int n = read(control_sock, buffer + total_n, sizeof(buffer) - total_n - 1);
        if (n <= 0) break;
        total_n += n;
        buffer[total_n] = '\0';
        if (strstr(buffer, "\r\n")) break;
    }

    if (total_n <= 0) {
        printf("Failed to receive PASV response\n");
        return;
    }

    // 3. Phân tích PASV response để lấy IP và port
    char data_ip[32];
    int data_port;
    parse_pasv_response(buffer, data_ip, &data_port);
    printf(">> Data connection IP: %s, Port: %d\n", data_ip, data_port);

    // 4. Tạo kết nối dữ liệu
    int data_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    inet_pton(AF_INET, data_ip, &data_addr.sin_addr);

    if (connect(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
        perror("Data connection failed");
        close(data_sock);
        return;
    }

    // 5. Gửi lệnh RETR <filename>\r\n qua control socket
    snprintf(command, sizeof(command), "STOR %s\r\n", filename);
    send_command(control_sock, command);
    receive_response(control_sock); 

    // 6. Mở file local để đọc
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        perror("Failed to open local file for reading");
        close(data_sock);
        return;
    }

    // 7. Gửi dữ liệu từ file lên server qua data socket
    int n;
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        write(data_sock, buffer, n);
    }

    fclose(fp);
    close(data_sock); 

    // 8. Nhận phản hồi hoàn tất
    receive_response(control_sock); // "226 Transfer complete"

    printf(">> File \"%s\" has been uploaded successfully.\n", filename);
} 
void ftp_delete(int control_sock, const char* filename) {
    char command[256];

    // 1. Tạo lệnh DELE <filename>\r\n
    snprintf(command, sizeof(command), "DELE %s\r\n", filename);

    // 2. Gửi lệnh DELE qua control socket
    send_command(control_sock, command);

    // 3. Nhận và in phản hồi từ server
    receive_response(control_sock);  // thường là "250 File deleted" hoặc "550 File not found"
}
void ftp_cwd(int control_sock, const char* dirname) {
    char command[256];

    // 1. Tạo lệnh CWD <directory>\r\n
    snprintf(command, sizeof(command), "CWD %s\r\n", dirname);

    // 2. Gửi lệnh CWD qua control socket
    send_command(control_sock, command);

    // 3. Nhận và in phản hồi từ server
    receive_response(control_sock);  
}
void ftp_rnfr(int control_sock, const char* old_name) {
    char command[256];

    // 1. Tạo lệnh RNFR <old_filename>\r\n
    snprintf(command, sizeof(command), "RNFR %s\r\n", old_name);

    // 2. Gửi lệnh RNFR qua control socket
    send_command(control_sock, command);

    // 3. Nhận và in phản hồi từ server
    receive_response(control_sock);
}
void ftp_rnto(int control_sock, const char* new_name) {
    char command[256];

    // 1. Tạo lệnh RNTO <new_filename>\r\n
    snprintf(command, sizeof(command), "RNTO %s\r\n", new_name);

    // 2. Gửi lệnh RNTO qua control socket
    send_command(control_sock, command);

    // 3. Nhận và in phản hồi từ server
    receive_response(control_sock);
}

void ftp_list(int control_sock) {
    char buffer[1024];

    // 1. Gửi PASV để lấy IP và port
    send_command(control_sock, "PASV\r\n");

    // 2. Nhận phản hồi PASV
    memset(buffer, 0, sizeof(buffer));
    int total_n = 0;
    while (1) {
        int n = read(control_sock, buffer + total_n, sizeof(buffer) - total_n - 1);
        if (n <= 0) break;
        total_n += n;
        buffer[total_n] = '\0';
        if (strstr(buffer, "\r\n")) break;
    }
    if (total_n <= 0) {
        printf("Failed to receive PASV response\n");
        return;
    }

    // 3. Phân tích IP và port
    char data_ip[32];
    int data_port;
    parse_pasv_response(buffer, data_ip, &data_port);
    printf(">> Data connection IP: %s, Port: %d\n", data_ip, data_port);

    // 4. Tạo kết nối dữ liệu
    int data_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    inet_pton(AF_INET, data_ip, &data_addr.sin_addr);

    if (connect(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
        perror("Data connection failed");
        close(data_sock);
        return;
    }

    // 5. Gửi LIST
    send_command(control_sock, "LIST\r\n");
    receive_response(control_sock);  // thường là 150

    // 6. Nhận danh sách file từ data socket
    printf("<< Directory listing:\n");
    int n;
    while ((n = read(data_sock, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);
    }

    // 7. Đóng kết nối data, nhận phản hồi cuối
    close(data_sock);
    receive_response(control_sock);  
}





int main() {
    int control_sock;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // 1. Create TCP socket for FTP control connection (port 21)
    control_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (control_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(21);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(control_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(control_sock);
        exit(1);
    }

    receive_response(control_sock);

    // 2. Login
    send_command(control_sock, "USER user\r\n");
    receive_response(control_sock);

    send_command(control_sock, "PASS pass\r\n");
    receive_response(control_sock);

    // 3. Enter PASV mode
    send_command(control_sock, "PASV\r\n");

    // Receive PASV response
    memset(buffer, 0, sizeof(buffer));
    int total_n = 0;
    while (1) {
        int n = read(control_sock, buffer + total_n, sizeof(buffer) - total_n - 1);
        if (n <= 0) break;
        total_n += n;
        buffer[total_n] = '\0';
        if (strstr(buffer, "\r\n")) break;
    }
    if (total_n > 0) {
        printf("<< Server PASV response: %s", buffer);
    } else {
        printf("Failed to receive PASV response\n");
        close(control_sock);
        exit(1);
    }

    // Parse PASV IP and port
    char data_ip[32];
    int data_port;
    parse_pasv_response(buffer, data_ip, &data_port);
    printf(">> Data connection IP: %s, Port: %d\n", data_ip, data_port);

    // Force data IP to localhost (127.0.0.1)
    // strcpy(data_ip, "127.0.0.1");

    // 4. Create data connection
    int data_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in data_addr;
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    inet_pton(AF_INET, data_ip, &data_addr.sin_addr);

    if (connect(data_sock, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
        perror("Data connection failed");
        close(data_sock);
        close(control_sock);
        exit(1);
    }

    // 5. Send LIST command
    send_command(control_sock, "LIST\r\n");
    receive_response(control_sock);

    // Receive and print file list from data connection
    printf("<< File list from server:\n");
    while (1) {
        int n = read(data_sock, buffer, sizeof(buffer) - 1);
        if (n <= 0) break;
        buffer[n] = '\0';
        printf("%s", buffer);
    }
    
    
    // Close data connection
    close(data_sock);
    receive_response(control_sock);
    while (1) {
        char cmd[16], arg[256];

        printf("\nEnter FTP command (retr/stor/del/cwd/ls/rn/q): ");
        scanf("%s", cmd);

        if (strcmp(cmd, "retr") == 0) {
            printf("Enter filename to download: ");
            scanf("%s", arg);
            ftp_retr(control_sock, arg);
        } else if (strcmp(cmd, "stor") == 0) {
            printf("Enter filename to upload: ");
            scanf("%s", arg);
            ftp_stor(control_sock, arg);
        } else if (strcmp(cmd, "del") == 0) {
            printf("Enter filename to delete: ");
            scanf("%s", arg);
            ftp_delete(control_sock, arg);
        } else if (strcmp(cmd, "cwd") == 0) {
            printf("Enter directory to change to: ");
            scanf("%s", arg);
            ftp_cwd(control_sock, arg);
        }
        else if (strcmp(cmd, "ls") == 0){
            ftp_list(control_sock);
        }
        else if(strcmp(cmd, "rn") == 0){
            char from[256], to[256];
            printf("Enter file to change name: ");
            scanf("%s", from);
            printf("Enter name to chane: ");
            scanf("%s", to);
            ftp_rnfr(control_sock, from);
            ftp_rnto(control_sock, to);

        }
        else if (strcmp(cmd, "q") == 0) {
            break;
        } else {
            printf("Unknown command. Valid commands: retr, stor, del, cwd, q\n");
        }
    }

    
    send_command(control_sock, "QUIT\r\n");
    receive_response(control_sock);
    close(control_sock);

    return 0;
}

