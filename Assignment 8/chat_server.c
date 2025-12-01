#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 2048

// Global variables to keep track of clients
int clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Helper function to get current timestamp
void get_timestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", t);
}

// Function to send message to all OTHER clients (Broadcast)
void send_to_all(char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    
    // 1. Write to Log File
    FILE *fp = fopen("log.txt", "a");
    char time_str[20];
    get_timestamp(time_str);
    fprintf(fp, "[%s] %s", time_str, message);
    fclose(fp);

    // 2. Send to other clients
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] != sender_socket) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("Sending failure");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Thread function to handle each client
void *handle_client(void *arg) {
    int sock = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        send_to_all(buffer, sock); // Broadcast message
    }

    // If client disconnects
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] == sock) {
            // Remove client from array (simple shift)
            for (int j = i; j < client_count - 1; ++j) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    close(sock);
    free(arg);
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8888);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Chat Server Started on Port 8888...\n");
    printf("Logs are being saved to log.txt\n");

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

        // Add new client to the array
        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = new_socket;
            printf("New Client Connected. Total: %d\n", client_count);
            
            // Create a thread for this client
            pthread_t tid;
            int *new_sock_ptr = malloc(sizeof(int));
            *new_sock_ptr = new_socket;
            pthread_create(&tid, NULL, handle_client, (void *)new_sock_ptr);
        } else {
            printf("Max clients reached. Connection rejected.\n");
            close(new_socket);
        }
        pthread_mutex_unlock(&clients_mutex);
    }
    return 0;
}
