#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define P 55550
#define B 1024

double calc(char* op, double x, double y) {
    if(strcmp(op, "sin") == 0) return sin(x);
    if(strcmp(op, "cos") == 0) return cos(x);
    if(strcmp(op, "tan") == 0) return tan(x);
    if(strcmp(op, "log") == 0) return log(x);
    if(strcmp(op, "sqrt") == 0) return sqrt(x);
    if(strcmp(op, "inv") == 0) return 1.0/x;
    if(strcmp(op, "+") == 0) return x + y;
    if(strcmp(op, "-") == 0) return x - y;
    if(strcmp(op, "*") == 0) return x * y;
    if(strcmp(op, "/") == 0) return x / y;
    if(strcmp(op, "pow") == 0) return pow(x, y);
    return 0.0;
}

int main() {
    int s_fd;
    struct sockaddr_in s_a, c_a;
    socklen_t a_l = sizeof(c_a);
    char buf[B];
    
    s_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    s_a.sin_family = AF_INET;
    s_a.sin_addr.s_addr = INADDR_ANY;
    s_a.sin_port = htons(P);
    
    bind(s_fd, (struct sockaddr*)&s_a, sizeof(s_a));
    
    printf("UDP Calculator Server on port %d\n", P);
    printf("Operations: sin, cos, tan, log, sqrt, inv, +, -, *, /, pow\n");
    
    int pkt_recv = 0, pkt_sent = 0;
    
    while(1) {
        memset(buf, 0, B);
        int b_r = recvfrom(s_fd, buf, B-1, 0, (struct sockaddr*)&c_a, &a_l);
        
        if(b_r > 0) {
            pkt_recv++;
            buf[b_r] = '\0';
            
            char c_ip[16];
            strcpy(c_ip, inet_ntoa(c_a.sin_addr));
            int c_p = ntohs(c_a.sin_port);
            
            printf("\nPacket #%d from %s:%d\n", pkt_recv, c_ip, c_p);
            printf("Request: %s\n", buf);
            
            // Parse: "op x y" or "op x"
            char op[10];
            double x = 0, y = 0;
            int n = sscanf(buf, "%s %lf %lf", op, &x, &y);
            
            char res[B];
            if(n >= 2) {
                double r = calc(op, x, y);
                sprintf(res, "Result: %.6f", r);
                printf("Response: %s\n", res);
            } else {
                strcpy(res, "Error: Use 'op x' or 'op x y'");
                printf("Response: Error\n");
            }
            
            // Send response
            sendto(s_fd, res, strlen(res), 0, (struct sockaddr*)&c_a, a_l);
            pkt_sent++;
            
            printf("Stats: Received=%d, Sent=%d\n", pkt_recv, pkt_sent);
        }
    }
    
    close(s_fd);
    return 0;
}
