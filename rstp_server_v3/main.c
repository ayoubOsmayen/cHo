#include <stdio.h>
#include <stdlib.h>
#include "config.h"

int main() {
    Config config;

    // Load configuration from a file
    if (load_config("config.txt", &config) != 0) {
        printf("Failed to load configuration\n");
        return 1;
    }

    // Print the configuration
    print_config(&config);

    // Get file size of the video file
    long video_size = get_file_size(config.video_file);
    if (video_size == -1) {
        printf("Failed to get the file size\n");
        return 1;
    }
    printf("Video file size: %ld bytes\n", video_size);

    // You could implement logic here to process the RTSP stream or video file with overlay

    if (config.overlay_enabled) {
        // Implement overlay logic here if enabled
        printf("Applying overlay with text: %s\n", config.overlay_text);
        // For example, use some video processing library to add overlay
    }

    // Further processing or streaming logic
    printf("Processing video from RTSP URL: %s\n", config.rtsp_url);

    // Process the video or stream in your desired way
    // Example: Initialize an RTSP client and stream video
    // You would need a library like FFmpeg or OpenCV for actual video handling

    return 0;
}
