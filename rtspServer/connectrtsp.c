#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

extern char *current_destination_port;

static MYSQL *get_db_connection() {
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "password", "database", 0, NULL, 0)) {
        fprintf(stderr, "DB error: %s\n", mysql_error(conn));
        return NULL;
    }
    return conn;
}

int INSERT_CLIENT(ClientConnection *client) {
    MYSQL *conn = get_db_connection();
    if (!conn) return -1;

    char query[1024];
    snprintf(query, sizeof(query), "INSERT INTO rstp_clients (ip_address, session_id, connection_start, bandwidth_used) VALUES ('%s', '%s', '%s', '%s')",
             client->ip_address, client->session_id, client->connection_start, client->bandwidth_used);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "DB error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    mysql_close(conn);
    return 0;
}

int UPDATE_CLIENT(ClientConnection *client) {
    MYSQL *conn = get_db_connection();
    if (!conn) return -1;

    char query[1024];
    snprintf(query, sizeof(query), "UPDATE rstp_clients SET connection_end='%s', bandwidth_used='%s' WHERE ip_address='%s' AND session_id='%s'",
             client->connection_end, client->bandwidth_used, client->ip_address, client->session_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "DB error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    mysql_close(conn);
    return 0;
}

int DELETE_CLIENT(const char *ip_address) {
    MYSQL *conn = get_db_connection();
    if (!conn) return -1;

    char query[1024];
    snprintf(query, sizeof(query), "DELETE FROM rstp_clients WHERE ip_address='%s'", ip_address);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "DB error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    mysql_close(conn);
    return 0;
}

int GET_CLIENT(const char *ip_address, ClientConnection *client) {
    MYSQL *conn = get_db_connection();
    if (!conn) return -1;

    char query[1024];
    snprintf(query, sizeof(query), "SELECT ip_address, session_id, connection_start, connection_end, bandwidth_used FROM rstp_clients WHERE ip_address='%s'", ip_address);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "DB error: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res) {
        fprintf(stderr, "Failed to fetch result: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        client->ip_address = strdup(row[0]);
        client->session_id = strdup(row[1]);
        client->connection_start = strdup(row[2]);
        client->connection_end = strdup(row[3]);
        client->bandwidth_used = strdup(row[4]);
    }

    mysql_free_result(res);
    mysql_close(conn);
    return 0;
}

int create_connection(ClientConnection *client, const char *ip_address, const char *session_id) {
    client->ip_address = strdup(ip_address);
    client->session_id = strdup(session_id);
    client->connection_start = strdup("CURRENT_TIMESTAMP");
    client->connection_end = strdup("");
    client->bandwidth_used = strdup("0");
    return 0;
}

void free_client_connection(ClientConnection *client) {
    if (client) {
        free(client->ip_address);
        free(client->session_id);
        free(client->connection_start);
        free(client->connection_end);
        free(client->bandwidth_used);
        free(client);
    }
}


   
