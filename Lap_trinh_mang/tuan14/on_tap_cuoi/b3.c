#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define FTP_PORT 21
#define BUFFER_SIZE 4096
#define RESPONSE_SIZE 1024

// Hàm gửi lệnh FTP và nhận phản hồi
int send_ftp_command(int sockfd, const char* command, char* response) {
    char cmd_with_crlf[BUFFER_SIZE];
    
    // Thêm CRLF vào cuối lệnh
    snprintf(cmd_with_crlf, sizeof(cmd_with_crlf), "%s\r\n", command);
    
    // Gửi lệnh
    if (send(sockfd, cmd_with_crlf, strlen(cmd_with_crlf), 0) < 0) {
        perror("Lỗi gửi lệnh FTP");
        return -1;
    }
    
    printf("-> %s\n", command);
    
    // Nhận phản hồi
    int bytes_received = recv(sockfd, response, RESPONSE_SIZE - 1, 0);
    if (bytes_received < 0) {
        perror("Lỗi nhận phản hồi FTP");
        return -1;
    }
    
    response[bytes_received] = '\0';
    printf("<- %s", response);
    
    return bytes_received;
}


int connect_to_server(const char* hostname, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent* host_entry;
    
    // Tạo socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Lỗi tạo socket");
        return -1;
    }
    
    // Resolve hostname
    host_entry = gethostbyname(hostname);
    if (host_entry == NULL) {
        fprintf(stderr, "Lỗi: Không thể resolve hostname %s\n", hostname);
        close(sockfd);
        return -1;
    }
    
    // Thiết lập địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host_entry->h_addr_list[0], host_entry->h_length);
    
    // Kết nối
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Lỗi kết nối đến server");
        close(sockfd);
        return -1;
    }
    
    printf("Đã kết nối đến %s:%d\n", hostname, port);
    return sockfd;
}

