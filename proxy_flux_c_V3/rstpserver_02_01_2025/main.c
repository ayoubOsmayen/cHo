#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <gst/gst.h>

#define CONFIG_FILE "config.txt"

typedef struct {
    char video_url[256];
    char rtsp_mount[256];
    int rtsp_port;
    char display_text[256];
} Config;

Config read_config(const char *config_file) {
    Config config;
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        perror("Error opening configuration file");
        exit(1);
    }

    fgets(config.video_url, sizeof(config.video_url), file);
    config.video_url[strcspn(config.video_url, "\n")] = 0;  // Remove newline

    fgets(config.rtsp_mount, sizeof(config.rtsp_mount), file);
    config.rtsp_mount[strcspn(config.rtsp_mount, "\n")] = 0;

    fscanf(file, "%d\n", &config.rtsp_port);

    fgets(config.display_text, sizeof(config.display_text), file);
    config.display_text[strcspn(config.display_text, "\n")] = 0;

    fclose(file);
    return config;
}

void watch_config_file(const char *config_file) {
    int fd, wd;
    char buffer[1024];

    fd = inotify_init();
    if (fd == -1) {
        perror("inotify_init");
        exit(1);
    }

    wd = inotify_add_watch(fd, config_file, IN_MODIFY);
    if (wd == -1) {
        perror("inotify_add_watch");
        exit(1);
    }

    while (1) {
        int length = read(fd, buffer, sizeof(buffer));
        if (length < 0) {
            perror("read");
        }

        for (int i = 0; i < length; i += sizeof(struct inotify_event) + ((struct inotify_event*)&buffer[i])->len) {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            if (event->mask & IN_MODIFY) {
                printf("Config file modified\n");
                // Reload configuration after change
            }
        }
    }

    close(fd);
}

void log_client_info(const char *ip_address, const char *session_id, const char *bandwidth) {
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return;
    }

    if (mysql_real_connect(conn, "localhost", "root", "password", "rtsp_db", 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed\n");
        mysql_close(conn);
        return;
    }

    char query[512];
    snprintf(query, sizeof(query), 
             "INSERT INTO rtsp_clients (ip_address, session_id, connection_start, bandwidth_used) VALUES ('%s', '%s', NOW(), '%s')",
             ip_address, session_id, bandwidth);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "INSERT INTO rtsp_clients failed. Error: %s\n", mysql_error(conn));
    }

    mysql_close(conn);
}

GstElement *pipeline, *video_source, *text_overlay, *rtsp_sink;

void create_pipeline(const char *video_url, const char *display_text) {
    gst_init(NULL, NULL);

    pipeline = gst_pipeline_new("video-pipeline");

    video_source = gst_element_factory_make("uridecodebin", "video_source");
    g_object_set(video_source, "uri", video_url, NULL);

    text_overlay = gst_element_factory_make("textoverlay", "text_overlay");
    g_object_set(text_overlay, "text", display_text, NULL);
    g_object_set(text_overlay, "valign", 2, NULL);  // Vertically center

    rtsp_sink = gst_element_factory_make("rtspclientsink", "rtsp_sink");
    g_object_set(rtsp_sink, "location", "rtsp://localhost:5267", NULL);

    gst_bin_add_many(GST_BIN(pipeline), video_source, text_overlay, rtsp_sink, NULL);
    gst_element_link_many(video_source, text_overlay, rtsp_sink, NULL);
}

void start_pipeline() {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void stop_pipeline() {
    gst_element_set_state(pipeline, GST_STATE_NULL);
}

int main(int argc, char *argv[]) {
    // Read configuration file
    Config config = read_config(CONFIG_FILE);
    printf("Video URL: %s\n", config.video_url);
    printf("RTSP Mount: %s\n", config.rtsp_mount);
    printf("RTSP Port: %d\n", config.rtsp_port);
    printf("Display Text: %s\n", config.display_text);

    // Set up GStreamer pipeline
    create_pipeline(config.video_url, config.display_text);
    start_pipeline();

    // Watch for changes in the configuration file
    watch_config_file(CONFIG_FILE);

    // Simulate logging RTSP client info
    log_client_info("192.168.1.10", "12345", "4Mbps");

    // Cleanup and stop GStreamer pipeline
    stop_pipeline();

    return 0;
}
