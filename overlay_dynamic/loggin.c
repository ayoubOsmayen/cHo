#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <gst/gst.h>
#include <string.h>

// Global variables to hold configuration values
char rtsp_url[256];
int gst_debug_level;
int enable_logging;

// Log function to log messages to the console and log file
void log_message(const char *message) {
    if (enable_logging) {
        FILE *log_file = fopen("log_service.txt", "a");
        if (log_file == NULL) {
            perror("Failed to open log file");
            return;
        }
        fprintf(log_file, "%s\n", message);
        fclose(log_file);
    }
    printf("%s\n", message);  // Print to console for real-time logging
}

// Read configuration from the conf.txt file
void read_config() {
    FILE *config_file = fopen("conf.txt", "r");
    if (config_file == NULL) {
        perror("Failed to open conf.txt");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), config_file)) {
        if (strncmp(line, "RTSP_URL=", 9) == 0) {
            sscanf(line + 9, "%s", rtsp_url);
        } else if (strncmp(line, "GST_DEBUG=", 10) == 0) {
            sscanf(line + 10, "%d", &gst_debug_level);
        } else if (strncmp(line, "ENABLE_LOGGING=", 15) == 0) {
            sscanf(line + 15, "%d", &enable_logging);
        }
    }

    fclose(config_file);
}

// Function to handle the RTSP video stream using GStreamer
void *rtsp_dynamic_vid(void *arg) {
    // Initialize GStreamer
    gst_init(NULL, NULL);

    // Set GStreamer debug level based on configuration
    char gst_debug_env[10];
    snprintf(gst_debug_env, sizeof(gst_debug_env), "%d", gst_debug_level);
    setenv("GST_DEBUG", gst_debug_env, 1);

    log_message("RTSP stream initialized with GStreamer.");

    // Create a pipeline to stream video from the RTSP URL
    GstElement *pipeline = gst_parse_launch(
        "rtspsrc location=%s ! decodebin ! autovideosink", 
        NULL
    );

    if (!pipeline) {
        log_message("Failed to create GStreamer pipeline.");
        return NULL;
    }

    // Start playing the pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Wait for errors or end-of-stream (EOS)
    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = NULL;
    
    while (1) {
        msg = gst_bus_poll(bus, GST_MESSAGE_ERROR | GST_MESSAGE_EOS, GST_CLOCK_TIME_NONE);
        if (msg != NULL) {
            if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                GError *err = NULL;
                gchar *debug_info = NULL;
                gst_message_parse_error(msg, &err, &debug_info);
                log_message("GStreamer Error: ");
                log_message(err->message);
                g_error_free(err);
                g_free(debug_info);
            } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS) {
                log_message("End of Stream (EOS) received.");
            }
            gst_message_unref(msg);
            break;
        }
    }

    // Clean up GStreamer
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(bus);

    log_message("RTSP stream processing finished.");

    return NULL;
}

// Function to create a thread for RTSP stream processing
void create_log_service_thread() {
    pthread_t thread;
    int ret = pthread_create(&thread, NULL, rtsp_dynamic_vid, NULL);
    if (ret != 0) {
        log_message("Failed to create thread.");
        return;
    }

    // Wait for the thread to finish
    pthread_join(thread, NULL);
}

int main() {
    // Read configuration from conf.txt
    read_config();

    // Log the configuration settings
    log_message("Configuration loaded:");
    log_message("RTSP URL: ");
    log_message(rtsp_url);
    log_message("GStreamer Debug Level: ");
    log_message((gst_debug_level == 3) ? "Normal Debugging" : "Other Level");
    log_message("Enable Logging: ");
    log_message(enable_logging ? "true" : "false");

    // Create log service thread to process RTSP stream
    create_log_service_thread();

    // Final log message
    log_message("Program finished.");

    return 0;
}
