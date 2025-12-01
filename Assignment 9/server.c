#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 8080
#define BUF_SIZE 1024
#define SERVER_FILE "server_file.txt"
#define RECEIVED_FILE "received_from_client.txt"

void send_file(int client_fd, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Server: fopen (send)");
        return;
    }

    char buffer[BUF_SIZE];
    size_t n;
    long total_bytes = 0;

    clock_t start = clock();

    while ((n = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        if (send(client_fd, buffer, n, 0) != (ssize_t)n) {
            perror("Server: send");
            break;
        }
        total_bytes += n;
    }

    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Server: Sent %ld bytes from '%s'\n", total_bytes, filename);
    printf("Server: Time taken to SEND = %.6f seconds\n\n", time_taken);

    fclose(fp);
}

void receive_file(int client_fd, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Server: fopen (receive)");
        return;
    }

    char buffer[BUF_SIZE];
    ssize_t n;
    long total_bytes = 0;

    clock_t start = clock();

    while ((n = recv(client_fd, buffer, BUF_SIZE, 0)) > 0) {
        fwrite(buffer, 1, n, fp);
        total_bytes += n;
    }

    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    if (n < 0) {
        perror("Server: recv");
    }

    printf("Server: Received %ld bytes into '%s'\n", total_bytes, filename);
    printf("Server: Time taken to RECEIVE = %.6f seconds\n\n", time_taken);

    fclose(fp);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Server: socket");
        exit(EXIT_FAILURE);
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // listen on all interfaces
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("Server: bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 5) == -1) {
        perror("Server: listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server: Listening on port %d...\n", PORT);

    // -------- FIRST CONNECTION: CLIENT DOWNLOADS FILE --------
    printf("Server: Waiting for client (DOWNLOAD)...\n");
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == -1) {
        perror("Server: accept (download)");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server: Client connected for DOWNLOAD.\n");
    send_file(client_fd, SERVER_FILE);
    close(client_fd);
    printf("Server: DOWNLOAD finished.\n\n");

    // -------- SECOND CONNECTION: CLIENT UPLOADS FILE --------
    printf("Server: Waiting for client (UPLOAD)...\n");
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == -1) {
        perror("Server: accept (upload)");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server: Client connected for UPLOAD.\n");
    receive_file(client_fd, RECEIVED_FILE);
    close(client_fd);
    printf("Server: UPLOAD finished.\n");

    close(server_fd);
    return 0;
}

