#include <gst/gst.h>
#include <stdio.h>
#include "function.h"
#include "config.h"

void init_gstreamer(Config *config) {
    gst_init(NULL, NULL);
    char pipeline_str[1024];
    snprintf(pipeline_str, sizeof(pipeline_str),
             "rtspsrc location=%s ! decodebin ! textoverlay text=\"%s\" ! autovideosink",
             config->video_url, config->overlay_text);

    GstElement *pipeline = gst_parse_launch(pipeline_str, NULL);
    if (!pipeline) {
        fprintf(stderr, "Failed to create GStreamer pipeline\n");
        return;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    printf("GStreamer pipeline started.\n");
}
