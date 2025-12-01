#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char fruit[20];
    char buffer[1024];
    socklen_t len = sizeof(servaddr);

    printf("Enter fruit: ");
    scanf("%s", fruit);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8080);
    inet_pton(AF_INET, "10.0.0.1", &servaddr.sin_addr);

    sendto(sockfd, fruit, strlen(fruit), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    int n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&servaddr, &len);
    buffer[n] = '\0';
    printf("Server: %s\n", buffer);
    return 0;
}
