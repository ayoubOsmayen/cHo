#include <pthread.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "utils.h"

void *serverThread(void *args) {
    const char **argv = (const char **)args;

    gst_init(NULL, NULL);
    GstRTSPServer *server = gst_rtsp_server_new();
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    char pipeline[1024];
    snprintf(pipeline, sizeof(pipeline),
             "( rtspsrc location=%s user-id=%s user-pw=%s ! rtph264depay ! h264parse ! mp4mux ! filesink location=output.mp4 )",
             argv[1], argv[4], argv[5]);
    gst_rtsp_media_factory_set_launch(factory, pipeline);
    gst_rtsp_mount_points_add_factory(mounts, argv[2], factory);

    g_object_unref(mounts);
    gst_rtsp_server_set_service(server, argv[3]);

    if (gst_rtsp_server_attach(server, NULL) == 0) {
        g_printerr("Failed to attach RTSP server\n");
        return NULL;
    }

    g_print("RTSP server running at rtsp://127.0.0.1:%s%s\n", argv[3], argv[2]);
    g_main_loop_run(g_main_loop_new(NULL, FALSE));

    return NULL;
}

int main(int argc, const char *argv[]) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <RTSP source> <mount point> <port> <username> <password>\n", argv[0]);
        return -1;
    }

    FILE *file = fopen("data.txt", "w");
    if (!file) {
        perror("Error opening file");
        return -1;
    }
    fprintf(file, "RTSP Source: %s\nMount Point: %s\nPort: %s\nUsername: %s\nPassword: %s\n",
            argv[1], argv[2], argv[3], argv[4], argv[5]);
    fclose(file);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, serverThread, (void *)argv);
    pthread_join(thread_id, NULL);

    return 0;
}
