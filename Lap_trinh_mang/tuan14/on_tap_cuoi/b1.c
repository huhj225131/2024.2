#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define EMAIL_SUFFIX "@example.com"
#define USERS_FILE "users.txt"

// Hàm kiểm tra email có đúng định dạng @example.com
int validate_email(const char* email) {
    int email_len = strlen(email);
    int suffix_len = strlen(EMAIL_SUFFIX);
    
    if (email_len <= suffix_len) {
        return 0;
    }
    
    // Kiểm tra phần cuối email có phải @example.com
    return strcmp(email + email_len - suffix_len, EMAIL_SUFFIX) == 0;
}

// Hàm kiểm tra email đã tồn tại trong file
int email_exists(const char* email) {
    FILE* file = fopen(USERS_FILE, "r");
    if (file == NULL) {
        return 0; // File không tồn tại, email chưa có
    }
    
    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), file)) {
        // Tìm dấu : để tách email và password
        char* colon_pos = strchr(line, ':');
        if (colon_pos != NULL) {
            *colon_pos = '\0'; // Cắt chuỗi tại dấu :
            if (strcmp(line, email) == 0) {
                fclose(file);
                return 1; // Email đã tồn tại
            }
        }
    }
    
    fclose(file);
    return 0; // Email chưa tồn tại
}

// Hàm lưu email và password vào file
int save_user(const char* email, const char* password) {
    FILE* file = fopen(USERS_FILE, "a");
    if (file == NULL) {
        return 0; // Không thể mở file
    }
    
    fprintf(file, "%s:%s\n", email, password);
    fclose(file);
    return 1; // Lưu thành công
}

// Hàm xử lý đăng ký
void process_registration(const char* data, char* response) {
    char email[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    
    // Tìm dấu phẩy để tách email và password
    char* comma_pos = strchr(data, ',');
    if (comma_pos == NULL) {
        strcpy(response, "ERROR Cú pháp không hợp lệ");
        return;
    }
    
    
    int email_len = comma_pos - data;
    strncpy(email, data, email_len);
    email[email_len] = '\0';
    strcpy(password, comma_pos + 1);
    
    // Loại bỏ ký tự xuống dòng nếu có
    char* newline = strchr(password, '\n');
    if (newline) *newline = '\0';
    newline = strchr(password, '\r');
    if (newline) *newline = '\0';
    
    
    if (strlen(email) == 0 || strlen(password) == 0) {
        strcpy(response, "ERROR Cú pháp không hợp lệ");
        return;
    }
    
    
    if (!validate_email(email)) {
        strcpy(response, "ERROR Cú pháp không hợp lệ");
        return;
    }
    
    // Kiểm tra email đã tồn tại
    if (email_exists(email)) {
        strcpy(response, "ERROR Email đã tồn tại");
        return;
    }
    
    // Lưu user và gửi phản hồi
    if (save_user(email, password)) {
        strcpy(response, "OK Đăng ký thành công");
    } else {
        strcpy(response, "ERROR Không thể lưu dữ liệu");
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE];
    
    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Thiết lập socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Email Registration Server đang chạy trên port %d...\n", PORT);
    
    while (1) {
        printf("Đang đợi kết nối...\n");
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                               (socklen_t*)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        }
        
        printf("Client đã kết nối!\n");
        
        
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("Nhận từ client: %s\n", buffer);
            
            // Xử lý đăng ký
            process_registration(buffer, response);
            
            // Gửi phản hồi
            send(new_socket, response, strlen(response), 0);
            printf("Phản hồi: %s\n", response);
        }
        
        close(new_socket);
        printf("Đã đóng kết nối với client\n\n");
    }
    
    return 0;
}