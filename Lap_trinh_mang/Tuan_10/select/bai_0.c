#include <stdio.h> 
#include <sys/select.h> 
int main() { 
    fd_set fdset; 
    FD_ZERO(&fdset); 
    FD_SET(3, &fdset); // Set bit thứ 3 
    FD_SET(4, &fdset); // Set bit thứ 4 
    printf("__fds_bits[0] = %ld\n", fdset.__fds_bits[0]); 
    // Hiển thị giá trị nhị phân của __fds_bits[0] 
    for (int i = sizeof(fdset.__fds_bits[0])*8 - 1; i >= 0; i--) { 
    printf("%ld", (fdset.__fds_bits[0] >> i) & 1); 
    } 
    printf("\n"); 
    return 0; 
} 