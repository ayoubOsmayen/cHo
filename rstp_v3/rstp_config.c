#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILE "config.txt"

// Structure pour stocker les informations de configuration
typedef struct {
    gchar* rtsp_url;
    gchar* video_uri;
    gint port;
    gchar* overlay_text;
} Config;

// Fonction pour charger la configuration à partir du fichier config.txt
Config load_config() {
    Config config = {NULL, NULL, 0, NULL};
    FILE* file = fopen(CONFIG_FILE, "r");
    if (!file) {
        g_printerr("Erreur lors de l'ouverture du fichier de configuration.\n");
        exit(EXIT_FAILURE);
    }
    
    char line[1024];
    fgets(line, sizeof(line), file); // Lecture de l'URL RTSP
    config.rtsp_url = g_strdup(line);
    fgets(line, sizeof(line), file); // URI du fichier vidéo
    config.video_uri = g_strdup(line);
    fgets(line, sizeof(line), file); // Port du serveur RTSP
    config.port = atoi(line);
    fgets(line, sizeof(line), file); // Texte initial de l'overlay
    config.overlay_text = g_strdup(line);
    
    fclose(file);
    return config;
}

// Fonction pour mettre à jour dynamiquement le texte de l'overlay
void update_overlay_text(GstElement* overlay, const gchar* text) {
    g_object_set(overlay, "text", text, NULL);
}

// Fonction pour créer le pipeline GStreamer
GstElement* create_pipeline(Config config) {
    GstElement* pipeline;
    GstElement* rtsp_src;
    GstElement* decode_bin;
    GstElement* video_convert;
    GstElement* overlay;
    GstElement* video_sink;
    
    pipeline = gst_pipeline_new("rtsp-pipeline");
    
    // Création des éléments GStreamer
    rtsp_src = gst_element_factory_make("rtspsrc", "rtsp-src");
    decode_bin = gst_element_factory_make("decodebin", "decode-bin");
    video_convert = gst_element_factory_make("videoconvert", "video-convert");
    overlay = gst_element_factory_make("textoverlay", "overlay");
    video_sink = gst_element_factory_make("autovideosink", "video-sink");
    
    if (!pipeline || !rtsp_src || !decode_bin || !video_convert || !overlay || !video_sink) {
        g_printerr("Impossible de créer les éléments GStreamer.\n");
        return NULL;
    }
    
    // Configuration du source RTSP
    g_object_set(rtsp_src, "location", config.rtsp_url, NULL);
    
    // Connexion des éléments
    gst_bin_add_many(GST_BIN(pipeline), rtsp_src, decode_bin, video_convert, overlay, video_sink, NULL);
    gst_element_link(rtsp_src, decode_bin);
    gst_element_link(video_convert, overlay);
    gst_element_link(overlay, video_sink);
    
    // Connexion à la sortie du decodeur
    g_signal_connect(decode_bin, "pad-added", G_CALLBACK(gst_element_link_pads), video_convert);
    
    // Mise à jour du texte overlay
    update_overlay_text(overlay, config.overlay_text);
    
    return pipeline;
}

// Fonction pour démarrer le serveur RTSP
void start_rtsp_server(Config config, GstElement* pipeline) {
    GstRTSPServer* server;
    GstRTSPMediaFactory* factory;
    GstRTSPMedia* media;
    
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_address(server, "0.0.0.0");
    gst_rtsp_server_set_service(server, "8554");  // Port 8554
    
    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory, "( rtspsrc location=rtsp://192.168.1.100:8554/test ! decodebin ! videoconvert ! textoverlay name=overlay ! autovideosink )");
    
    gst_rtsp_server_attach(server, NULL);
    
    g_print("Serveur RTSP démarré sur rtsp://localhost:%d/\n", config.port);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_main_loop_run(g_main_loop_new(NULL, FALSE));
}

// Fonction principale
int main(int argc, char* argv[]) {
    gst_init(&argc, &argv);
    
    // Charger la configuration à partir du fichier config.txt
    Config config = load_config();
    
    // Créer le pipeline GStreamer
    GstElement* pipeline = create_pipeline(config);
    if (!pipeline) {
        return EXIT_FAILURE;
    }
    
    // Démarrer le serveur RTSP
    start_rtsp_server(config, pipeline);
    
    // Nettoyer la mémoire
    g_free(config.rtsp_url);
    g_free(config.video_uri);
    g_free(config.overlay_text);
    
    return EXIT_SUCCESS;
}
