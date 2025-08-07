#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 100

int main(){
    int sockfd;
    char line[BUFFER_SIZE];
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
    int valread;
    while(1){
        while(1){
        valread = read(sockfd, buffer, BUFFER_SIZE);
        if (valread != 0){
            // printf("Da doc\n");
            break;
        }
        }
        if (buffer[0] == 0x05){
            // printf("Danh \n");
            printf("Nhap vi tri \n");
            buffer[0] = 0x02;
            fgets(line, sizeof(line), stdin);
            sscanf(line, "%c %c", &buffer[1], &buffer[2]);
            send(sockfd, buffer, 3, 0);  
        }
        else if (buffer[0] == 0x03){
            // printf("ban \n");
            for(int i = 1; i <= 9; i++){
                if(i % 3 == 1){
                    printf("\n %c|", buffer[i]);
                }
                else{
                    printf(" %c|", buffer[i]);
                }
            }
            printf("\n");
        }
        else if(buffer[0] == 0x04){
            // printf("thua \n");
            printf("%s", &buffer[1]);
            break;
        }

    }
    close(sockfd);
}