#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/ethernet.h>
#include <net/if.h>

int main() {
    int saddr_size, data_size;
    struct sockaddr saddr;
    unsigned char buffer[65536];

    // 1. Create Raw Socket
    int sock_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(sock_raw < 0) { perror("Socket Error"); return 1; }

    // 2. Bind to Root Switch Interface (s1-eth1)
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "s1-eth1");
    if (setsockopt(sock_raw, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0) {
        perror("Bind failed"); return 1;
    }

    printf("C Analyzer Listening on Root Switch (s1-eth1)...\n");

    while(1) {
        data_size = recvfrom(sock_raw, buffer, 65536, 0, &saddr, (socklen_t*)&saddr_size);
        if(data_size < 0) return 1;

        struct ethhdr *eth = (struct ethhdr *)buffer;
        struct iphdr *iph = (struct iphdr *)(buffer + sizeof(struct ethhdr));

        // Filter for ICMP (Protocol 1)
        if(iph->protocol == 1) {
            struct icmphdr *icmph = (struct icmphdr *)(buffer + sizeof(struct ethhdr) + iph->ihl*4);

            printf("\n--- Packet Captured at Root ---\n");
            // L2: Ethernet
            printf("[L2 Ethernet] Src: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X, Dest: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",
                eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5],
                eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);

            // L3: IP
            struct in_addr source, dest;
            source.s_addr = iph->saddr;
            dest.s_addr = iph->daddr;
            printf("[L3 IP] Src: %s, Dest: %s\n", inet_ntoa(source), inet_ntoa(dest));

            // L4: ICMP
            printf("[L4 ICMP] Type: %d, Code: %d\n", icmph->type, icmph->code);
        }
    }
    return 0;
}
