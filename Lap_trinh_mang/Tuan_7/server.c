#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];
void notify(int client){
    buffer[0] = 0x05;
    send(client, buffer, strlen(buffer),0);
}
int opt = 1;
int game_board[3][3] = {0};


int check_win() {
    // Kiểm tra hàng ngang
    for (int i = 0; i < 3; i++) {
        if (game_board[i][0] != 0 &&
            game_board[i][0] == game_board[i][1] &&
            game_board[i][1] == game_board[i][2]) {
            return game_board[i][0]; // 1 hoặc 2 thắng
        }
    }

    // Kiểm tra hàng dọc
    for (int i = 0; i < 3; i++) {
        if (game_board[0][i] != 0 &&
            game_board[0][i] == game_board[1][i] &&
            game_board[1][i] == game_board[2][i]) {
            return game_board[0][i]; // 1 hoặc 2 thắng
        }
    }

    // Kiểm tra haihai đường chéo chính
    if (game_board[0][0] != 0 &&
        game_board[0][0] == game_board[1][1] &&
        game_board[1][1] == game_board[2][2]) {
        return game_board[0][0]; // 1 hoặc 2 thắng
    }

    
    if (game_board[0][2] != 0 &&
        game_board[0][2] == game_board[1][1] &&
        game_board[1][1] == game_board[2][0]) {
        return game_board[0][2]; // 1 hoặc 2 thắng
    }

    // Kiểm tra hòa (không còn ô trống)
    int is_full = 1;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (game_board[i][j] == 0) {
                is_full = 0;
                break;
            }
        }
    }

    if (is_full) {
        return 3; // Hòa
    }

    return 0; // Chưa kết thúc
}
void res_send(int cl_1, int cl_2, int res) {
    char buffer[BUFFER_SIZE];
    buffer[0] = 0x04; 

    const char *message;

    if (res == 1) {
        message = "Nguoi choi X thang\n";
    } else if (res == 2) {
        message = "Nguoi choi O thang\n";
    } else {
        message = "Tran dau hoa\n";
    }

    // Ghi message vào buffer từ vị trí thứ 2 (sau header)
    snprintf(buffer + 1, BUFFER_SIZE - 1, "%s", message);

    // Gửi cho cả 2 client
    send(cl_1, buffer, strlen(buffer), 0);
    send(cl_2, buffer, strlen(buffer), 0);
}

void update(int cl_1, int cl_2){
    buffer[0] = 0x03;
    for(int i = 0 ; i < 3; i++ ){
        for(int j = 0 ; j < 3; j++){
            if(game_board[i][j] == 0){
                buffer[i * 3 + j + 1] = ' ';
            }
            else if(game_board[i][j] == 1){
                buffer[i * 3 + j + 1] = 'X';
            }
            else if(game_board[i][j] == 2){
                buffer[i * 3 + j + 1] = 'O';
            }
        }
    }
    send(cl_1, buffer, strlen(buffer),0);
    send(cl_2, buffer,  strlen(buffer),0);
}
bool check_board(int row, int col , int mark){
    if (row < 4 && row >0 && col < 4 && col > 0){
        if (game_board[row - 1][col - 1] == 0){
            game_board[row - 1][col - 1] = mark;
            return true;
        }
        return false;
    }
    return false;
}
int main(){
    int tcp_socket, client_1, client_2;
    struct sockaddr_in address;
    
    
    // Create socket
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(tcp_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(tcp_socket, 2) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    socklen_t addrlen = sizeof(address);
    client_1 = accept(tcp_socket, (struct sockaddr *)&address, &addrlen);
    client_2 =  accept(tcp_socket, (struct sockaddr *)&address, &addrlen);
    int valread;
    while(1){
        while(1){
        printf("goi 1\n");
        notify(client_1);
        valread = read(client_1,buffer, BUFFER_SIZE);
        if(buffer[0] == 0x02){
            int row = buffer[1] - '0';
            int col =buffer[2] - '0';
            if (check_board(row, col, 1) == false){
                continue;
            }
            else{
                break;
            }
        } 
    }
    
    update(client_1, client_2);
    printf("up cho sau khi 1 danh\n");
    if(check_win() != 0){
        break;
    }
    while(1){
        printf("goi 2\n");
        notify(client_2);
        valread = read(client_2,buffer, BUFFER_SIZE);
        if(buffer[0] == 0x02){
            int row = buffer[1] - '0';
            int col =buffer[2] - '0';
            if (check_board(row, col, 2) == false){
                continue;
            }
            else{
                break;
            }
        } 
    }
    
    update(client_1, client_2);
    printf("up cho sau khi 2 danh\n");
    if(check_win() != 0){
        break;
    } 

    }
    int result = check_win();
    res_send(client_1, client_2, result);
    close(client_1);
    close(client_2);
    close(tcp_socket);

}