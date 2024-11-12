#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <stdlib.h>

typedef struct {
    char method[8];
    char path[256];
    char http_version[16];
    char body[1024];
    int body_length;
    char headers[512];
} http_client_message_t;

typedef enum {
    BAD_REQUEST,
    CLOSE_CONNECTION,
    MESSAGE
} http_read_result_t;

http_client_message_t *read_http_client_message(int client_sock, http_read_result_t *result);
void http_client_message_free(http_client_message_t *msg);

#endif
