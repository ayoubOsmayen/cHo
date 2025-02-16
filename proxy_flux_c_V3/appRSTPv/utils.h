#ifndef UTILS_H
#define UTILS_H

#include <gst/gst.h>

// Declare the functions and structures
typedef struct {
    char *RYSP;
    char *port;
    char *mount_point;
    char *username;
    char *password;
} Server;

typedef struct {
    int id_data;
    int val;
    char infoserver[256];
} Data;

typedef struct Node {
    Data data;
    Server server;
    struct Node *next;
    struct Node *previous;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} LinkedList;

LinkedList *initialize_list();
Node *create_node(Data data, Server server);
void add_node(LinkedList *list, Data data, Server server);
void print_list(LinkedList *list);
void save_to_file(LinkedList *list, const char *filename);
void load_from_file(LinkedList *list, const char *filename);

#endif
