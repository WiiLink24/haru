#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <curl/curl.h>
#include "../include/defs.h"
#include "../config/cfg.h"

struct upload_status {
    size_t bytes_sent;
};

size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    const char *data = "From: " FROM_EMAIL "\r\n"
                       "To: " TO_EMAIL "\r\n"
                       "Subject: Test Email\r\n"
                       "\r\n"
                       "This is a test email.";
    size_t len = strlen(data);
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
        return 0;
    }
    if (upload_ctx->bytes_sent >= len) {
        return 0;
    }
    size_t remaining = len - upload_ctx->bytes_sent;
    size_t sending = size * nmemb;
    size_t min = (sending < remaining) ? sending : remaining;
    memcpy(ptr, data + upload_ctx->bytes_sent, min);
    upload_ctx->bytes_sent += min;
    return min;
}

void send_email(const char *subject, const char *body) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct upload_status upload_ctx;
    struct curl_slist *recipients = NULL;
    char payload[BUFFER_SIZE];
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, FROM_EMAIL);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, FROM_PASSWORD);
        curl_easy_setopt(curl, CURLOPT_URL, "smtp://" SMTP_SERVER ":" "587");
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);    
        struct curl_slist *recipients = NULL;
        recipients = curl_slist_append(recipients, TO_EMAIL);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM_EMAIL);
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: text/plain");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Email sending failed: %s\n", curl_easy_strerror(res));
        }
        curl_slist_free_all(recipients);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    if (res != CURLE_OK) {
        fprintf(stderr, "Email sending failed: %s\n", curl_easy_strerror(res));
    }
}

void handle_post_request(const char *data) {
    char subject[BUFFER_SIZE];
    char body[BUFFER_SIZE];
    sscanf(data, "subject=%[^&]&body=%s", subject, body);
    send_email(subject, body);
}

void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        if (strstr(buffer, "POST /") != NULL) {
            char *data_start = strstr(buffer, "\r\n\r\n");
            if (data_start != NULL) {
                data_start += 4;
                handle_post_request(data_start);
            }
        }
    }
    const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Email Sent!</h1></body></html>";
    write(client_socket, response, strlen(response));
    close(client_socket);
}
int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size = sizeof(struct sockaddr_in);
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); 
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_socket, 10) < 0) {
        perror("Listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Server running. Waiting for connections...\n");
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &sin_size);
        if (client_socket < 0) {
            perror("Acceptance failed");
            continue;
        }
        handle_request(client_socket);
    }
    close(server_socket);
    return 0;
}

