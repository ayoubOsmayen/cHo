#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/inotify.h>  // Include this for IN_MODIFY and inotify_event
#include <errno.h>        // For error handling

#define LOG_FILE "rstp_logservice.log"
#define CONFIG_FILE "conf.txt"
#define VIDEO_LIST_FILE "videolist.txt"

GMutex config_mutex;
char *current_video_source = NULL, *current_mount_point = NULL, *current_destination_port = NULL, *current_text = NULL;
GstElement *text_overlay_element = NULL;
GMainLoop *loop = NULL;
pthread_t config_thread;
GList *video_url_list = NULL; // Linked list to store video URLs
int current_video_index = 0;  // Index for the current video URL

// Function Prototypes
int read_configuration(const char *file, char **video_source, char **mount_point, char **destination_port, char **text);
void log_message(const char *message);

void log_message(const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Unable to open log file");
        return;
    }

    time_t current_time = time(NULL);
    char *timestamp = ctime(&current_time);
    timestamp[strlen(timestamp) - 1] = '\0';  // Remove newline from timestamp

    fprintf(log_file, "[%s] %s\n", timestamp, message);
    fclose(log_file);
}

int load_video_urls() {
    FILE *file = fopen(VIDEO_LIST_FILE, "r");
    if (file == NULL) {
        log_message("Error: Could not open videolist.txt.");
        return -1;
    }

    char url[1024];
    while (fgets(url, sizeof(url), file)) {
        // Remove newline character from the URL if present
        url[strcspn(url, "\n")] = '\0';
        video_url_list = g_list_append(video_url_list, g_strdup(url));
    }

    fclose(file);
    return 0;
}

void update_video_url(const char *new_url) {
    g_mutex_lock(&config_mutex);
    g_free(current_video_source);
    current_video_source = g_strdup(new_url);
    log_message("Video source updated.");
    g_mutex_unlock(&config_mutex);
}

void* rtsp_server_function(void* arg) {
    GstRTSPServer *server = (GstRTSPServer *)arg;
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    while (1) {
        gchar *launch_string = g_strdup_printf(
            "( uridecodebin uri=%s ! queue ! videoconvert ! textoverlay name=text_overlay text=\"%s\" font-desc=\"Sans 40px\" shaded-background=true halignment=center valignment=center ! x264enc tune=zerolatency ! rtph264pay name=pay0 pt=96 )",
            current_video_source, current_text);

        gst_rtsp_media_factory_set_launch(factory, launch_string);
        g_free(launch_string);

        gst_rtsp_media_factory_set_shared(factory, FALSE);
        gst_rtsp_mount_points_add_factory(mounts, current_mount_point, factory);
        g_object_unref(mounts);

        if (gst_rtsp_server_attach(server, NULL) == 0) {
            g_printerr("Failed to attach the RTSP server.\n");
            return NULL;
        }

        log_message("RTSP Server started.");

        // Cycle through videos
        if (video_url_list != NULL) {
            while (1) {
                // Update the video source from the list
                update_video_url((char *)video_url_list->data);
                video_url_list = g_list_next(video_url_list);
                if (!video_url_list) {
                    video_url_list = g_list_first(video_url_list);  // Loop the list
                }

                // Wait 30 seconds before switching to the next video
                sleep(30);
            }
        }

        g_main_loop_run(loop);
    }

    return NULL;
}

void* file_monitor_function(void* arg) {
    int fd, wd;
    char buffer[1024];

    // Create inotify instance
    fd = inotify_init();
    if (fd == -1) {
        perror("inotify_init");
        return NULL;
    }

    // Watch for changes to the configuration file
    wd = inotify_add_watch(fd, CONFIG_FILE, IN_MODIFY);  // IN_MODIFY will now be recognized
    if (wd == -1) {
        perror("inotify_add_watch");
        close(fd);
        return NULL;
    }

    while (1) {
        ssize_t length = read(fd, buffer, sizeof(buffer));
        if (length == -1) {
            perror("read");
            break;
        }

        for (int i = 0; i < length; i += sizeof(struct inotify_event) + ((struct inotify_event*)&buffer[i])->len) {
            struct inotify_event* event = (struct inotify_event*)&buffer[i];
            if (event->mask & IN_MODIFY) {
                log_message("conf.txt has been modified.");
                break;
            }
        }
    }

    // Clean up inotify
    close(fd);
    return NULL;
}

int read_configuration(const char *file, char **video_source, char **mount_point, char **destination_port, char **text) {
    FILE *fp = fopen(file, "r");
    if (!fp) {
        g_printerr("Failed to open the configuration file.\n");
        return -1;
    }

    if (fscanf(fp, "%ms\n%ms\n%ms\n%ms", video_source, mount_point, destination_port, text) != 4) {
        g_printerr("Error reading the configuration file.\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        g_printerr("Usage: %s <config_file>\n", argv[0]);
        return -1;
    }

    const char *config_file = argv[1];
    gst_init(&argc, &argv);

    if (access(config_file, F_OK) == -1) {
        g_printerr("Configuration file '%s' does not exist.\n", config_file);
        return -1;
    }

    if (load_video_urls() != 0) {
        return -1;
    }

    // Read initial configuration
    if (read_configuration(config_file, &current_video_source, &current_mount_point, &current_destination_port, &current_text) != 0) {
        return -1;
    }

    // Initialize server
    loop = g_main_loop_new(NULL, FALSE);
    GstRTSPServer *server = gst_rtsp_server_new();
    g_object_set(server, "service", current_destination_port, NULL);

    // Start threads
    pthread_t rtsp_thread, monitor_thread;

    if (pthread_create(&monitor_thread, NULL, file_monitor_function, NULL)) {
        fprintf(stderr, "Error creating file monitoring thread.\n");
        return 1;
    }

    if (pthread_create(&rtsp_thread, NULL, rtsp_server_function, server)) {
        fprintf(stderr, "Error creating RTSP server thread.\n");
        return 1;
    }

    // Wait for threads to finish
    pthread_join(rtsp_thread, NULL);
    pthread_join(monitor_thread, NULL);

    return 0;
}
