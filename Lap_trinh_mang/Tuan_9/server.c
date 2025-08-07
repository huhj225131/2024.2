#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];
void handle_client(int client){
    int valread = recv(client, buffer, BUFFER_SIZE, 0);
    if (valread <= 0) {
        perror("recv failed");
        return;
    }
    else{
        const char *content_type = "text/plain";
        FILE *file;
        char file_buffer[8192];
        char header[256];
        if(strncmp(buffer, "GET / HTTP/1.1", strlen("GET / HTTP/1.1")) == 0){
            content_type="text/html";
            file = fopen("index.html", "r");
            
        }
        else if(strncmp(buffer, "GET /style_3.css HTTP/1.1", strlen("GET /style_3.css HTTP/1.1")) == 0){
            content_type="text/css";
            file=fopen("style_3.css", "r");
        }
        else if(strncmp(buffer, "GET /3.jpg HTTP/1.1", strlen("GET /3.jpg HTTP/1.1")) == 0){
            content_type="image/jpeg";
            file=fopen("3.jpg", "r");
        }
        else if(strncmp(buffer, "POST ", strlen("POST ")) == 0){
            char *body_start = strstr(buffer, "\r\n\r\n");

            if(body_start != NULL){
                body_start += 4;
                printf("Body:%s\n", body_start);
                FILE *data = fopen("data.txt", "w");
                fprintf(data, "%s", body_start);
                fclose(data);
                content_type="text/html";
                file = fopen("post_rep.html", "r");
            }
        }
        else if(strncmp(buffer, "HEAD ", strlen("HEAD ")) == 0){
            content_type="text/html";
            file = fopen("index.html", "r");
            if (file == NULL){
                const char *not_found =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                send(client, not_found, strlen(not_found), 0);
                return;
            }
            size_t bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file);
            
            fclose(file);
            
            snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %zu\r\n"
                "\r\n",
                content_type,bytes_read
            );
    
            send(client, header, strlen(header), 0);
            return;

        }
        else if (strncmp(buffer, "PUT ", strlen("PUT ")) == 0) {

            char *body_start = strstr(buffer, "\r\n\r\n");
            if (body_start != NULL) {
                body_start += 4; 
                
              
                char filename[256] = {0}; 
                sscanf(buffer, "PUT /%s HTTP/1.1", filename);
        
                
                char *qmark = strchr(filename, '?');
                if (qmark) *qmark = '\0';
        
               
                printf("Body: %s\n", body_start);
        
                
                content_type = "text/plain";  
        
                FILE *data = fopen(filename, "w");
                fprintf(data, "%s", body_start);
                fclose(data);
                file = fopen(filename,"r");

            }
        }   
        else if (strncmp(buffer, "DELETE ", strlen("DELETE ")) == 0){
            char filename[256] = {0}; 
            sscanf(buffer, "DELETE /%s HTTP/1.1", filename);
            char *qmark = strchr(filename, '?');
            if (qmark) *qmark = '\0';
            if (remove(filename) == 0) {
                // Thành công, trả về 200 OK
                const char *success_response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                send(client, success_response, strlen(success_response), 0);
            } else {
                printf("Khong có");
                // Lỗi xóa file, trả về 404 Not Found
                const char *not_found_response =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                send(client, not_found_response, strlen(not_found_response), 0);
            }
            return;

        }     
        else{
        const char *method_not_allowed =
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Content-Length: 0\r\n"
            "Allow: GET POST HEAD PUT DELETE\r\n"  
            "\r\n";
        send(client, method_not_allowed, strlen(method_not_allowed), 0);
        return;
        }
        if (file == NULL){
            const char *not_found =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Length: 0\r\n"
                "\r\n";
            send(client, not_found, strlen(not_found), 0);
            return;
        }
        size_t bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file);
        
        fclose(file);
        
        snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "\r\n",
            content_type,bytes_read
        );

        send(client, header, strlen(header), 0);
        send(client, file_buffer, bytes_read, 0);
        return;
    }

}
int main(){
    int tcp_socket, new_socket;
    struct sockaddr_in address;
    
    // Create socket
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    int opt = 1;
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
    signal(SIGCHLD, SIG_IGN);
    while(1){
        if ((new_socket = accept(tcp_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            close(new_socket);
        }
        else if (pid == 0) {
            // Child process: handle the client
            srand(time(NULL));
            close(tcp_socket);  // Close the listening socket in the child process
            handle_client(new_socket);
            close(new_socket);
            exit(0);
        }
        else{
            close(new_socket);
        }

    }
    
    close(tcp_socket);
}