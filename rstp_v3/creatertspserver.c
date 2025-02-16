
ayoub smayen <ayoubjobs.2019@gmail.com>
08:58 (il y a 3 heures)
À moi

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

static gboolean bus_callback (GstBus * bus, GstMessage * msg, gpointer user_data) {
    GError *error;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_ERROR:
            gst_message_parse_error(msg, &error, &debug_info);
            g_print("ERROR: %s\n", error->message);
            g_clear_error(&error);
            g_free(debug_info);
            return FALSE;
        default:
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    GstRTSPServer *server;
    GstRTSPMediaFactory *factory;
    GstBus *bus;
    guint bus_watch_id;

    server = gst_rtsp_server_new();
    factory = gst_rtsp_media_factory_new();

    // Configurer le pipeline de médias RTSP (exemple avec vidéo en boucle)
    gst_rtsp_media_factory_set_launch(factory, "( videotestsrc ! x264enc ! rtph264pay name=pay0 pt=96 )");
    gst_rtsp_server_attach(server, NULL);

    // Créer un point de montage
    gst_rtsp_server_mount_points_add_factory(gst_rtsp_server_get_mount_points(server), "/test", factory);

    bus = gst_rtsp_server_get_bus(server);
    bus_watch_id = gst_bus_add_watch(bus, bus_callback, NULL);

    g_print("Serveur RTSP en écoute sur rtsp://127.0.0.1:8554/test\n");
    gst_rtsp_server_loop(server);

    return 0;
}