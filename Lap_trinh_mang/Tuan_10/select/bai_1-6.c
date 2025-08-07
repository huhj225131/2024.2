#include <stdio.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
int main() {
    fd_set read_fds; 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    FD_ZERO(&read_fds);    // Khởi tạo tập FD rỗng 
    FD_SET(sockfd, &read_fds);  // Thêm sockfd vào tập 
    if (FD_ISSET(sockfd, &read_fds)) { 
        printf("Socket có sẵn để đọc\n"); 
        // if(FD_CLR(sockfd, &read_fds)){
        //     printf("Đã xóa socket\n");
        // } 
        
    } 
    select(sockfd + 1, &read_fds, NULL, NULL, NULL); 
    struct timeval timeout; 
    timeout.tv_sec = 5; 
    timeout.tv_usec = 0; 
    select(sockfd + 1, &read_fds, NULL, NULL, &timeout); 
    

}