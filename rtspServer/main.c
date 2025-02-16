#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <glib.h>
#include <sys/inotify.h>
#include <errno.h>
#include "config.h"
#include "functions.h"

GMutex config_mutex;
char *current_video_source = NULL, *current_mount_point = NULL, *current_destination_port = NULL, *current_text = NULL;

gboolean config_file_changed(gpointer data) {
    GstRTSPServer *server = (GstRTSPServer *)data;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    char *video_source = NULL, *mount_point = NULL, *destination_port = NULL, *text = NULL;

    const char *config_file = (const char *)g_object_get_data(G_OBJECT(server), "config_file");

    if (lire_configuration(config_file, &video_source, &mount_point, &destination_port, &text) != 0) {
        return G_SOURCE_CONTINUE;
    }

    g_mutex_lock(&config_mutex);

    gboolean config_changed = FALSE;
    if (g_strcmp0(video_source, current_video_source) != 0 ||
        g_strcmp0(mount_point, current_mount_point) != 0 ||
        g_strcmp0(destination_port, current_destination_port) != 0 ||
        g_strcmp0(text, current_text) != 0) {
        config_changed = TRUE;
    }

    if (config_changed) {
        mounts = gst_rtsp_server_get_mount_points(server);
        gst_rtsp_mount_points_remove_factory(mounts, current_mount_point);
        g_object_unref(mounts);

        factory = gst_rtsp_media_factory_new();
        gchar *launch_string = g_strdup_printf(
            "( uridecodebin uri=%s ! videoconvert ! textoverlay text=\"%s\" font-desc=\"Sans 32px\" shaded-background=true ! x264enc ! rtph264pay name=pay0 pt=96 )",
            video_source, text);
        gst_rtsp_media_factory_set_launch(factory, launch_string);
        g_free(launch_string);

        mounts = gst_rtsp_server_get_mount_points(server);
        gst_rtsp_mount_points_add_factory(mounts, mount_point, factory);
        g_object_unref(mounts);

        g_free(current_video_source);
        g_free(current_mount_point);
        g_free(current_destination_port);
        g_free(current_text);

        current_video_source = video_source;
        current_mount_point = mount_point;
        current_destination_port = destination_port;
        current_text = text;

        printf("Configuration mise à jour.\n");
    } else {
        g_free(video_source);
        g_free(mount_point);
        g_free(destination_port);
        g_free(text);
    }

    g_mutex_unlock(&config_mutex);

    return G_SOURCE_CONTINUE;
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    int inotify_fd, wd;

    setlocale(LC_ALL, "");

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        return -1;
    }

    const char *config_file = argv[1];

    gst_init(&argc, &argv);

    if (access(config_file, F_OK) == -1) {
        fprintf(stderr, "Le fichier de configuration '%s' n'existe pas.\n", config_file);
        return -1;
    }

    if (lire_configuration(config_file, &current_video_source, &current_mount_point, &current_destination_port, &current_text) != 0) {
        return -1;
    }

    loop = g_main_loop_new(NULL, FALSE);
    server = gst_rtsp_server_new();

    g_object_set(server, "service", current_destination_port, NULL);
    mounts = gst_rtsp_server_get_mount_points(server);

    factory = gst_rtsp_media_factory_new();
    gchar *launch_string = g_strdup_printf(
        "( uridecodebin uri=%s ! videoconvert ! textoverlay text=\"%s\" font-desc=\"Sans 32px\" shaded-background=true ! x264enc ! rtph264pay name=pay0 pt=96 )",
        current_video_source, current_text);

    gst_rtsp_media_factory_set_launch(factory, launch_string);
    g_free(launch_string);

    gst_rtsp_mount_points_add_factory(mounts, current_mount_point, factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);

    g_object_set_data(G_OBJECT(server), "config_file", (gpointer)config_file);

    g_timeout_add_seconds(1, config_file_changed, server);

    g_main_loop_run(loop);

    g_free(current_video_source);
    g_free(current_mount_point);
    g_free(current_destination_port);
    g_free(current_text);
    g_main_loop_unref(loop);
    g_object_unref(server);
    g_object_unref(factory);

    g_mutex_clear(&config_mutex);

    return 0;
}
