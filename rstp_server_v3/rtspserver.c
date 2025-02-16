#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "config_rtsp.h"

// Global variables
static GstRTSPServer *server = NULL;
static GstRTSPMediaFactory *factory = NULL;
static gchar *current_camera_url = NULL;

// Function to create the RTSP server with the pipeline
GstRTSPMediaFactory* create_rtsp_server(Config *config) {
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
    GstElement *pipeline;
    GstElement *video_source, *video_decoder, *video_encoder, *text_overlay;
    GstElement *video_sink;
    
    // Build pipeline
    video_source = gst_element_factory_make("rtspsrc", "video_source");
    g_object_set(G_OBJECT(video_source), "location", config->rtsp_url, NULL);

    video_decoder = gst_element_factory_make("decodebin", "video_decoder");
    video_encoder = gst_element_factory_make("x264enc", "video_encoder");
    text_overlay = gst_element_factory_make("textoverlay", "text_overlay");

    g_object_set(G_OBJECT(text_overlay), "text", config->overlay_text, NULL);

    video_sink = gst_element_factory_make("rtspclientsink", "video_sink");

    pipeline = gst_pipeline_new("video-pipeline");
    gst_bin_add_many(GST_BIN(pipeline), video_source, video_decoder, video_encoder, text_overlay, video_sink, NULL);
    gst_element_link(video_source, video_decoder);
    gst_element_link(video_decoder, video_encoder);
    gst_element_link(video_encoder, text_overlay);
    gst_element_link(text_overlay, video_sink);

    gst_rtsp_media_factory_set_launch(factory, "( v4l2src ! decodebin ! x264enc ! rtph264pay )");
    return factory;
}

// Function to handle the RTSP server
void start_rtsp_server(Config *config) {
    server = gst_rtsp_server_new();
    gst_rtsp_server_set_address(server, "0.0.0.0");
    gst_rtsp_server_set_service(server, "8554");

    factory = create_rtsp_server(config);
    gst_rtsp_server_attach(server, NULL);
    
    g_print("RTSP server running on rtsp://localhost:%d\n", config->port);
    g_main_loop_run(g_main_loop_new(NULL, FALSE));  // Start the main loop
}

// Function to monitor the configuration file and update dynamically
void check_file_update(const char *file_path, Config *config) {
    FILE *file = fopen(file_path, "r");
    if (file != NULL) {
        char key;
        fscanf(file, "%c", &key);
        fclose(file);
        
        // Here, map the key to a camera URL, for example:
        if (key == '1') {
            strcpy(config->rtsp_url, "rtsp://192.168.1.101:8554/test1");
        } else if (key == '2') {
            strcpy(config->rtsp_url, "rtsp://192.168.1.102:8554/test2");
        }

        // Restart RTSP server with new configuration
        g_print("Switching to camera %c\n", key);
        start_rtsp_server(config);
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <config_file> <file_with_key>\n", argv[0]);
        return -1;
    }

    Config config;
    if (load_config(argv[1], &config) != 0) {
        return -1;
    }
    
    print_config(&config);

    // Initialize GStreamer
    gst_init(&argc, &argv);
    
    // Start RTSP server
    start_rtsp_server(&config);
    
    // Monitor the file for camera changes
    while (1) {
        check_file_update(argv[2], &config);
        sleep(1);  // Check for updates every second
    }

    return 0;
}
