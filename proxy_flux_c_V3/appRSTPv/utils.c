#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        printf("ID: %d, Value: %d, Info: %s\n", current->data.id_data, current->data.val, current->data.infoserver);
        printf("Server: Source=%s, Port=%s, Mount=%s, User=%s, Password=%s\n",
               current->server.RYSP, current->server.port, current->server.mount_point,
               current->server.username, current->server.password);
        current = current->next;
    }
}

void save_to_file(LinkedList *list, const char *filename) {
    FILE *file = fopen(filename, "a+");
    if (!file) {
        perror("File error");
        return;
    }
    Node *current = list->head;
    while (current) {
        fprintf(file, "%d %d %s %s %s %s %s\n", current->data.id_data, current->data.val,
                current->data.infoserver, current->server.RYSP, current->server.port,
                current->server.username, current->server.password);
        current = current->next;
    }
    fclose(file);
}
