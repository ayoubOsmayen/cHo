#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

#define MAX_INPUT_STRING 1024
#define PORT 8080

typedef struct {
    char *mount_point;
    char *destination;
    char *port;
    char *username;
    char *password;
} ServerConfig;

void save_to_file(const char *filename, ServerConfig config);
void *serverThread(void *arg);

int main(int argc, const char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <rtsp_flux> <mount_point> <port> <username> <password>\n", argv[0]);
        return EXIT_FAILURE;
    }

    ServerConfig config;
    config.mount_point = strdup(argv[2]);
    config.destination = strdup(argv[1]);
    config.port = strdup(argv[3]);
    config.username = strdup(argv[4]);
    config.password = strdup(argv[5]);

    save_to_file("data.txt", config);

    pthread_t thread_id;
    printf("Starting RTSP server thread...\n");
    pthread_create(&thread_id, NULL, serverThread, &config);
    pthread_join(thread_id, NULL);

    printf("Server terminated.\n");
    return 0;
}

void save_to_file(const char *filename, ServerConfig config) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("File open error");
        return;
    }
    fprintf(file, "RTSP Source: %s\n", config.destination);
    fprintf(file, "Mount Point: %s\n", config.mount_point);
    fprintf(file, "Port: %s\n", config.port);
    fprintf(file, "Username: %s\n", config.username);
    fprintf(file, "Password: %s\n", config.password);
    fclose(file);
    printf("Configuration saved to %s\n", filename);
}

void *serverThread(void *arg) {
    ServerConfig *config = (ServerConfig *)arg;

    gst_init(NULL, NULL);

    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_launch(factory,
        g_strdup_printf("( rtspsrc location=%s ! rtph264depay ! h264parse ! mp4mux ! filesink location=video.mp4 )",
                        config->destination));

    gst_rtsp_mount_points_add_factory(mounts, config->mount_point, factory);
    g_object_unref(mounts);

    gst_rtsp_server_set_service(server, config->port);

    if (!gst_rtsp_server_attach(server, NULL)) {
        g_printerr("Failed to attach RTSP server to port %s\n", config->port);
        return NULL;
    }

    printf("RTSP server running on rtsp://127.0.0.1:%s%s\n", config->port, config->mount_point);

    g_main_loop_run(g_main_loop_new(NULL, FALSE));
    return NULL;
}
