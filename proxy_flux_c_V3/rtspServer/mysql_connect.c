#include <stdio.h>
#include <mariadb/mysql.h>  
#include <stdlib.h>
// Global variables for database connection details
char *server = "62.210.181.123";
char *user = "u_flux_RSTP";
char *password = "#flux_RSTP#!!";
char *database = "EAGLE_Master";

// Function to print query results
void print_query_results(MYSQL *conn, const char *query) {
    // Execute the query
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);  // Store query result
    if (result == NULL) {
        fprintf(stderr, "Result retrieval failed: %s\n", mysql_error(conn));
        return;
    }

    int num_fields = mysql_num_fields(result);  // Get number of columns
    MYSQL_ROW row;

    // Print results row by row
    while ((row = mysql_fetch_row(result))) {
        for (int i = 0; i < num_fields; i++) {
            printf("%s\t", row[i] ? row[i] : "NULL");  // Print each field in the row
        }
        printf("\n");
    }

    mysql_free_result(result);  // Free result set
}

int main(int argc , const char *argv[]) {
    MYSQL *conn;
    conn = mysql_init(NULL);  // Initialize connection handler

    // Check if initialization was successful
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

    // Show tables in the database
    printf("\nShowing Tables:\n");
    print_query_results(conn, "SHOW TABLES");

    // Describe a specific table
    const char *table_name = argv[1];  
    char query[256];
    snprintf(query, sizeof(query), "DESCRIBE %s", table_name);
    printf("\nDescribing Table %s:\n", table_name);
    print_query_results(conn, query);

    // Select all rows from a table
    snprintf(query, sizeof(query), "SELECT * FROM %s", table_name);
    printf("\nSelecting * from Table %s:\n", table_name);
    print_query_results(conn, query);

    // Close the connection
    mysql_close(conn);

    return EXIT_SUCCESS;
}
