#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include "http_message.h"

#define PORT 8080

int request_so_far = 0;
int total_bytes_received = 0;
int total_bytes_sent = 0;
const char *bad_request = "HTTP/1.1 400 Bad Request\r\n\r\n";

const char STATIC_PATH[] = "/static"; 
const char STATUS[] = "/stats";
const char CALC[] = "/calc"; 

pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_static(int client_socket, const char *file_path) {
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "./static%s", file_path);

    FILE *file = fopen(full_path, "rb");
    if (!file) {
        const char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client_socket, not_found_response, strlen(not_found_response), 0);
        total_bytes_sent += strlen(not_found_response);
        return;
    }

    const char *header = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\n\r\n";
    send(client_socket, header, strlen(header), 0);
    total_bytes_sent += strlen(header);

    char buffer[1024];
    int bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
        total_bytes_sent += bytes_read;
    }

    fclose(file);
}

void handle_stats(int client_socket) {
    pthread_mutex_lock(&stats_mutex);
    char response[256];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
             "<html><body>\n"
             "<h1>Server Statistics</h1>\n"
             "<p>Requests received: %d</p>\n"
             "<p>Total bytes received: %d</p>\n"
             "<p>Total bytes sent: %d</p>\n"
             "</body></html>\n",
             request_so_far, total_bytes_received, total_bytes_sent);
    pthread_mutex_unlock(&stats_mutex);

    send(client_socket, response, strlen(response), 0);
    total_bytes_sent += strlen(response);
}

void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr;
    free(client_socket_ptr);

    http_read_result_t result;
    http_client_message_t *msg = read_http_client_message(client_socket, &result);

    pthread_mutex_lock(&stats_mutex);
    request_so_far++;
    total_bytes_received += msg ? msg->body_length : 0;
    pthread_mutex_unlock(&stats_mutex);

    if (!msg || result == BAD_REQUEST) {
        send(client_socket, bad_request, strlen(bad_request), 0);
        total_bytes_sent += strlen(bad_request);
    } else if (result == MESSAGE) {
        if (strncmp(msg->path, STATIC_PATH, 7) == 0) {
            handle_static(client_socket, msg->path + 7); 
        } else if (strcmp(msg->path, STATUS) == 0) {
            handle_stats(client_socket);
        } else if (strncmp(msg->path, CALC, 5) == 0) {
            int ax = 0, bx = 0;
            
            if (sscanf(msg->path, "/calc?a=%d&b=%d", &ax, &bx) == 2) {
                int sum = ax + bx;
                char response[1024];
                snprintf(response, sizeof(response), 
                        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nSum of %d and %d is %d\n", 
                        ax, bx, sum);
                send(client_socket, response, strlen(response), 0);
                total_bytes_sent += strlen(response);
        } else {
            send(client_socket, bad_request, strlen(bad_request), 0);
            total_bytes_sent += strlen(bad_request);
        }
}

    }

    http_client_message_free(msg);
    close(client_socket);
    return NULL;
}

int main() {
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(socket_desc, (struct sockaddr *)&server_address, sizeof(server_address)); 
    listen(socket_desc, SOMAXCONN); 

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        int *client_socket = malloc(sizeof(int));
        *client_socket = accept(socket_desc, NULL, NULL);
        if (*client_socket == -1) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_client, client_socket);
        pthread_detach(thread_id);
    }

    close(socket_desc);
    return 0;
}
