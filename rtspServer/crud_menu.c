#include <stdio.h>
#include <stdlib.h>
#include <mariadb/mysql.h>
#include <time.h>

// Global variables for database connection details
char *server = "62.210.181.123";
char *user = "u_flux_RSTP";
char *password = "#flux_RSTP#!!";
char *database = "EAGLE_Master";

// Function to get current date and time as a string
void get_current_datetime(char *datetime_str) {
    time_t t = time(NULL); // Get current time
    struct tm tm_info;
    localtime_r(&t, &tm_info);

    // Format date and time as YYYY-MM-DD HH:MM:SS
    strftime(datetime_str, 20, "%Y-%m-%d %H:%M:%S", &tm_info);
}

// Function to get end time (current time + 2 hours)
void get_end_time(char *end_time_str) {
    time_t t = time(NULL); // Get current time
    struct tm tm_info;
    localtime_r(&t, &tm_info);

    // Add 2 hours to current time
    tm_info.tm_hour += 2;

    // Normalize the time structure
    mktime(&tm_info);

    // Format end time as YYYY-MM-DD HH:MM:SS
    strftime(end_time_str, 20, "%Y-%m-%d %H:%M:%S", &tm_info);
}

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
             "INSERT INTO rtsp_clients (ip_address, session_id, connection_start, connection_end, bandwidth_used) "
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
    snprintf(query, sizeof(query), "SELECT * FROM rtsp_clients WHERE id = %d", id);

    printf("\nRecord with ID %d:\n", id);
    print_query_results(conn, query);
}

// Function to update a record by id
void update_record(MYSQL *conn, int id, const char *ip_address, const char *session_id, const char *connection_start, const char *connection_end, const char *bandwidth_used) {
    char query[1024];
    snprintf(query, sizeof(query),
             "UPDATE rtsp_clients SET ip_address = '%s', session_id = '%s', connection_start = '%s', "
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
    snprintf(query, sizeof(query), "DELETE FROM rtsp_clients WHERE id = %d", id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Delete record failed: %s\n", mysql_error(conn));
    } else {
        printf("Record with ID %d deleted successfully.\n", id);
    }
}

// Function to save query results to a file
void save_to_file(MYSQL *conn, const char *query, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Unable to open file for writing\n");
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "Result retrieval failed: %s\n", mysql_error(conn));
        fclose(file);
        return;
    }

    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result))) {
        for (int i = 0; i < num_fields; i++) {
            fprintf(file, "%s\t", row[i] ? row[i] : "NULL");
        }
        fprintf(file, "\n");
    }

    mysql_free_result(result);
    fclose(file);
    printf("Data saved to file %s\n", filename);
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

    int choice;
    int id;
    char ip_address[45], session_id[100], connection_start[20], connection_end[20], bandwidth_used[50];
    char filename[256];

    printf("Connected to MariaDB Server\n");

    while (1) {
        // Display menu
        printf("\nMenu:\n");
        printf("1. Insert a new record\n");
        printf("2. Read a record by ID\n");
        printf("3. Update a record by ID\n");
        printf("4. Delete a record by ID\n");
        printf("5. Search for a record by ID\n");
        printf("6. Save query results to a file\n");
        printf("7. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // Insert a new record
                get_current_datetime(connection_start);
                get_end_time(connection_end);
                printf("Enter IP address: ");
                scanf("%s", ip_address);
                printf("Enter session ID: ");
                scanf("%s", session_id);
                printf("Enter bandwidth used: ");
                scanf("%s", bandwidth_used);
                create_record(conn, ip_address, session_id, connection_start, connection_end, bandwidth_used);
                break;
            case 2:
                // Read a record by ID
                printf("Enter ID: ");
                scanf("%d", &id);
                read_record(conn, id);
                break;
            case 3:
                // Update a record by ID
                printf("Enter ID: ");
                scanf("%d", &id);
                printf("Enter new IP address: ");
                scanf("%s", ip_address);
                printf("Enter new session ID: ");
                scanf("%s", session_id);
                get_current_datetime(connection_start);
                get_end_time(connection_end);
                printf("Enter new bandwidth used: ");
                scanf("%s", bandwidth_used);
                update_record(conn, id, ip_address, session_id, connection_start, connection_end, bandwidth_used);
                break;
            case 4:
                // Delete a record by ID
                printf("Enter ID: ");
                scanf("%d", &id);
                delete_record(conn, id);
                break;
            case 5:
                // Search by ID
                printf("Enter ID: ");
                scanf("%d", &id);
                read_record(conn, id);
                break;
            case 6:
                // Save to file
                printf("Enter SQL query to fetch data: ");
                scanf("%s", filename); // For simplicity, assume a query fits in the filename buffer
                save_to_file(conn, filename, "output.txt");
                break;
            case 7:
                // Exit the program
                mysql_close(conn);
                return EXIT_SUCCESS;
            default:
                printf("Invalid choice, please try again.\n");
        }
    }
}
