#include <stdio.h> 
#include <poll.h> 
#include <unistd.h>

int main(){
    struct pollfd fds[2]; 
    fds[0].fd = sockfd1; 
    fds[0].events = POLLIN; 
    fds[1].fd = sockfd2; 
    fds[1].events = POLLIN; 
    poll(fds, 2, 10000); // 10 giây timeout 
    for (int i = 0; i < 2; i++) { 
        if (fds[i].revents & POLLIN) { 
        printf("Socket %d có dữ liệu\n", fds[i].fd); 
        } 
    } 
    poll(fds, 2, 0);
}