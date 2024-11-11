/*Socket programming is a way of connecting two nodes on a network to communicate with each other. 
One socket(node) listens on a particular port at an IP, while the other socket reaches out to the 
other to form a connection.*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

// Requests 
char static_path[] = "GET /static"; 
char status[] = "GET /stats";
char calc[] = "GET /calc"; 

// address structure used by bind to associate the socket with the specified IP address and port, 
// letting the server receive connections at that address.
struct sockaddr_in server_address = {
    .sin_family = AF_INET,
    .sin_addr.s_addr = INADDR_ANY,
    .sin_port = 0  
};

void *handle_client(void *client_socket){
    int client_socket_desc = *(int *)client_socket; 
    char buffer[1024]; 
    ssize_t bytes_read = recv(client_socket_desc, buffer, sizeof(buffer) - 1, 0); 

    if (bytes_read > 0){
        buffer[bytes_read] = '\0'; 
    }

    if (strncmp(buffer, static_path, strlen(static_path)) == 0) {

    } else if (strncmp(buffer, status, strlen(status)) == 0){

    } else if (strncmp(buffer, status, strlen(calc)) == 0) {

    } else {}

}

int main(int argc, char *argv[]){
    int port;
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[2]); 
    server_address.sin_port = htons(port); 

    bind(socket_desc, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(socket_desc, SOMAXCONN);

    while (1){
        int client_socket = accept(socket_desc, NULL, NULL);

        pthread thread_id; 
        pthread_create(&thread_id, NULL, handle_client, (void *)&client_socket);
    }
}