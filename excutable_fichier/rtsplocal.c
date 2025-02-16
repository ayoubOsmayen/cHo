#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_FILE "rtsp_server.log"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT "8555"
#define STREAM_PATH "/stream"

// Logging function
void log_message(const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        fprintf(log_file, "%s\n", message);
        fclose(log_file);
    }
    printf("%s\n", message); // Also print to console
}

// Pthread function to handle dynamic logs
void *log_thread_function(void *arg) {
    while (1) {
        log_message("RTSP server is running...");
        sleep(10); // Log a message every 10 seconds
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    pthread_t log_thread;

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create the main loop
    loop = g_main_loop_new(NULL, FALSE);

    // Create the RTSP server
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_address(server, SERVER_IP);
    gst_rtsp_server_set_service(server, SERVER_PORT);

    // Get the mount points for the server
    mounts = gst_rtsp_server_get_mount_points(server);

    // Create a media factory
    factory = gst_rtsp_media_factory_new();

    // Set the pipeline to include a text overlay
    gst_rtsp_media_factory_set_launch(factory,
                                      "( videotestsrc is-live=1 ! textoverlay text=\"RTSP Server Demo\" "
                                      "valign=top halign=center font-desc=\"Sans, 36\" ! "
                                      "videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )");

    // Enable shared media
    gst_rtsp_media_factory_set_shared(factory, TRUE);

    // Add the factory to the mount points
    gst_rtsp_mount_points_add_factory(mounts, STREAM_PATH, factory);
    g_object_unref(mounts);

    // Attach the server to the main context
    if (gst_rtsp_server_attach(server, NULL) == 0) {
        log_message("Failed to attach the RTSP server");
        return -1;
    }

    // Show the stream URL
    char stream_url[128];
    snprintf(stream_url, sizeof(stream_url), "Stream ready at rtsp://%s:%s%s", SERVER_IP, SERVER_PORT, STREAM_PATH);
    log_message(stream_url);

    // Start the logging thread
    if (pthread_create(&log_thread, NULL, log_thread_function, NULL) != 0) {
        log_message("Failed to create the logging thread");
        return -1;
    }

    // Run the main loop
    g_main_loop_run(loop);

    // Cleanup
    pthread_cancel(log_thread);
    pthread_join(log_thread, NULL);
    g_main_loop_unref(loop);

    return 0;
}
