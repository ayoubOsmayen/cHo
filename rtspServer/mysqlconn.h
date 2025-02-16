#include <mariadb/mysql.h> 
char *server = "62.210.181.123";
char *user = "u_flux_RSTP";
char *password = "#flux_RSTP#!!";
char *database = "EAGLE_Master";
typedef struct ClientConnection {
    char *ip_address;
    char *timeBuff;
} ClientConnection;

