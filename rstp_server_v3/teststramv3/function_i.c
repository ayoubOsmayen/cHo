/* function.c */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gst/gst.h>
#include <mariadb/mysql.h>
#include <string.h>
#include "function.h"

char *server = "62.210.181.123";
char *user = "u_flux_RSTP";
char *password = "#flux_RSTP#!!";
char *database = "EAGLE_Master";






void init_gstreamer(Config *config) {
    gst_init(NULL, NULL);
    char pipeline_str[1024];
    snprintf(pipeline_str, sizeof(pipeline_str),
             "rtspsrc location=%s ! decodebin ! textoverlay text=\"%s\" ! autovideosink",
             config->video_url, config->overlay_text);
    GstElement *pipeline = gst_parse_launch(pipeline_str, NULL);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void save_to_mysql(const char *ip, int session_id, int bandwidth) {
    MYSQL *conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "MySQL initialization failed\n");
        return;
    }
    
    

    if (!mysql_real_connect(conn, server , user, password, "database", 0, NULL, 0)) {
        fprintf(stderr, "MySQL connection failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO rtsp_clients (ip_address, session_id, connection_start, bandwidth_used) "
             "VALUES ('%s', %d, NOW(), '%d Mbps')",
             ip, session_id, bandwidth);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
    }

    mysql_close(conn);
}
