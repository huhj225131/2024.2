#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char ip_address[20];
    char read_address[30];
    printf("Nhap dia chi IP: ");
    if (fgets(ip_address, 20, stdin) == NULL) {
        perror("Loi khi nhap dia chi IP");
        exit(EXIT_FAILURE);
    }

    ip_address[strcspn( ip_address, "\n")] = 0;
    address.sin_family = AF_INET;

    if(inet_pton(AF_INET, ip_address, &address.sin_addr) <= 0){
        printf("Loi chuyen doi 1");
        exit(EXIT_FAILURE);
    }
    if(inet_ntop(AF_INET, &address.sin_addr, read_address , 30)==  NULL){
        printf("Loi chuyen doi 2");
        exit(EXIT_FAILURE);
    }
    printf("Dia chi IP (dang van ban): %s\n", read_address);

    return 0;
}