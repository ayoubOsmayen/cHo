#include <stdio.h>
#include <stdlib.h>
#include <mariadb/mysql.h>

// Global variables for database connection details
char *server = "62.210.181.123";
char *user = "u_flux_RSTP";
char *password = "#flux_RSTP#!!";
char *database = "EAGLE_Master";

// Function to print query results
void print_query_results(MYSQL *conn, const char *query) {
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "Result retrieval failed: %s\n", mysql_error(conn));
        return;
    }

    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result))) {
        for (int i = 0; i < num_fields; i++) {
            printf("%s\t", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }

    mysql_free_result(result);
}

// Function to add a new record
void create_record(MYSQL *conn, const char *ip_address, const char *session_id, const char *connection_start, const char *connection_end, const char *bandwidth_used) {
    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO your_table_name (ip_address, session_id, connection_start, connection_end, bandwidth_used) "
             "VALUES ('%s', '%s', '%s', '%s', '%s')",
             ip_address, session_id, connection_start, connection_end, bandwidth_used);
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Create record failed: %s\n", mysql_error(conn));
    } else {
        printf("Record created successfully.\n");
    }
}

// Function to read a record by id
void read_record(MYSQL *conn, int id) {
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM your_table_name WHERE id = %d", id);

    printf("\nRecord with ID %d:\n", id);
    print_query_results(conn, query);
}

// Function to update a record by id
void update_record(MYSQL *conn, int id, const char *ip_address, const char *session_id, const char *connection_start, const char *connection_end, const char *bandwidth_used) {
    char query[1024];
    snprintf(query, sizeof(query),
             "UPDATE your_table_name SET ip_address = '%s', session_id = '%s', connection_start = '%s', "
             "connection_end = '%s', bandwidth_used = '%s' WHERE id = %d",
             ip_address, session_id, connection_start, connection_end, bandwidth_used, id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Update record failed: %s\n", mysql_error(conn));
    } else {
        printf("Record updated successfully.\n");
    }
}

// Function to delete a record by id
void delete_record(MYSQL *conn, int id) {
    char query[256];
    snprintf(query, sizeof(query), "DELETE FROM your_table_name WHERE id = %d", id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Delete record failed: %s\n", mysql_error(conn));
    } else {
        printf("Record with ID %d deleted successfully.\n", id);
    }
}

int main() {
    MYSQL *conn;
    conn = mysql_init(NULL);

    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return EXIT_FAILURE;
    }

    // Connect to MariaDB
    if (mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    printf("Connected to MariaDB Server\n");

    // CRUD Operations

    // Create a new record
    create_record(conn, "192.168.1.1", "session_123", "2025-01-08 12:00:00", "2025-01-08 12:30:00", "1GB");

    // Read a record by ID
    int search_id = 1;  // Replace with the desired ID
    read_record(conn, search_id);

    // Update a record by ID
    update_record(conn, search_id, "192.168.1.2", "session_456", "2025-01-08 13:00:00", "2025-01-08 13:30:00", "2GB");

    // Delete a record by ID
    delete_record(conn, search_id);

    // Close the connection
    mysql_close(conn);

    return EXIT_SUCCESS;
}
