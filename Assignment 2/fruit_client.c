#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock;
    struct sockaddr_in server;
    char fruit[20];
    int qty;
    char response[500];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("10.0.0.1");   // Server (h1)
    server.sin_family = AF_INET;
    server.sin_port = htons(9000);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Enter fruit name: ");
    scanf("%s", fruit);
    printf("Enter quantity: ");
    scanf("%d", &qty);

    send(sock, fruit, sizeof(fruit), 0);
    send(sock, &qty, sizeof(qty), 0);

    recv(sock, response, sizeof(response), 0);

    printf("\n----- Server Response -----\n%s\n", response);

    close(sock);
    return 0;
}

