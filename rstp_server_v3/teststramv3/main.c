#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gst/gst.h>
#include <mariadb/mysql.h>
#include <sys/inotify.h>
#include <unistd.h>

#define CONFIG_FILE "config.txt"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

// Configuration structure
typedef struct {
    char video_url[256];
    char rtsp_mount[256];
    int rtsp_port;
    char overlay_text[256];
} Config;

char *server = "62.210.181.123";
char *user = "u_flux_RSTP";
char *password = "#flux_RSTP#!!";
char *database = "EAGLE_Master";





// Function to read configuration from file
void read_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%255s\n%255s\n%d\n%255[^\n]", 
           config->video_url, 
           config->rtsp_mount, 
           &config->rtsp_port, 
           config->overlay_text);
    fclose(file);
    printf("Config loaded: Video URL=%s, Mount=%s, Port=%d, Text=%s\n",
           config->video_url, config->rtsp_mount, config->rtsp_port, config->overlay_text);
}

// Function to initialize GStreamer pipeline
void init_gstreamer(Config *config) {
    gst_init(NULL, NULL);
    char pipeline_str[1024];
    snprintf(pipeline_str, sizeof(pipeline_str),
             "rtspsrc location=%s ! decodebin ! textoverlay text=\"%s\" "
             "! autovideosink",
             config->video_url, config->overlay_text);
    GstElement *pipeline = gst_parse_launch(pipeline_str, NULL);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    printf("GStreamer pipeline started.\n");
}

// Function to monitor configuration file changes
void monitor_config_file(const char *filename, Config *config) {
    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    int wd = inotify_add_watch(fd, filename, IN_MODIFY);
    if (wd == -1) {
        perror("inotify_add_watch");
        close(fd);
        exit(EXIT_FAILURE);
    }

    char buffer[EVENT_BUF_LEN];
    while (1) {
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            perror("read");
        }

        for (int i = 0; i < length; i += EVENT_SIZE + ((struct inotify_event *)&buffer[i])->len) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->mask & IN_MODIFY) {
                printf("Config file modified. Reloading...\n");
                read_config(filename, config);
                init_gstreamer(config);  // Restart pipeline with new config
            }
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}

// Function to log client data
void log_client_data(const char *ip, int session_id, int bandwidth) {
    FILE *log_file = fopen("clients.log", "a");
    if (!log_file) {
        perror("Failed to open log file");
        return;
    }
    fprintf(log_file, "[%ld] Client Connected: IP=%s, SessionID=%d, Bandwidth=%d Mbps\n", 
            time(NULL), ip, session_id, bandwidth);
    fclose(log_file);
}

// Function to insert data into MySQL
void save_to_mysql(const char *ip, int session_id, int bandwidth) {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return;
    }
    if (mysql_real_connect(conn, server, user, password, database, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        return;
    }

    char query[256];
    snprintf(query, sizeof(query), 
             "INSERT INTO rtsp_clients (ip, session_id, bandwidth) VALUES ('%s', %d, %d)", 
             ip, session_id, bandwidth);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "INSERT failed. Error: %s\n", mysql_error(conn));
    }

    mysql_close(conn);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Config config;
    read_config(argv[1], &config);

    // Start GStreamer pipeline
    init_gstreamer(&config);

    // Start monitoring config file for changes
    monitor_config_file(argv[1], &config);

    return EXIT_SUCCESS;
}
