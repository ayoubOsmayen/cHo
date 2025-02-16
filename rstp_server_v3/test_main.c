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

char *server = "62.210.181.123";
char *user = "u_flux_RSTP";
char *password = "#flux_RSTP#!!";
char *database = "EAGLE_Master";





// Global variables for GStreamer pipeline and configuration
GstElement *pipeline;
char current_video[256] = "";
char current_text[256] = "";
int current_port = 0;

pthread_mutex_t config_lock = PTHREAD_MUTEX_INITIALIZER;

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
        fprintf(stderr, "Failed to create GStreamer pipeline\n");
        pthread_mutex_unlock(&config_lock);
        return;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    pthread_mutex_unlock(&config_lock);
}

void read_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        perror("Failed to open config file");
        return;
    }

    char video[256], text[256];
    int port;

    fscanf(file, "%255s\n%255s\n%d", video, text, &port);
    fclose(file);

    if (strcmp(video, current_video) != 0 || strcmp(text, current_text) != 0 || port != current_port) {
        strcpy(current_video, video);
        strcpy(current_text, text);
        current_port = port;
        update_pipeline(video, text, port);
    }
}

void *monitor_config(void *arg) {
    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return NULL;
    }

    int wd = inotify_add_watch(fd, CONFIG_FILE, IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        return NULL;
    }

    char buffer[EVENT_BUF_LEN];
    while (1) {
        int length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            perror("read");
            break;
        }

        for (int i = 0; i < length;) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->mask & IN_MODIFY) {
                read_config();
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return NULL;
}

void log_client_activity(const char *ip, const char *session_id, const char *status) {
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, "rtsp_clients", 0, NULL, 0)) {
        fprintf(stderr, "MySQL connection error: %s\n", mysql_error(conn));
        return;
    }

    char query[512];
    snprintf(query, sizeof(query),
             "INSERT INTO rtsp_clients (ip_address, session_id, connection_start, connection_end, bandwidth_used) VALUES ('%s', '%s', NOW(), NULL, 'N/A')",
             ip, session_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "MySQL query error: %s\n", mysql_error(conn));
    }

    mysql_close(conn);
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    pthread_t monitor_thread;
    if (pthread_create(&monitor_thread, NULL, monitor_config, NULL) != 0) {
        perror("Failed to create config monitor thread");
        return EXIT_FAILURE;
    }

    read_config();

    printf("RTSP streaming server running.\n");
    pthread_join(monitor_thread, NULL);

    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }

    return EXIT_SUCCESS;
}
