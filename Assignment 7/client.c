#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define S_IP "10.0.0.1"
#define P 55550
#define B 1024
#define T_OUT 3

int main() {
    int c_fd;
    struct sockaddr_in s_a;
    socklen_t a_l = sizeof(s_a);
    char buf[B];
    
    c_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // Set timeout
    struct timeval t_v;
    t_v.tv_sec = T_OUT;
    t_v.tv_usec = 0;
    setsockopt(c_fd, SOL_SOCKET, SO_RCVTIMEO, &t_v, sizeof(t_v));
    
    s_a.sin_family = AF_INET;
    s_a.sin_port = htons(P);
    s_a.sin_addr.s_addr = inet_addr(S_IP);
    
    printf("UDP Calculator Client\n");
    printf("Server: %s:%d\n", S_IP, P);
    printf("Format: op x [y]\n");
    printf("Operations: sin, cos, tan, log, sqrt, inv, +, -, *, /, pow\n");
    printf("Type 'quit' to exit\n\n");
    
    int pkt_sent = 0, pkt_recv = 0, pkt_lost = 0;
    
    while(1) {
        printf("Enter operation: ");
        fgets(buf, B, stdin);
        
        if(buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';
        
        if(strcmp(buf, "quit") == 0) break;
        if(strlen(buf) == 0) continue;
        
        // Send request
        pkt_sent++;
        sendto(c_fd, buf, strlen(buf), 0, (struct sockaddr*)&s_a, a_l);
        printf("Sent: %s (Packet #%d)\n", buf, pkt_sent);
        
        // Receive response
        memset(buf, 0, B);
        int b_r = recvfrom(c_fd, buf, B-1, 0, (struct sockaddr*)&s_a, &a_l);
        
        if(b_r > 0) {
            pkt_recv++;
            buf[b_r] = '\0';
            printf("Server: %s\n", buf);
        } else {
            pkt_lost++;
            printf("Packet lost! No response from server.\n");
        }
        
        printf("Stats: Sent=%d, Received=%d, Lost=%d\n\n", 
               pkt_sent, pkt_recv, pkt_lost);
    }
    
    printf("\nFinal Stats:\n");
    printf("Total Sent: %d\n", pkt_sent);
    printf("Total Received: %d\n", pkt_recv);
    printf("Total Lost: %d\n", pkt_lost);
    printf("Success Rate: %.1f%%\n", (pkt_recv*100.0)/pkt_sent);
    
    close(c_fd);
    return 0;
}
