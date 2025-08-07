#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket, client_sockets[MAX_CLIENTS], max_sd, activity, i;
    struct sockaddr_in address;
    int opt = 1;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    char client_message[BUFFER_SIZE];
    char client_names[MAX_CLIENTS][BUFFER_SIZE];

    // Initialize all client sockets to 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
        memset(client_names[i], 0, BUFFER_SIZE);
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        
        FD_ZERO(&readfds);

       
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            if (sd > max_sd) {
                max_sd = sd;
            }
        }

      
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            printf("select error\n");
        }

       
        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip: %s, port: %d\n", 
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

           
           memset(buffer, 0, BUFFER_SIZE);
            int bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                printf("Failed to receive client name.\n");
                close(new_socket);
                continue;
            }
            buffer[strcspn(buffer, "\n")] = '\0';

   
            for (i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    strncpy(client_names[i], buffer, BUFFER_SIZE - 1);
                    client_names[i][BUFFER_SIZE - 1] = '\0';

                    printf("Adding client: %s\n", client_names[i]);

            
                    snprintf(client_message, BUFFER_SIZE, "New client connected: %s", client_names[i]);
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] != 0 && j != i) {
                            send(client_sockets[j], client_message, strlen(client_message), 0);
                        }
                    }
                    break;
                }
            }
        }

        
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = read(sd, buffer, BUFFER_SIZE);

                if (valread == 0) {
                    printf("Client disconnected: %s\n", client_names[i]);
                    snprintf(client_message, BUFFER_SIZE, "Client disconnected: %s", client_names[i]);
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] != 0 && j != i) {
                            send(client_sockets[j], client_message, strlen(client_message), 0);
                        }
                    }

                    close(sd);
                    client_sockets[i] = 0;
                    memset(client_names[i], 0, BUFFER_SIZE);
                } else {
                    buffer[valread] = '\0';
                    printf("Message from %s: %s", client_names[i], buffer);

                    
                    snprintf(client_message, BUFFER_SIZE, "%s: %s", client_names[i], buffer);
                    for (int j = 0; j < MAX_CLIENTS; j++) {
                        if (client_sockets[j] != 0 && j != i) {
                            send(client_sockets[j], client_message, strlen(client_message), 0);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
