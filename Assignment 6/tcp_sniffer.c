#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#define BUFFER_SIZE 65536

int main() {
    int raw_socket;
    struct sockaddr saddr;
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);

    printf("TCP Packet Sniffer Started...\n");

    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (raw_socket < 0) {
        perror("Socket Error");
        return 1;
    }

    while (1) {
        socklen_t saddr_len = sizeof(saddr);
        int data_size = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0,
                                 &saddr, &saddr_len);

        if (data_size < 0) {
            perror("Recvfrom Error");
            return 1;
        }

        struct iphdr *ip_header =
            (struct iphdr*)(buffer + sizeof(struct ethhdr));

        if (ip_header->protocol == IPPROTO_TCP) {
            struct tcphdr *tcp_header =
                (struct tcphdr*)(buffer + sizeof(struct ethhdr) + ip_header->ihl * 4);

            printf("\n====== TCP Packet ======\n");
            printf("Source IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr));
            printf("Destination IP: %s\n", inet_ntoa(*(struct in_addr *)&ip_header->daddr));
            printf("Source Port: %d\n", ntohs(tcp_header->source));
            printf("Destination Port: %d\n", ntohs(tcp_header->dest));
            printf("SYN: %d  ACK: %d  FIN: %d\n", tcp_header->syn, tcp_header->ack, tcp_header->fin);
            printf("=========================\n");
        }
    }

    close(raw_socket);
    return 0;
}

