// tcp_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "10.0.0.1", &serv_addr.sin_addr);   // Server IP (h1 or h2)

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    char *msg = "Hi";
    send(sock, msg, strlen(msg), 0);

    read(sock, buffer, 1024);
    printf("Server says: %s\n", buffer);

    close(sock);
    return 0;
}

