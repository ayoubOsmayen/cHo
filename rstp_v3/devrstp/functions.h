#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <mysql/mysql.h>

typedef struct ClientConnection {
    char *ip_address;
    char *session_id;
    char *connection_start;
    char *connection_end;
    char *bandwidth_used;
} ClientConnection;

int INSERT_CLIENT(ClientConnection *client);
int UPDATE_CLIENT(ClientConnection *client);
int DELETE_CLIENT(const char *ip_address);
int GET_CLIENT(const char *ip_address, ClientConnection *client);
int create_connection(ClientConnection *client, const char *ip_address, const char *session_id);
void free_client_connection(ClientConnection *client);

#endif // FUNCTIONS_H
