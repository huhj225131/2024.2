#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>


#define PORT 8080
#define BUFFER_SIZE 100

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    fd_set read_fds;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Change this if needed

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // printf("Connected to the server.\n");
    printf("Nhap ten \n");
    fgets(buffer, sizeof(buffer), stdin);
                // Send the input to the server
    send(sockfd, buffer, strlen(buffer), 0);
    while (1) {
        // Initialize the file descriptor set
        printf("Enter message to send to server: \n");
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds); // Monitor standard input
        FD_SET(sockfd, &read_fds);  // Monitor the socket to get message from server
         // Determine the maximum file descriptor value
        int max_fd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;
        
        int result = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (result == -1) {
            perror("select");
            break;
        }
        
        else if(result > 0){
            if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                // Read input from the user
                fgets(buffer, sizeof(buffer), stdin);
                // Send the input to the server
                send(sockfd, buffer, strlen(buffer), 0);
            }
        
            if (FD_ISSET(sockfd, &read_fds)) {
                int valread;
                if ((valread = read(sockfd, buffer, BUFFER_SIZE)) != 0){
                    buffer[valread] = '\0';
                    printf("%s", buffer);
                }
                else if(valread == 0){
                    
                    break;
                }
            }
        }
    }

    // Clean up
    close(sockfd);
    return 0;
}
