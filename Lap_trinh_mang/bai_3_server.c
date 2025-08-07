#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_ADDR_STRLEN 128
char *sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    static char str[MAX_ADDR_STRLEN];   // Buffer to hold the string representation
    char portstr[8];                    // Buffer to hold the port as a string

    // Check if the address is IPv4
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *) sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    }
    // Check if the address is IPv6
    else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin6->sin6_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    } else {
        return NULL;  // Unsupported address family
    }
}
int main(){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = "Da nhan thanh cong tu client!";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket failed");
        exit(EXIT_FAILURE);

    }
    
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_fd, (struct sockaddr *) & address, sizeof(address)) <0){
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    char* result;
    if( (result = sock_ntop((struct sockaddr *) &address, sizeof(address))) != NULL){
        printf("Dia chi ip:port client %s  \n" , result);
    }

    send(new_socket, buffer, strlen(buffer), 0);

    close(new_socket);
    close(server_fd);
    return 0;
}