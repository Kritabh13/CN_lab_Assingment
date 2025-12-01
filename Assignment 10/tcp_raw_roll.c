#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>    // struct iphdr
#include <netinet/tcp.h>   // struct tcphdr
#include <sys/socket.h>

// Pseudo header for TCP checksum
struct pseudo_header {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t tcp_length;
};

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
    char datagram[4096];

    // Change these according to your setup
    char source_ip[] = "10.0.0.1";  // sender IP
    char dest_ip[]   = "10.0.0.2";  // receiver IP (target)

    // Payload = YOUR ROLL NUMBER
    const char *payload = "ROLLNO: CSM24035";

    // Zero out the buffer
    memset(datagram, 0, sizeof(datagram));

    // IP header and TCP header pointers
    struct iphdr  *iph  = (struct iphdr *) datagram;
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct iphdr));
    char *data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);

    // Copy payload
    strcpy(data, payload);
    int payload_len = strlen(payload);

    // Create raw socket
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0) {
        perror("socket() error");
        exit(EXIT_FAILURE);
    }

    // Tell kernel that headers are included in the packet
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt() error");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Fill in IP header
    iph->ihl = 5;  // header length
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr) + payload_len);
    iph->id = htons(12345); // identification
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0; // set to 0 before calculating
    iph->saddr = inet_addr(source_ip);
    iph->daddr = inet_addr(dest_ip);

    // IP checksum
    iph->check = csum((unsigned short *) datagram, iph->ihl * 4);

    // Fill in TCP header
    tcph->source = htons(1234); // any source port
    tcph->dest = htons(80);     // destination port (can be anything, e.g. 8080)
    tcph->seq = htonl(1);
    tcph->ack_seq = 0;
    tcph->doff = 5; // tcp header size
    tcph->fin = 0;
    tcph->syn = 1;  // SYN packet (for example)
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(5840); // max allowed window size
    tcph->check = 0;            // will calculate
    tcph->urg_ptr = 0;

    // Pseudo header for checksum
    struct pseudo_header psh;
    char pseudo_packet[4096];

    psh.src_addr = inet_addr(source_ip);
    psh.dst_addr = inet_addr(dest_ip);
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(sizeof(struct tcphdr) + payload_len);

    memset(pseudo_packet, 0, sizeof(pseudo_packet));
    memcpy(pseudo_packet, &psh, sizeof(psh));
    memcpy(pseudo_packet + sizeof(psh), tcph, sizeof(struct tcphdr) + payload_len);

    tcph->check = csum((unsigned short *) pseudo_packet,
                       sizeof(struct pseudo_header) + sizeof(struct tcphdr) + payload_len);

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = tcph->dest;
    dest.sin_addr.s_addr = iph->daddr;

    // Send packet
    if (sendto(sockfd, datagram,
               sizeof(struct iphdr) + sizeof(struct tcphdr) + payload_len,
               0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("sendto() error");
        close(sockfd);
        exit(EXIT_FAILURE);
    } else {
        printf("TCP raw packet sent to %s with payload: %s\n", dest_ip, payload);
    }

    close(sockfd);
    return 0;
}

