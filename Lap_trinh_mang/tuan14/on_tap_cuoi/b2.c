#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_USERS 100
#define CONTACTS_FILE "contacts.txt"

// Cấu trúc lưu thông tin user
typedef struct {
    char username[100];
    char phone[20];
} User;

// Hàm kiểm tra số điện thoại hợp lệ (chỉ chứa số 0-9 và độ dài 10-11)
int validate_phone(const char* phone) {
    int len = strlen(phone);
    
    // Kiểm tra độ dài
    if (len < 10 || len > 11) {
        return 0;
    }
    
    // Kiểm tra chỉ chứa số
    for (int i = 0; i < len; i++) {
        if (!isdigit(phone[i])) {
            return 0;
        }
    }
    
    return 1;
}

// Hàm kiểm tra username hợp lệ (không chứa dấu cách)
int validate_username(const char* username) {
    if (strlen(username) == 0) {
        return 0;
    }
    
    for (int i = 0; username[i]; i++) {
        if (username[i] == ' ') {
            return 0;
        }
    }
    
    return 1;
}

// Hàm đọc tất cả users từ file
int read_users(User users[], int max_users) {
    FILE* file = fopen(CONTACTS_FILE, "r");
    if (file == NULL) {
        return 0; // File không tồn tại
    }
    
    char line[BUFFER_SIZE];
    int count = 0;
    
    while (fgets(line, sizeof(line), file) && count < max_users) {
        // Loại bỏ ký tự xuống dòng
        line[strcspn(line, "\n\r")] = '\0';
        
        // Tìm dấu : để tách username và phone
        char* colon_pos = strchr(line, ':');
        if (colon_pos != NULL) {
            *colon_pos = '\0';
            strcpy(users[count].username, line);
            strcpy(users[count].phone, colon_pos + 1);
            count++;
        }
    }
    
    fclose(file);
    return count;
}

// Hàm ghi tất cả users vào file
int write_users(User users[], int count) {
    FILE* file = fopen(CONTACTS_FILE, "w");
    if (file == NULL) {
        return 0;
    }
    
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s:%s\n", users[i].username, users[i].phone);
    }
    
    fclose(file);
    return 1;
}

// Hàm tìm user theo username
int find_user(User users[], int count, const char* username) {
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return i;
        }
    }
    return -1; // Không tìm thấy
}

// Hàm xử lý cập nhật số điện thoại
void process_phone_update(const char* data, char* response) {
    char username[100], old_phone[20], new_phone[20];
    User users[MAX_USERS];
    int user_count;
    
    // Parse dữ liệu theo định dạng username|old_phone|new_phone
    char* first_pipe = strchr(data, '|');
    if (first_pipe == NULL) {
        strcpy(response, "ERROR Cú pháp không hợp lệ");
        return;
    }
    
    char* second_pipe = strchr(first_pipe + 1, '|');
    if (second_pipe == NULL) {
        strcpy(response, "ERROR Cú pháp không hợp lệ");
        return;
    }
    
    // Tách username
    int username_len = first_pipe - data;
    strncpy(username, data, username_len);
    username[username_len] = '\0';
    
    // Tách old_phone
    int old_phone_len = second_pipe - first_pipe - 1;
    strncpy(old_phone, first_pipe + 1, old_phone_len);
    old_phone[old_phone_len] = '\0';
    
    // Tách new_phone
    strcpy(new_phone, second_pipe + 1);
    
    // Loại bỏ ký tự xuống dòng nếu có
    new_phone[strcspn(new_phone, "\n\r")] = '\0';
    
    // Kiểm tra username hợp lệ
    if (!validate_username(username)) {
        strcpy(response, "ERROR Cú pháp không hợp lệ");
        return;
    }
    
    // Kiểm tra số điện thoại hợp lệ
    if (!validate_phone(old_phone) || !validate_phone(new_phone)) {
        strcpy(response, "ERROR Cú pháp không hợp lệ");
        return;
    }
    
    // Đọc tất cả users từ file
    user_count = read_users(users, MAX_USERS);
    
    // Tìm user
    int user_index = find_user(users, user_count, username);
    if (user_index == -1) {
        strcpy(response, "ERROR Tài khoản không tồn tại");
        return;
    }
    
    // Kiểm tra old_phone có khớp không
    if (strcmp(users[user_index].phone, old_phone) != 0) {
        strcpy(response, "ERROR Số điện thoại cũ không đúng");
        return;
    }
    
    // Cập nhật new_phone
    strcpy(users[user_index].phone, new_phone);
    
    // Ghi lại file
    if (write_users(users, user_count)) {
        strcpy(response, "OK Cập nhật thành công");
    } else {
        strcpy(response, "ERROR Không thể cập nhật file");
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
    
    printf("Phone Update Server đang chạy trên port %d...\n", PORT);
    printf("File contacts: %s\n", CONTACTS_FILE);
    
    while (1) {
        printf("Đang đợi kết nối...\n");
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                               (socklen_t*)&addrlen)) < 0) {
            perror("Accept");
            exit(EXIT_FAILURE);
        }
        
        printf("Client đã kết nối!\n");
        
        // Đọc dữ liệu từ client
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread > 0) {
            buffer[valread] = '\0';
            printf("Nhận từ client: %s\n", buffer);
            
            // Xử lý cập nhật số điện thoại
            process_phone_update(buffer, response);
            
            // Gửi phản hồi
            send(new_socket, response, strlen(response), 0);
            printf("Phản hồi: %s\n", response);
        }
        
        close(new_socket);
        printf("Đã đóng kết nối với client\n\n");
    }
    
    return 0;
}