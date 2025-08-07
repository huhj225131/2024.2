#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    // int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    if(inet_aton("192.168.1.1", &address.sin_addr) == 0){
        printf("Invalid IP");
        exit(EXIT_FAILURE);
    }
}