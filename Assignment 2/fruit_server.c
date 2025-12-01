#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

struct Fruit {
    char name[20];
    int quantity;
    char last_sold[50];
};

struct Fruit fruits[3] = {
    {"apple", 50, "N/A"},
    {"banana", 40, "N/A"},
    {"orange", 30, "N/A"}
};

char customers[100][50];
int customer_count = 0;

pthread_mutex_t lock;

int is_unique_customer(char *ip_port) {
    for (int i = 0; i < customer_count; i++) {
        if (strcmp(customers[i], ip_port) == 0) return 0;
    }
    return 1;
}

void add_customer(char *ip_port) {
    if (is_unique_customer(ip_port)) {
        strcpy(customers[customer_count], ip_port);
        customer_count++;
    }
}

void get_timestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 50, "%Y-%m-%d %H:%M:%S", t);
}

void *client_handler(void *socket_desc) {
    int client_sock = *(int *)socket_desc;
    free(socket_desc);

    char client_ip[INET_ADDRSTRLEN];
    int client_port;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getpeername(client_sock, (struct sockaddr *)&addr, &len);

    inet_ntop(AF_INET, &addr.sin_addr, client_ip, sizeof(client_ip));
    client_port = ntohs(addr.sin_port);

    char ip_port[50];
    sprintf(ip_port, "%s:%d", client_ip, client_port);

    add_customer(ip_port);

    char fruit[20];
    int qty;

    // Receive fruit name
    recv(client_sock, fruit, sizeof(fruit), 0);
    // Receive quantity
    recv(client_sock, &qty, sizeof(qty), 0);

    char response[300];

    pthread_mutex_lock(&lock);

    int found = 0;
    for (int i = 0; i < 3; i++) {
        if (strcmp(fruit, fruits[i].name) == 0) {
            found = 1;
            if (qty <= fruits[i].quantity) {
                fruits[i].quantity -= qty;
                get_timestamp(fruits[i].last_sold);

                sprintf(response,
                        "SUCCESS: %d %s sold.\nRemaining stock: %d\n"
                        "Last sold time updated.\n"
                        "Unique customers: %d\n",
                        qty, fruits[i].name, fruits[i].quantity,
                        customer_count);
            } else {
                sprintf(response,
                        "REGRET: Not enough %s available.\n"
                        "Stock: %d\nUnique customers: %d\n",
                        fruits[i].name, fruits[i].quantity,
                        customer_count);
            }
            break;
        }
    }

    if (!found) {
        sprintf(response, "ERROR: Fruit not found.\n");
    }

    pthread_mutex_unlock(&lock);

    send(client_sock, response, strlen(response), 0);

    close(client_sock);
    pthread_exit(NULL);
}

int main() {
    int server_sock, client_sock, *new_sock;
    struct sockaddr_in server, client;

    pthread_mutex_init(&lock, NULL);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        printf("Could not create socket\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9000);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind failed");
        return 1;
    }

    listen(server_sock, 5);

    printf("Fruit Server Running on Port 9000...\n");

    while (1) {
        socklen_t c = sizeof(struct sockaddr_in);
        client_sock = accept(server_sock, (struct sockaddr *)&client, &c);

        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread;
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        pthread_create(&thread, NULL, client_handler, (void *)new_sock);
        pthread_detach(thread);
    }

    close(server_sock);
    return 0;
}

