#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mariadb/mysql.h>
#include <gst/gst.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <pthread.h>

#define CONFIG_FILE "config.txt"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

// Global variables for GStreamer pipeline and configuration
GstElement *pipeline;
char current_video[256] = "";
char current_text[256] = "";
int current_port = 0;

pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;

// Log message to file
void log_to_file(const char *message) {
    FILE *log_file = fopen("server.log", "a");
    if (!log_file) {
        perror("Failed to open log file");
        return;
    }
    fprintf(log_file, "%s\n", message);
    fclose(log_file);
}

// Update GStreamer pipeline
void update_pipeline(const char *video, const char *text, int port) {
    pthread_mutex_lock(&config_lock);

    // Stop the current pipeline if running
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = NULL;
    }

    // Build the GStreamer pipeline dynamically
    char pipeline_str[1024];
    snprintf(pipeline_str, sizeof(pipeline_str),
             "rtspsrc location=%s ! decodebin ! videoconvert ! textoverlay text=\"%s\" ! x264enc ! rtph264pay name=pay0 pt=96 ! udpsink host=127.0.0.1 port=%d",
             video, text, port);

    pipeline = gst_parse_launch(pipeline_str, NULL);
    if (!pipeline) {
        log_to_file("Error: Failed to create GStreamer pipeline.");
        pthread_mutex_unlock(&config_lock);
        return;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    log_to_file("Pipeline updated and set to PLAYING state.");
    pthread_mutex_unlock(&config_lock);
}

// Read configuration file
void read_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        log_to_file("Error: Failed to open config file.");
        return;
    }

    char video[256], text[256];
    int port;

    fscanf(file, "%255s\n%255s\n%d", video, text, &port);
    fclose(file);

    if (strcmp(video, current_video) != 0 || strcmp(text, current_text) != 0 || port != current_port) {
        char config_msg[512];
        snprintf(config_msg, sizeof(config_msg), "Configuration updated: video=%s, text=%s, port=%d", video, text, port);
        log_to_file(config_msg);

        strcpy(current_video, video);
        strcpy(current_text, text);
        current_port = port;
        update_pipeline(video, text, port);
    }
}

// Monitor configuration file for changes
void *monitor_config(void *arg) {
    int fd = inotify_init();
    if (fd < 0) {
        log_to_file("Error: inotify_init failed.");
        return NULL;
    }

    int wd = inotify_add_watch(fd, CONFIG_FILE, IN_MODIFY);
    if (wd < 0) {
        log_to_file("Error: inotify_add_watch failed.");
        close(fd);
        return NULL;
    }

    char buffer[EVENT_BUF_LEN];
    while (1) {
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            log_to_file("Error: read failed in inotify loop.");
            break;
        }

        for (int i = 0; i < length;) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->mask & IN_MODIFY) {
                log_to_file("Configuration file modified. Reloading...");
                read_config();
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return NULL;
}

// Log client activity to MySQL
void log_client_activity(const char *ip, const char *session_id) {
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "user", "password", "rtsp_db", 0, NULL, 0)) {
        log_to_file("Error: MySQL connection failed.");
        return;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO rtsp_clients (ip_address, session_id, connection_start, connection_end, bandwidth_used) VALUES ('%s', '%s', NOW(), NULL, 'N/A')",
             ip, session_id);

    if (mysql_query(conn, query)) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Error: MySQL query failed: %s", mysql_error(conn));
        log_to_file(error_msg);
    } else {
        log_to_file("Client activity logged successfully.");
    }

    mysql_close(conn);
}

// Main function
int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    pthread_t monitor_thread;
    if (pthread_create(&monitor_thread, NULL, monitor_config, NULL) != 0) {
        log_to_file("Error: Failed to create config monitor thread.");
        return EXIT_FAILURE;
    }

    log_to_file("RTSP streaming server started.");
    read_config();

    pthread_join(monitor_thread, NULL);

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }

    log_to_file("RTSP streaming server stopped.");
    return EXIT_SUCCESS;
}
