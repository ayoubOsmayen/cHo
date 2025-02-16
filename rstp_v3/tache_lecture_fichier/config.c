#include "config.h"

// Function to load the configuration from a file
int load_config(const char *config_file, Config *config) {
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        return -1;
    }

    // Read each line and populate the struct
    while (fscanf(file, "%255s", config->rtsp_url) != EOF) {
        fscanf(file, "%255s", config->video_file);
        fscanf(file, "%d", &config->port);
        fscanf(file, "%d", &config->overlay_enabled);
        if (config->overlay_enabled) {
            fscanf(file, "%255s", config->overlay_text);
        }
    }

    fclose(file);
    return 0;
}

// Function to print the configuration
void print_config(const Config *config) {
    printf("RTSP URL: %s\n", config->rtsp_url);
    printf("Video file: %s\n", config->video_file);
    printf("Port: %d\n", config->port);
    printf("Overlay enabled: %d\n", config->overlay_enabled);
    if (config->overlay_enabled) {
        printf("Overlay text: %s\n", config->overlay_text);
    }
}

// Function to get the file size of a given file
long get_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Unable to open file to get size");
        return -1;
    }

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}
