// udp_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <port> <fruit> <qty>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    char buffer[BUFSIZE];
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);

    // Prepare request
    sprintf(buffer, "%s %s", argv[3], argv[4]);

    sendto(sockfd, buffer, strlen(buffer), 0,
           (struct sockaddr*)&servaddr, sizeof(servaddr));

    int n = recvfrom(sockfd, buffer, BUFSIZE, 0, NULL, NULL);
    buffer[n] = '\0';

    printf("Server: %s\n", buffer);

    close(sockfd);
    return 0;
}

