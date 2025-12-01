#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

struct Fruit { char name[20]; int qty; };
struct Fruit stock[2] = {{"Apple", 5}, {"Mango", 5}};

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    socklen_t len = sizeof(cliaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8080);
    bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("UDP Shop Open...\n");
    while(1) {
        int n = recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';
        
        char response[100] = "Regret: Not Available";
        for(int i=0; i<2; i++) {
            if(strcmp(buffer, stock[i].name) == 0) {
                if(stock[i].qty > 0) {
                    stock[i].qty--;
                    sprintf(response, "Sold 1 %s. Remaining: %d", stock[i].name, stock[i].qty);
                } else {
                    strcpy(response, "Regret: Out of stock");
                }
            }
        }
        sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *)&cliaddr, len);
    }
    return 0;
}
