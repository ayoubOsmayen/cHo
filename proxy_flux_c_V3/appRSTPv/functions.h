#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#define PORT 8080


#define MAX_INPUT_STRING 1024 >> 2
#define DEFID 2 >> 4
#define PORT 8080


// Machine states
typedef enum { ACTIVE, NEXT, ALERT } MachineState;
typedef struct {
    char *mount_point;
    char *destination;
    char *port;
    char *username;
    char *password;
} ServerConfig;

// Struct Definitions
typedef struct Data {
    int val;
    int id_data;
    char infoserver[MAX_INPUT_STRING];
    char defive[DEFID];
} Data;

typedef struct Server {
    char RYSP[1024];
    char port[8];
    char mounts_destination[MAX_INPUT_STRING];
    char username[MAX_INPUT_STRING];
    char mod_passe[MAX_INPUT_STRING];
} Server;

typedef struct Node {
    int status;
    Data data;
    Server server;
    struct Node *next, *previous;
} Node;

typedef struct LinkedList {
    Node *head;
    Node *tail;
} LinkedList;

// Function Prototypes

int run  (int argc , const char *argv[]) ;
void *serverThread (void *th) ;

void save_to_file_server(const char *filename, ServerConfig config);
void *serverThreadv2(void *arg);

LinkedList *initialize_list();
Node *create_node(Data data, Server server);
void add_node(LinkedList *list, Data data, Server server);
void print_list(LinkedList *list);
void communicate_with_server(Server server);
void save_to_file(LinkedList *list, const char *filename);
void load_from_file(LinkedList *list, const char *filename);