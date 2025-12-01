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
#define SERVER_IP "10.0.0.1"               // IP of h1
#define DOWNLOAD_FILE "downloaded_from_server.txt"
#define CLIENT_FILE   "client_file.txt"

void download_file(int sockfd, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Client: fopen (download)");
        return;
    }

    char buffer[BUF_SIZE];
    ssize_t n;
    long total_bytes = 0;

    clock_t start = clock();

    while ((n = recv(sockfd, buffer, BUF_SIZE, 0)) > 0) {
        fwrite(buffer, 1, n, fp);
        total_bytes += n;
    }

    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    if (n < 0) {
        perror("Client: recv");
    }

    printf("Client: Downloaded %ld bytes into '%s'\n", total_bytes, filename);
    printf("Client: Time taken to DOWNLOAD = %.6f seconds\n\n", time_taken);

    fclose(fp);
}

void upload_file(int sockfd, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        perror("Client: fopen (upload)");
        return;
    }

    char buffer[BUF_SIZE];
    size_t n;
    long total_bytes = 0;

    clock_t start = clock();

    while ((n = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        if (send(sockfd, buffer, n, 0) != (ssize_t)n) {
            perror("Client: send");
            break;
        }
        total_bytes += n;
    }

    clock_t end = clock();
    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Client: Uploaded %ld bytes from '%s'\n", total_bytes, filename);
    printf("Client: Time taken to UPLOAD = %.6f seconds\n\n", time_taken);

    fclose(fp);
}

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;

    // ------------ FIRST CONNECTION: DOWNLOAD ------------
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Client: socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Client: inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client: Connecting to server for DOWNLOAD...\n");
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Client: connect (download)");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client: Connected. Starting DOWNLOAD...\n");
    download_file(sockfd, DOWNLOAD_FILE);
    close(sockfd);
    printf("Client: DOWNLOAD finished.\n\n");

    // ------------ SECOND CONNECTION: UPLOAD ------------
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Client: socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Client: inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client: Connecting to server for UPLOAD...\n");
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Client: connect (upload)");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client: Connected. Starting UPLOAD...\n");
    upload_file(sockfd, CLIENT_FILE);
    close(sockfd);
    printf("Client: UPLOAD finished.\n");

    return 0;
}

