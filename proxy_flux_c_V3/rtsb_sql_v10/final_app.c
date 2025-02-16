#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    const char *rtsp_url = "rtsp://152.228.142.4:5327/video5327";
    const char *file_uri = "/home/ayoub/rtsb_sql_v10/file_input.txt";
    int port = 8554;
    const char *video1 = "file:///amine/cppuser/rtsp/phstreamv1/video.mp4";
    const char *video2 = "file:///amine/cppuser/rtsp/phstreamv1/video2.webp";

    printf("Static inputs being used:\n");
    printf("RTSP URL: %s\n", rtsp_url);
    printf("File URI: %s\n", file_uri);
    printf("Port: %d\n", port);
    printf("Video 1: %s\n", video1);
    printf("Video 2: %s\n", video2);

    GstRTSPServer *server = gst_rtsp_server_new();
    gst_rtsp_server_set_service(server, g_strdup_printf("%d", port));

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);

    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_shared(factory, TRUE);

    // GStreamer pipeline using input_selector to select between video1 and video2
    const gchar *pipeline = g_strdup_printf(
        "uridecodebin uri=%s ! queue ! input_selector.sink_0 "
        "uridecodebin uri=%s ! queue ! input_selector.sink_1 "
        "input-selector name=input_selector ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96",
        video1, video2);

    gst_rtsp_media_factory_set_launch(factory, pipeline);
    g_free((void *)pipeline);

    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_object_unref(mounts);

    if (gst_rtsp_server_attach(server, NULL) == 0) {
        g_printerr("Failed to attach the RTSP server.\n");
        return -1;
    }

    printf("RTSP server is running at rtsp://127.0.0.1:%d/test\n", port);
    printf("Static RTSP URL: %s\n", rtsp_url);
    printf("File URI (not used in pipeline currently): %s\n", file_uri);

    g_main_loop_run(g_main_loop_new(NULL, FALSE));

    g_object_unref(server);
    return 0;
}
