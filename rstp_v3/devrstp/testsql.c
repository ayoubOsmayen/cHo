#include <stdio.h>
#include <mariadb/mysql.h>

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

int main() {
	    MYSQL *conn;
	        conn = mysql_init(NULL);

		    // Connect to MariaDB server
		    //     if (conn == NULL) {
		    //             fprintf(stderr, "mysql_init() failed\n");
		    //                     return EXIT_FAILURE;
		    //                         }
		    //
		    //                             // Replace with your own database credentials
		    //                                 const char *host = "localhost";
		    //                                     const char *user = "root";
		    //                                         const char *password = "yourpassword"; // Change this
		    //                                             const char *dbname = "yourdbname";     // Change this
		    //
		    //                                                 if (mysql_real_connect(conn, host, user, password, dbname, 0, NULL, 0) == NULL) {
		    //                                                         fprintf(stderr, "mysql_real_connect() failed\n");
		    //                                                                 mysql_close(conn);
		    //                                                                         return EXIT_FAILURE;
		    //                                                                             }
		    //
		    //                                                                                 printf("Connected to MariaDB Server\n");
		    //
		    //                                                                                     // Show tables
		    //                                                                                         printf("\nShowing Tables:\n");
		    //                                                                                             print_query_results(conn, "SHOW TABLES");
		    //
		    //                                                                                                 // Describe a specific table
		    //                                                                                                     const char *table_name = "your_table_name";  // Change this
		    //                                                                                                         char query[256];
		    //                                                                                                             snprintf(query, sizeof(query), "DESCRIBE %s", table_name);
		    //                                                                                                                 printf("\nDescribing Table %s:\n", table_name);
		    //                                                                                                                     print_query_results(conn, query);
		    //
		    //                                                                                                                         // Select * from a table
		    //                                                                                                                             snprintf(query, sizeof(query), "SELECT * FROM %s", table_name);
		    //                                                                                                                                 printf("\nSelecting * from Table %s:\n", table_name);
		    //                                                                                                                                     print_query_results(conn, query);
		    //
		    //                                                                                                                                         // Close the connection
		    //                                                                                                                                             mysql_close(conn);
		    //                                                                                                                                                 return EXIT_SUCCESS;
		    //                                                                                                                                                 }
		    //