// Hàm parse phản hồi PASV để lấy IP và port
int parse_pasv_response(const char* response, char* ip, int* port) {
    char* start = strchr(response, '(');
    if (!start) {
        return -1;
    }
    start++; // Bỏ qua dấu (
    
    int ip1, ip2, ip3, ip4, port1, port2;
    if (sscanf(start, "%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &port1, &port2) != 6) {
        return -1;
    }
    
    sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    *port = port1 * 256 + port2;
    
    return 0;
}

// Hàm kiểm tra mã phản hồi FTP
int check_ftp_response(const char* response, int expected_code) {
    int code = atoi(response);
    return (code == expected_code);
}

// Hàm download file
int download_file(const char* hostname, const char* username, 
                  const char* password, const char* filename) {
    int control_sock, data_sock;
    char response[RESPONSE_SIZE];
    char command[BUFFER_SIZE];
    char data_ip[16];
    int data_port;
    FILE* local_file;
    
    // 1. Kết nối đến FTP server (port 21)
    printf("=== Bước 1: Kết nối đến FTP server ===\n");
    control_sock = connect_to_server(hostname, FTP_PORT);
    if (control_sock < 0) {
        return -1;
    }
    
    // Nhận thông báo chào mừng
    if (recv(control_sock, response, RESPONSE_SIZE - 1, 0) < 0) {
        perror("Lỗi nhận thông báo chào mừng");
        close(control_sock);
        return -1;
    }
    response[RESPONSE_SIZE - 1] = '\0';
    printf("<- %s", response);
    
    if (!check_ftp_response(response, 220)) {
        printf("Lỗi: Server không sẵn sàng\n");
        close(control_sock);
        return -1;
    }
    
    // 2. Đăng nhập bằng username
    printf("\n=== Bước 2: Đăng nhập ===\n");
    snprintf(command, sizeof(command), "USER %s", username);
    if (send_ftp_command(control_sock, command, response) < 0) {
        close(control_sock);
        return -1;
    }
    
    if (!check_ftp_response(response, 331)) {
        printf("Lỗi: Username không hợp lệ\n");
        close(control_sock);
        return -1;
    }
    
    // Gửi password
    snprintf(command, sizeof(command), "PASS %s", password);
    if (send_ftp_command(control_sock, command, response) < 0) {
        close(control_sock);
        return -1;
    }
    
    if (!check_ftp_response(response, 230)) {
        printf("Lỗi: Password không đúng hoặc đăng nhập thất bại\n");
        close(control_sock);
        return -1;
    }
    
    printf("Đăng nhập thành công!\n");
    
    // 3. Chuyển sang chế độ Binary
    printf("\n=== Bước 3: Chuyển sang chế độ Binary ===\n");
    if (send_ftp_command(control_sock, "TYPE I", response) < 0) {
        close(control_sock);
        return -1;
    }
    
    if (!check_ftp_response(response, 200)) {
        printf("Lỗi: Không thể chuyển sang chế độ Binary\n");
        close(control_sock);
        return -1;
    }
    
    // 4. Chuyển sang chế độ Passive (PASV)
    printf("\n=== Bước 4: Chuyển sang chế độ Passive ===\n");
    if (send_ftp_command(control_sock, "PASV", response) < 0) {
        close(control_sock);
        return -1;
    }
    
    if (!check_ftp_response(response, 227)) {
        printf("Lỗi: Không thể chuyển sang chế độ Passive\n");
        close(control_sock);
        return -1;
    }
    
    // Parse thông tin IP và port từ phản hồi PASV
    if (parse_pasv_response(response, data_ip, &data_port) < 0) {
        printf("Lỗi: Không thể parse thông tin PASV\n");
        close(control_sock);
        return -1;
    }
    
    printf("Thông tin kết nối dữ liệu: %s:%d\n", data_ip, data_port);
    
    // 5. Thiết lập kết nối dữ liệu
    printf("\n=== Bước 5: Thiết lập kết nối dữ liệu ===\n");
    data_sock = connect_to_server(data_ip, data_port);
    if (data_sock < 0) {
        close(control_sock);
        return -1;
    }
    
    // 6. Gửi lệnh RETR để download file
    printf("\n=== Bước 6: Yêu cầu download file ===\n");
    snprintf(command, sizeof(command), "RETR %s", filename);
    if (send_ftp_command(control_sock, command, response) < 0) {
        close(control_sock);
        close(data_sock);
        return -1;
    }
    
    if (!check_ftp_response(response, 150) && !check_ftp_response(response, 125)) {
        printf("Lỗi: Không thể download file (có thể file không tồn tại)\n");
        close(control_sock);
        close(data_sock);
        return -1;
    }
    
    // 7. Mở file cục bộ để ghi
    local_file = fopen(filename, "wb");
    if (!local_file) {
        perror("Lỗi tạo file cục bộ");
        close(control_sock);
        close(data_sock);
        return -1;
    }
    
    // 8. Nhận dữ liệu và ghi vào file
    printf("\n=== Bước 7: Nhận dữ liệu file ===\n");
    char buffer[BUFFER_SIZE];
    int bytes_received;
    long total_bytes = 0;
    
    while ((bytes_received = recv(data_sock, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytes_received, local_file);
        total_bytes += bytes_received;
        printf("Đã nhận: %ld bytes\r", total_bytes);
        fflush(stdout);
    }
    
    printf("\nDownload hoàn tất! Tổng cộng: %ld bytes\n", total_bytes);
    
    // Đóng file
    fclose(local_file);
    
    // Đóng kết nối dữ liệu
    close(data_sock);
    
    // Nhận phản hồi cuối cùng
    if (recv(control_sock, response, RESPONSE_SIZE - 1, 0) > 0) {
        response[RESPONSE_SIZE - 1] = '\0';
        printf("<- %s", response);
    }
    
    // 9. Đóng kết nối điều khiển
    printf("\n=== Bước 8: Đóng kết nối ===\n");
    send_ftp_command(control_sock, "QUIT", response);
    close(control_sock);
    
    printf("Download file '%s' thành công!\n", filename);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Cách sử dụng: %s <hostname> <username> <password> <filename>\n", argv[0]);
        printf("Ví dụ: %s ftp.example.com user123 secret123 myvideo.mp4\n", argv[0]);
        return 1;
    }
    
    const char* hostname = argv[1];
    const char* username = argv[2];
    const char* password = argv[3];
    const char* filename = argv[4];
    
    printf("=== FTP CLIENT DOWNLOAD ===\n");
    printf("Server: %s\n", hostname);
    printf("Username: %s\n", username);
    printf("Password: %s\n", password);
    printf("File: %s\n", filename);
    printf("===========================\n\n");
    
    if (download_file(hostname, username, password, filename) < 0) {
        printf("Download thất bại!\n");
        return 1;
    }
    
    return 0;
}