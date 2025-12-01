#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

char name[32];

// Thread to receive messages from server
void *recv_msg_handler(void *socket_desc) {
    int sock = *(int *)socket_desc;
    char buffer[2048];
    int len;

    while ((len = recv(sock, buffer, 2048, 0)) > 0) {
        buffer[len] = '\0';
        printf("%s", buffer); // Print incoming message
        fflush(stdout); // Force print
    }
    return NULL;
}

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[2048];
    char message[2048];
    pthread_t recv_thread;

    printf("Enter your name: ");
    fgets(name, 32, stdin);
    name[strcspn(name, "\n")] = 0; // Remove newline

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8888);
    inet_pton(AF_INET, "10.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    printf("Connected to Chat Server!\n");

    // Start thread to listen for incoming messages
    if (pthread_create(&recv_thread, NULL, recv_msg_handler, (void *)&sock) < 0) {
        perror("Thread creation failed");
        return 1;
    }

    // Main loop: Listen for user input and send
    while (1) {
        fgets(buffer, 2048, stdin);
        sprintf(message, "%s: %s", name, buffer);
        send(sock, message, strlen(message), 0);
    }

    close(sock);
    return 0;
}
