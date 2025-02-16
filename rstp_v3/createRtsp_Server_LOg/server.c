/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include "config.h"
#include "function.h"

#define PORT 8082

typedef struct {
    int socket;
    Config *config;
} ClientArgs;

void *client_handler(void *arg) {
    ClientArgs *client = (ClientArgs *)arg;
    int client_socket = client->socket;
    Config *config = client->config;

    char buffer[1024] = {0};
    read(client_socket, buffer, sizeof(buffer));
    printf("Client sent: %s\n", buffer);

    save_to_mysql("192.168.1.1", 1, 10);

    send(client_socket, "RTSP Pipeline Started", strlen("RTSP Pipeline Started"), 0);

    close(client_socket);
    free(client);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Config config;
    read_configv1(argv[1], &config);

    init_gstreamer(&config);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t thread_id;
        ClientArgs *args = malloc(sizeof(ClientArgs));
        args->socket = client_socket;
        args->config = &config;

        pthread_create(&thread_id, NULL, client_handler, args);
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
