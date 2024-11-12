#include "http_message.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

http_client_message_t *read_http_client_message(int client_socket, http_read_result_t *result) {
    char buffer[1024];
    ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        *result = CLOSE_CONNECTION;
        return NULL;
    }
    
    buffer[bytes_read] = '\0';
    http_client_message_t *msg = malloc(sizeof(http_client_message_t));
    if (!msg) return NULL;
    
    sscanf(buffer, "%s %s %s", msg->method, msg->path, msg->http_version);

    char *header_end = strstr(buffer, "\r\n\r\n");
    if (header_end) {
        strncpy(msg->headers, buffer, header_end - buffer);
        msg->headers[header_end - buffer] = '\0';
        msg->body_length = bytes_read - (header_end - buffer + 4);
        strncpy(msg->body, header_end + 4, msg->body_length);
    } else {
        msg->headers[0] = '\0';
        msg->body[0] = '\0';
        msg->body_length = 0;
    }

    *result = MESSAGE;
    return msg;
}

void http_client_message_free(http_client_message_t *msg) {
    if (msg) {
        free(msg);
    }
}
