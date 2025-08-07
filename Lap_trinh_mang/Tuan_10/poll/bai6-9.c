#include <stdio.h> 
#include <poll.h> 
#include <unistd.h>

int main(){
    struct pollfd fds[1]; 
    fds[0].fd = -1; // FD không hợp lệ 
    fds[0].events = POLLIN; 
    if (poll(fds, 1, 1000) > 0) { 
    if (fds[0].revents & POLLNVAL) { 
        printf("FD không hợp lệ\n"); 
        } 
    } 
    fds[0].events = POLLOUT; 
    int ret = poll(fds, 1, 5000); 
    if (fds[0].revents & POLLOUT) { 
        printf("Socket sẵn sàng ghi\n"); 
    }
    fds[0].events = POLLIN | POLLOUT; 
    if (poll(fds, 1, 5000) > 0) { 
        if (fds[0].revents & POLLIN) printf("Có dữ liệu để đọc\n"); 
        if (fds[0].revents & POLLOUT) printf("Sẵn sàng ghi\n"); 
    } 
    if (fds[0].revents & POLLHUP) { 
        printf("Client đóng kết nối\n"); 
    } 
}