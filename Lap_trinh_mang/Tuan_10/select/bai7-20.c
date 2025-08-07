#define FD_SETSIZE 4096
#include <stdio.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
// int main(){
//     int sockfd1, sockfd2; 
//     fd_set read_fds; 
//     FD_ZERO(&read_fds); 
//     FD_SET(sockfd1, &read_fds); 
//     FD_SET(sockfd2, &read_fds); 
//     int maxfd = (sockfd1 > sockfd2) ? sockfd1 : sockfd2; 
//     // select(maxfd + 1, &read_fds, NULL, NULL, NULL);
//     struct timeval timeout; 
// timeout.tv_sec = 5; 
// timeout.tv_usec = 0; 
// int result = select(maxfd + 1, &read_fds, NULL, NULL, &timeout); 
// if (result > 0) { 
//     // Có FD sẵn sàng 
//     } else if (result == 0) { 
//     printf("Timeout\n"); 
//     } else { 
//     perror("select"); 
//     } 
//     for (int i = 0; i <= maxfd; i++) { 
//         if (FD_ISSET(i, &read_fds)) { 
//         printf("Socket %d", i);
//         } 
//     } 
// }

// int main(){
//     int sockfd; 
//     fd_set read_fds; 
//     FD_ZERO(&read_fds); 
//     FD_SET(0, &read_fds); // stdin 
//     FD_SET(sockfd, &read_fds); 
//     maxfd = (sockfd > 0) ? sockfd : 0; 
//     int ready = select(maxfd + 1, &read_fds, NULL, NULL, NULL);
//     printf("%d socket sẵn sàng\n", ready);  
//     for (int i = 0; i <= maxfd; i++) { 
//         if (!FD_ISSET(i, &read_fds)) { 
//         close(i); 
//         } 
//         } 
// }

int main(){
    fd_set exceptfds; 
FD_ZERO(&exceptfds); 
FD_SET(sockfd, &exceptfds); 
select(sockfd + 1, NULL, NULL, &exceptfds, &timeout); 
}

int my_select(int maxfd, fd_set *readfds, struct timeval *timeout) { 
    return select(maxfd + 1, readfds, NULL, NULL, timeout); 
    }