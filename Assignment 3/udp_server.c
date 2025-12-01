// udp_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define BUFSIZE 1024

struct Fruit {
    char name[20];
    int quantity;
    char lastSold[40];
};

int main() {
    int sockfd;
    char buffer[BUFSIZE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);

    // Fruit database
    struct Fruit fruits[3] = {
        {"apple", 10, "none"},
        {"banana", 20, "none"},
        {"mango", 5, "none"}
    };

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("Socket"); exit(1); }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind");
        exit(1);
    }

    printf("UDP Server running on port %d...\n", PORT);

    while (1) {
        int n = recvfrom(sockfd, buffer, BUFSIZE, 0,
                         (struct sockaddr*)&cliaddr, &len);
        buffer[n] = '\0';

        char fruitName[20];
        int qtyReq;
        sscanf(buffer, "%s %d", fruitName, &qtyReq);

        printf("Client requested: %s %d\n", fruitName, qtyReq);

        int found = 0;

        for (int i = 0; i < 3; i++) {
            if (strcmp(fruitName, fruits[i].name) == 0) {
                found = 1;

                if (qtyReq <= fruits[i].quantity) {
                    fruits[i].quantity -= qtyReq;

                    // Timestamp update
                    time_t now = time(NULL);
                    strcpy(fruits[i].lastSold, ctime(&now));

                    sprintf(buffer,
                            "Success! %s sold. Remaining: %d\nLast Sold: %s",
                            fruits[i].name, fruits[i].quantity,
                            fruits[i].lastSold);
                } else {
                    sprintf(buffer,
                            "REGRET! Only %d %s available.",
                            fruits[i].quantity, fruits[i].name);
                }
                break;
            }
        }

        if (!found) {
            sprintf(buffer, "Fruit not found!");
        }

        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&cliaddr, len);
    }

    close(sockfd);
    return 0;
}

