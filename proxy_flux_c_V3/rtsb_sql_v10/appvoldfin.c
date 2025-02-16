#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <glib.h>

static GMainLoop *loop;
static GstElement *pipeline, *input_selector, *text_overlay;
static GstRTSPServer *server;
static GstRTSPMediaFactory *factory;
static const char *file_path = "/home/ayoub/rtsb_sql_v10/file_input.txt";
static gchar *initial_text = "Default Text";

void update_text_overlay(const char *prefix);
void switch_camera(char key);
gboolean check_file_update(gpointer user_data);

static gboolean stop_server(gpointer data) {
    g_print("Stopping RTSP server...\n");
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;
}

void update_text_overlay(const char *prefix) {
    if (text_overlay) {
        gchar *final_text = NULL;

        if (g_strcmp0(prefix, "REC") == 0) {
            final_text = g_strdup("REC");
        } else if (g_strcmp0(prefix, "ALARME") == 0) {
            final_text = g_strdup(initial_text);
        } else {
            final_text = g_strdup_printf("%s : %s", prefix, initial_text);
        }

        g_object_set(G_OBJECT(text_overlay), "text", final_text, NULL);
        g_free(final_text);
    } else {
        g_printerr("Text overlay element is not initialized.\n");
    }
}

void switch_camera(char key) {
    GstPad *new_active_pad = NULL;
    const char *prefix = NULL;

    if (!input_selector) {
        g_printerr("Input selector is not initialized.\n");
        return;
    }

    switch (key) {
        case '1': new_active_pad = gst_element_get_static_pad(input_selector, "sink_0"); prefix = "ON"; break;
        case '2': new_active_pad = gst_element_get_static_pad(input_selector, "sink_1"); prefix = "ALARME"; break;
        case '3': new_active_pad = gst_element_get_static_pad(input_selector, "sink_2"); prefix = "NO REC"; break;
        case '4': new_active_pad = gst_element_get_static_pad(input_selector, "sink_3"); prefix = "NO LIVE"; break;
        case '5': new_active_pad = gst_element_get_static_pad(input_selector, "sink_4"); prefix = "NO REC NO LIVE"; break;
        case '6': new_active_pad = gst_element_get_static_pad(input_selector, "sink_5"); prefix = "REC"; break;
        case '7': new_active_pad = gst_element_get_static_pad(input_selector, "sink_6"); prefix = "LIVE"; break;
        default: g_printerr("Invalid key: %c\n", key); return;
    }

    if (new_active_pad) {
        g_object_set(G_OBJECT(input_selector), "active-pad", new_active_pad, NULL);
        gst_object_unref(new_active_pad);
        update_text_overlay(prefix);
        g_print("Switched to camera %c\n", key);
    } else {
        g_printerr("Failed to get active pad for camera %c\n", key);
    }
}

gboolean check_file_update(gpointer user_data) {
    static time_t last_mod_time = 0;
    struct stat file_stat;

    if (stat(file_path, &file_stat) == -1) {
        perror("Error accessing file");
        return TRUE;
    }

    if (file_stat.st_mtime > last_mod_time) {
        last_mod_time = file_stat.st_mtime;

        FILE *file = fopen(file_path, "r");
        if (!file) {
            perror("Error opening file");
            return TRUE;
        }

        char key = fgetc(file);
        fclose(file);

        if (key >= '1' && key <= '7') {
            switch_camera(key);
        }
    }

    return TRUE;
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_print("End of stream. Restarting...\n");
            gst_element_set_state(pipeline, GST_STATE_READY);
            gst_element_set_state(pipeline, GST_STATE_PLAYING);
            break;
        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            g_printerr("Error: %s\n", error->message);
            g_error_free(error);
            g_free(debug);
            g_main_loop_quit(loop);
            break;
        }
        default: break;
    }
    return TRUE;
}

static void create_rtsp_server(const char *rtsp_url, const char *file_uri, const char *port, const char *video1, const char *video2) {
    g_print("Creating RTSP server...\n");
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, port);

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    factory = gst_rtsp_media_factory_new();

    gchar *launch_string = g_strdup_printf(
        "( input-selector name=input_selector ! textoverlay name=text_overlay text=\"%s\" font-desc=\"Sans, 32\" ! "
        "videoconvert ! x264enc tune=zerolatency bitrate=2048 ! rtph264pay name=pay0 pt=96 "
        "uridecodebin uri=%s ! queue ! input_selector.sink_0 "
        "uridecodebin uri=%s ! queue ! input_selector.sink_1 "
        "uridecodebin uri=%s ! queue ! input_selector.sink_5 "
        "rtspsrc location=%s ! rtpjitterbuffer ! rtph264depay ! avdec_h264 ! videoconvert ! queue ! input_selector.sink_6 )",
        initial_text, video1, video2, file_uri, rtsp_url
    );

    gst_rtsp_media_factory_set_launch(factory, launch_string);
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_free(launch_string);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);
    g_print("RTSP server running at rtsp://127.0.0.1:%s/test\n", port);
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    if (argc < 7) {
        g_printerr("Usage: %s <rtsp-url> <file-uri> <port> <video1> <video2>\n", argv[0]);
        return -1;
    }

    const char *rtsp_url = argv[1];
    const char *file_uri = argv[2];
    const char *port = argv[3];
    const char *video1 = argv[4];
    const char *video2 = argv[5];

    create_rtsp_server(rtsp_url, file_uri, port, video1, video2);

    g_timeout_add_seconds(1, check_file_update, NULL);
    signal(SIGINT, (void (*)(int))stop_server);
    g_main_loop_run(loop);

    return 0;
}
