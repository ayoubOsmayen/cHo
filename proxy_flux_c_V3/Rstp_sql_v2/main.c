#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    STATE_INACTIVE,
    STATE_ACTIVE
} ServerState;

typedef struct {
    char *rtsp_source;
    char *mount_point;
    char *destination_port;
    char *username;
    char *password;
    char *overlay_text;
    ServerState state;
} ServerConfig;

GMainLoop *loop;
GstRTSPServer *server;
GstRTSPMountPoints *mounts;
GstRTSPMediaFactory *factory;

void *check_stream_status(void *arg) {
    ServerConfig *config = (ServerConfig *)arg;
    if (config->state == STATE_ACTIVE) {
        g_print("Server is active, streaming...\n");
        return NULL;
    }
    g_print("Server is inactive.\n");
    return NULL;
}

void save_stream_to_mp4(GstElement *pipeline) {
    GstElement *sink = gst_element_factory_make("filesink", "sink");
    g_object_set(sink, "location", "output.mp4", NULL);
    gst_bin_add(GST_BIN(pipeline), sink);
    gst_element_link(pipeline, sink);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

void setup_rtsp_server(ServerConfig *config) {
    gchar *launch_string = g_strdup_printf(
        "( rtspsrc location=%s ! rtph264depay ! decodebin ! textoverlay text=\"%s\" font-desc=\"Sans 34px\" shaded-background=true ! "
        "videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 )",
        config->rtsp_source, config->overlay_text);

    gst_rtsp_media_factory_set_launch(factory, launch_string);
    g_free(launch_string);

    g_object_set(factory, "auth-username", config->username, "auth-password", config->password, NULL);
    gst_rtsp_mount_points_add_factory(mounts, config->mount_point, factory);

    g_object_unref(mounts);
    gst_rtsp_server_attach(server, NULL);
    g_print("RTSP server started at rtsp://127.0.0.1:%s%s\n", config->destination_port, config->mount_point);

    // Save the stream to MP4
    GstElement *pipeline = gst_parse_launch(launch_string, NULL);
    save_stream_to_mp4(pipeline);
    g_main_loop_run(loop);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        g_printerr("Usage: %s <rtsp_source> <mount_point> <destination_port> <login> <password> <overlay_text>\n", argv[0]);
        return -1;
    }

    gst_init(&argc, &argv);

    ServerConfig config = {
        .rtsp_source = argv[1],
        .mount_point = argv[2],
        .destination_port = argv[3],
        .username = argv[4],
        .password = argv[5],
        .overlay_text = argv[6],
        .state = STATE_ACTIVE
    };

    loop = g_main_loop_new(NULL, FALSE);
    server = gst_rtsp_server_new();
    g_object_set(server, "service", config.destination_port, NULL);

    mounts = gst_rtsp_server_get_mount_points(server);
    factory = gst_rtsp_media_factory_new();

    pthread_t stream_status_thread;
    pthread_create(&stream_status_thread, NULL, check_stream_status, &config);
    pthread_join(stream_status_thread, NULL);

    if (config.state == STATE_ACTIVE) {
        setup_rtsp_server(&config);
    }

    return 0;
}
