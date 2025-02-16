#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <glib.h>
#include <signal.h>
#include <pthread.h>

GMainLoop *loop = NULL; // Main loop for RTSP server
char *current_video_source = NULL, *current_mount_point = NULL, *current_destination_port = NULL, *current_text = NULL;
GstElement *text_overlay_element = NULL;

// Signal handler for clean shutdown
void handle_sigterm(int sig) {
    if (loop) {
        g_main_loop_quit(loop);
    }
}

// Function to read configuration
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

// Main function
int main(int argc, char *argv[]) {
    GstRTSPServer *server = NULL;
    GstRTSPMountPoints *mounts = NULL;
    GstRTSPMediaFactory *factory = NULL;

    setlocale(LC_ALL, "");

    if (argc < 2) {
        g_printerr("Usage: %s <config_file>\n", argv[0]);
        return -1;
    }

    const char *config_file = argv[1];

    gst_init(&argc, &argv);

    if (read_configuration(config_file, &current_video_source, &current_mount_point, &current_destination_port, &current_text) != 0) {
        return -1;
    }

    // Initialize the RTSP server
    loop = g_main_loop_new(NULL, FALSE);
    server = gst_rtsp_server_new();
    g_object_set(server, "service", current_destination_port, NULL);
    mounts = gst_rtsp_server_get_mount_points(server);
    factory = gst_rtsp_media_factory_new();

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
        return -1;
    }

    // Signal handling for graceful shutdown
    signal(SIGTERM, handle_sigterm);
    signal(SIGINT, handle_sigterm);

    g_print("RTSP server is running at rtsp://127.0.0.1:%s%s\n", current_destination_port, current_mount_point);
    g_main_loop_run(loop);

    // Cleanup resources
    if (loop) {
        g_main_loop_unref(loop);
    }
    if (server) {
        g_object_unref(server);
    }
    if (factory) {
        g_object_unref(factory);
    }
    g_free(current_video_source);
    g_free(current_mount_point);
    g_free(current_destination_port);
    g_free(current_text);

    return 0;
}
