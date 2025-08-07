#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>



volatile sig_atomic_t got_signal = 0;

void signal_handler(int sig) {
    got_signal = 1;
    
}
void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}
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
    sigset_t block_mask, orig_mask;
    
    // Setup signal handler for SIGINT
    setup_signal_handler();

    // Initialize all client sockets to 0
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);
    // Sử dụng khi cho pselect 
    // sigset_t sigmask;
    // sigemptyset(&sigmask);
    // sigaddset(&sigmask, SIGINT);
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGINT);
    sigprocmask(SIG_BLOCK, &block_mask, &orig_mask);
    
    while (1) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++) {
            // Socket descriptor
            int sd = client_sockets[i];

            // If valid socket descriptor then add to read list
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }

            // Keep track of the maximum socket descriptor
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Wait for activity on one of the sockets
        //Su dung pselect
        int activity = pselect(max_sd + 1, &readfds, NULL, NULL, NULL, &orig_mask);
        // int activity = pselect(max_fd + 1, &readfds, NULL, NULL, NULL, &sigmask);

        // Su dung select
        // activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) ) {
            if(errno == EINTR){
                printf("Dong cac client\n");
                for(int j = 0; j<MAX_CLIENTS;j++){
                    if(client_sockets[j] != 0 ){
                        send(client_sockets[j], "Server dong client\n", 35, 0);
                        close(client_sockets[j]);
                    }
                } 
                break;
            }
            else{
            printf("select error");
            }
        }

        // If something happened on the master socket, then it's an incoming connection
        if (FD_ISSET(server_fd, &readfds)) {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip is : %s, port : %d\n", 
                   new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++) {
                // If position is empty
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    for(int j = 0;j <MAX_CLIENTS; j++ ){
                        if (client_sockets[j] != 0 && j != i){
                            send(client_sockets[j], "New client connected\n", 30, 0) ;
                        }
                    }
                    break;
                }
            }
        }

        // Check for IO operations on other sockets
        for (i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_sockets[i];

            if (FD_ISSET(sd, &readfds)) {
                int valread;
                if ((valread = read(sd, buffer, BUFFER_SIZE)) == 0) {
                    // Client disconnected
                    socklen_t addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    
                    sprintf(client_message,"Client disconnected, ip %s, port %d/n", 
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    for(int j = 0; j<MAX_CLIENTS;j++){
                        if(client_sockets[j] != 0 && j != i){
                            send(client_sockets[j], client_message, 30, 0);
                        }
                    }
                        

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    client_sockets[i] = 0;
                } else {
                    // Process the incoming message
                    buffer[valread] = '\0';
                    printf("Message from client %d: %s", i, buffer);

                    // Optionally, send a response back to the client
                    
                    sprintf(client_message, "Message from client %d: %s", i, buffer);
                    for(int j = 0; j<MAX_CLIENTS;j++){
                        if(client_sockets[j] != 0 && j != i){
                            send(client_sockets[j],client_message, 30, 0);
                        }
                    }
                    
                }
            }
        }
    }
    printf("Dong server\n");
    close(server_fd);
    sigprocmask(SIG_SETMASK, &orig_mask, NULL);
    return 0;
}
