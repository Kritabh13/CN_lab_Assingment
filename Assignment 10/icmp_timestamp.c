#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>      // struct iphdr
#include <netinet/ip_icmp.h> // struct icmphdr
#include <sys/socket.h>
#include <time.h>

unsigned short csum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    unsigned short oddbyte;
    unsigned short answer;

    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char *) &oddbyte) = *(unsigned char *) ptr;
        sum += oddbyte;
    }

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    answer = (unsigned short) ~sum;
    return answer;
}

int main() {
    int sockfd;
    char packet[4096];

    // Change these IPs as per your setup
    char source_ip[] = "10.0.0.1";
    char dest_ip[]   = "10.0.0.2";

    memset(packet, 0, sizeof(packet));

    struct iphdr   *iph  = (struct iphdr *) packet;
    struct icmphdr *icmph = (struct icmphdr *) (packet + sizeof(struct iphdr));
    // ICMP timestamp has 12 bytes data (3 x 32-bit timestamps)
    uint32_t *timestamps = (uint32_t *) (packet + sizeof(struct iphdr) + sizeof(struct icmphdr));

    // Create raw socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Fill in IP header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) + 12);
    iph->id = htons(54321);
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_ICMP;
    iph->check = 0;
    iph->saddr = inet_addr(source_ip);
    iph->daddr = inet_addr(dest_ip);

    iph->check = csum((unsigned short *) packet, iph->ihl * 4);

    // Fill ICMP Timestamp Request
    icmph->type = 13;  // ICMP Timestamp Request
    icmph->code = 0;
    icmph->checksum = 0;
    icmph->un.echo.id = htons(1);
    icmph->un.echo.sequence = htons(1);

    // Fill timestamps (simple example: all zeros or current time)
    // For assignment, any valid 32-bit values are fine.
    uint32_t originate = 0;
    uint32_t receive   = 0;
    uint32_t transmit  = 0;

    timestamps[0] = htonl(originate);
    timestamps[1] = htonl(receive);
    timestamps[2] = htonl(transmit);

    // ICMP checksum over header + timestamps
    int icmp_len = sizeof(struct icmphdr) + 12;
    icmph->checksum = csum((unsigned short *) icmph, icmp_len);

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = iph->daddr;

    if (sendto(sockfd, packet, sizeof(struct iphdr) + icmp_len,
               0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("sendto() error");
        close(sockfd);
        exit(EXIT_FAILURE);
    } else {
        printf("ICMP Timestamp Request (type 13) sent to %s\n", dest_ip);
    }

    close(sockfd);
    return 0;
}

