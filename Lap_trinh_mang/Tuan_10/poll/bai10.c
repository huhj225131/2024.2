#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket, i;
    struct sockaddr_in address;
    int opt = 1;
    struct pollfd clients[MAX_CLIENTS + 1];
    int num_clients = 1;  // Ban đầu chỉ có server
    char buffer[BUFFER_SIZE];
    char client_message[BUFFER_SIZE];

    // Khởi tạo danh sách client
    for (i = 0; i <= MAX_CLIENTS; i++) {
        clients[i].fd = 0;
        clients[i].events = POLLIN;
    }

    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Cấu hình socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);
    clients[0].fd = server_fd;
    clients[0].events = POLLIN;

    while (1) {
        int activity = poll(clients, num_clients, -1);

        if (activity < 0 && errno != EINTR) {
            perror("Poll error");
        }

        // Kiểm tra kết nối mới
        if (clients[0].revents & POLLIN) {
            socklen_t addrlen = sizeof(address);
            new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket < 0) {
                perror("Accept");
                continue;
            }

            printf("New connection: socket fd = %d, ip = %s, port = %d\n",
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Thêm client mới vào danh sách
            int added = 0;
            for (i = 1; i <= MAX_CLIENTS; i++) {
                if (clients[i].fd == 0) {
                    clients[i].fd = new_socket;
                    clients[i].events = POLLIN;
                    num_clients++;
                    added = 1;
                    
                    // Thông báo tới các client khác
                    snprintf(client_message, sizeof(client_message), "New client connected\n");
                    for (int j = 1; j <= MAX_CLIENTS; j++) {
                        if (clients[j].fd != 0 && j != i) {
                            send(clients[j].fd, client_message, strlen(client_message), 0);
                        }
                    }
                    break;
                }
            }

            if (!added) {
                printf("Too many clients, rejecting connection.\n");
                close(new_socket);
            }
        }

        // Kiểm tra dữ liệu từ client
        for (i = 1; i <= MAX_CLIENTS; i++) {
            int sd = clients[i].fd;

            if (sd > 0 && (clients[i].revents & POLLIN)) {
                int valread = read(sd, buffer, BUFFER_SIZE - 1);
                if (valread <= 0) {
                    // Client ngắt kết nối
                    socklen_t addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Client disconnected: ip %s, port %d\n", 
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    snprintf(client_message, sizeof(client_message),
                             "Client disconnected: ip %s, port %d\n",
                             inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    for (int j = 1; j <= MAX_CLIENTS; j++) {
                        if (clients[j].fd != 0 && j != i) {
                            send(clients[j].fd, client_message, strlen(client_message), 0);
                        }
                    }

                    close(sd);
                    clients[i].fd = 0;
                    num_clients--;
                } else {
                    // Nhận tin nhắn từ client
                    buffer[valread] = '\0';
                    printf("Message from client %d: %s", i, buffer);

                    snprintf(client_message, sizeof(client_message),
                             "Message from client %d: %s", i, buffer);

                    // Gửi tin nhắn tới các client khác
                    for (int j = 1; j <= MAX_CLIENTS; j++) {
                        if (clients[j].fd != 0 && j != i) {
                            send(clients[j].fd, client_message, strlen(client_message), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
