#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;

    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    server = gst_rtsp_server_new();
    mounts = gst_rtsp_server_get_mount_points(server);

    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, "( videotestsrc is-live=1 ! x264enc ! rtph264pay name=pay0 pt=96 )");
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);
    g_print("RTSP server is running at rtsp://127.0.0.1:8554/test\n");
    g_main_loop_run(loop);

    return 0;
}
