#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_ADDR_STRLEN 128

typedef struct quizz{
    char* ques;
    int ans[4];
    int true_ans;
    struct quizz* next;
} quizz;

quizz* quizzes = NULL;

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}


void shuffle(int arr[], int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap(&arr[i], &arr[j]);
    }
}
void add_ques(char* ques , int ans[], int true_ans){
    quizz* new_quizz = (quizz *) malloc(sizeof(quizz));
    new_quizz->ques = strdup(ques);
    for (int i = 0; i < 4; i++) {
        new_quizz->ans[i] = ans[i];  
    }
    new_quizz->true_ans = true_ans;
    new_quizz->next = NULL;
    if (quizzes == NULL){
        quizzes = new_quizz;
    }
    else{
        quizz * temp = quizzes;
        while(temp->next != NULL){
            temp = temp->next;
        }
        temp->next = new_quizz;

    }
}
    void free_quizz(quizz * top){
        quizz * temp = top;
        if(temp->next !=NULL){
            free_quizz(temp->next);
        }
        free(temp);
    }


    void handle_client(int new_socket){
        int scores = 0;
        char buffer[BUFFER_SIZE];
        quizz * temp = quizzes;
        while(temp != NULL){
            shuffle(temp->ans, 4);

            snprintf(buffer, BUFFER_SIZE, "Câu hỏi: %s\nA. %d  B. %d  C. %d  D. %d\n",
                temp->ques, temp->ans[0], temp->ans[1], temp->ans[2], temp->ans[3]);
            send(new_socket, buffer, strlen(buffer), 0);
            
        int n = recv(new_socket, buffer, BUFFER_SIZE, 0);
        buffer[n] = '\0';     
        int input = atoi(buffer);
        if (input == temp->true_ans){
            scores ++;
            // send(new_socket, "Đúng!\n", strlen("Đúng!\n"), 0);
        }
        else{
            // send(new_socket, "Sai!\n", strlen("Sai!\n"), 0);    
        }
            temp = temp->next;
        }
        snprintf(buffer, BUFFER_SIZE, "Số điểm: %d", scores);
    send(new_socket, buffer, strlen(buffer), 0);
    return;
        
    }
    

    // Signal handler to prevent zombie processes
    void sigchld_handler(int sig) {
        (void)sig; // Ignore unused parameter warning
        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

int main(){
    
    int q1[] = {2,3,4,5};
    int q2[] = {2,3,4,5};
    int q3[] = {6,7,8,9};
    int q4[] = {2,3,4,5};
    int q5[] = {8,9,10,11};
    int q6[] = {5,6,7,8};
    int q7[] = {12,13,14,15};
    int q8[] = {1,2,3,4};
    int q9[] = {6,7,8,9};
    int q10[] = {18,19,20,21};

    add_ques("1 + 1 = ", q1, 2);
    add_ques("5 - 2 = ", q2, 3);
    add_ques("3 × 3 = ", q3, 9);
    add_ques("8 ÷ 2 = ", q4, 4);
    add_ques("6 + 4 = ", q5, 10);
    add_ques("9 - 3 = ", q6, 6);
    add_ques("7 × 2 = ", q7, 14);
    add_ques("10 ÷ 5 = ", q8, 2);
    add_ques("15 - 7 = ", q9, 8);
    add_ques("4 × 5 = ", q10, 20);
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

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
    // Handle SIGCHLD to prevent zombie processes
    signal(SIGCHLD, sigchld_handler);
    printf("Server listening on port %d\n", PORT);
    while (1) {
        // Accept an incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Fork a child process to handle the client
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(new_socket);
        } else if (pid == 0) {
            // Child process: handle the client
            srand(time(NULL));
            close(server_fd);  // Close the listening socket in the child process
            handle_client(new_socket);
            close(new_socket);
            exit(0);
        } else {
            // Parent process: continue accepting new clients
            close(new_socket);  // Close the client socket in the parent process
        }
    }
    free_quizz(quizzes);
    return 0;

}