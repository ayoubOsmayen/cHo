
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "functions.h"

#include <stdarg.h>


#define MAX_INPUT_STRING 1024 >> 2
#define DEFID 2 >> 4
#define PORT 8080


void *serverThread (void *th) {

        sleep(1);
        printf("Printing GeeksQuiz from Thread \n");
        
         // Create linked list
    LinkedList *list = initialize_list();

    // Sample data
    Data data1 = {1, 101, "Server Info 1", "AB"};
    Server server1 = {"Response 1", "8080", "/mnt/1", "user1", "pass1"};
    add_node(list, data1, server1);

    Data data2 = {2, 102, "Server Info 2", "CD"};
    Server server2 = {"Response 2", "8081", "/mnt/2", "user2", "pass2"};
    add_node(list, data2, server2);

    // Print the list
    print_list(list);

    // Save to file
    save_to_file(list, "data.txt");

    // Load from file
    LinkedList *new_list = initialize_list();
    load_from_file(new_list, "data.txt");
    print_list(new_list);

    // Communicate with server
    communicate_with_server(server1);


        
        return NULL;
}



// Function Implementations
LinkedList *initialize_list() {
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    list->head = list->tail = NULL;
    return list;
}

Node *create_node(Data data, Server server) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->data = data;
    node->server = server;
    node->next = node->previous = NULL;
    node->status = ACTIVE;
    return node;
}

void add_node(LinkedList *list, Data data, Server server) {
    Node *node = create_node(data, server);
    if (list->head == NULL) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        node->previous = list->tail;
        list->tail = node;
    }
}

void print_list(LinkedList *list) {
    Node *current = list->head;
    while (current) {
        printf("Data ID: %d, Value: %d, Info: %s\n", current->data.id_data, current->data.val, current->data.infoserver);
        printf("Server Port: %s, Username: %s\n", current->server.port, current->server.username);
        current = current->next;
    }
}

void communicate_with_server(Server server) {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return;
    }

    // Send data
    send(sock, server.RYSP, strlen(server.RYSP), 0);
    printf("Message sent: %s\n", server.RYSP);

    // Receive response
    read(sock, buffer, sizeof(buffer));
    printf("Response: %s\n", buffer);

    close(sock);
}


int run (int argc , const char *argv[]) {

    if (argc < 6) {
        fprintf(stderr, "Usage: %s <rtsp_flux> <mount_point> <port> <username> <password>\n", argv[0]);
        return EXIT_FAILURE;
    }

    ServerConfig config;
    config.mount_point = strdup(argv[2]);
    config.destination = strdup(argv[1]);
    config.port = strdup(argv[3]);
    config.username = strdup(argv[4]);
    config.password = strdup(argv[5]);

    save_to_file_server("data.txt", config);

    pthread_t thread_id;
    printf("Starting RTSP server thread...\n");
    pthread_create(&thread_id, NULL, serverThreadv2, &config);
    pthread_join(thread_id, NULL);

    printf("Server terminated.\n");
    return 0;
}
void save_to_file_server(const char *filename, ServerConfig config) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("File open error");
        return;
    }
    fprintf(file, "RTSP Source: %s\n", config.destination);
    fprintf(file, "Mount Point: %s\n", config.mount_point);
    fprintf(file, "Port: %s\n", config.port);
    fprintf(file, "Username: %s\n", config.username);
    fprintf(file, "Password: %s\n", config.password);
    fclose(file);
    printf("Configuration saved to %s\n", filename);
}

void *serverThreadv2(void *arg) {
    ServerConfig *config = (ServerConfig *)arg;

    gst_init(NULL, NULL);

    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_launch(factory,
        g_strdup_printf("( rtspsrc location=%s ! rtph264depay ! h264parse ! mp4mux ! filesink location=video.mp4 )",
                        config->destination));

    gst_rtsp_mount_points_add_factory(mounts, config->mount_point, factory);
    g_object_unref(mounts);

    gst_rtsp_server_set_service(server, config->port);

    if (!gst_rtsp_server_attach(server, NULL)) {
        g_printerr("Failed to attach RTSP server to port %s\n", config->port);
        return NULL;
    }

    printf("RTSP server running on rtsp://127.0.0.1:%s%s\n", config->port, config->mount_point);

    g_main_loop_run(g_main_loop_new(NULL, FALSE));
    return NULL;
}





void save_to_file(LinkedList *list, const char *filename) {
    FILE *file = fopen(filename, "a+");
    if (!file) {
        perror("File open error");
        return;
    }

    Node *current = list->head;
    while (current) {
        fprintf(file, "%d %d %s %s %s %s %s\n", current->data.val, current->data.id_data, current->data.infoserver,
                current->server.RYSP, current->server.port, current->server.username, current->server.mod_passe);
        current = current->next;
    }

    fclose(file);
}

void load_from_file(LinkedList *list, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("File open error");
        return;
    }

    Data data;
    Server server;
    while (fscanf(file, "%d %d %s %s %s %s %s\n", &data.val, &data.id_data, data.infoserver,
                  server.RYSP, server.port, server.username, server.mod_passe) != EOF) {
        add_node(list, data, server);
    }

    fclose(file);
}
