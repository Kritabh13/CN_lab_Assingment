// tcp_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    char buffer[1024] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    socklen_t addrlen = sizeof(address);
    
    printf("Server is waiting for connection...\n");
    new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);

    read(new_socket, buffer, 1024);
    printf("Client says: %s\n", buffer);

    char *msg = "Hello";
    send(new_socket, msg, strlen(msg), 0);

    close(new_socket);
    close(server_fd);

    return 0;
}

