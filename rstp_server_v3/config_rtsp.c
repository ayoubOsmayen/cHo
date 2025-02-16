#include "config_rstp.h"

// Function to load the configuration from a file
int load_config(const char *config_file, Config *config) {
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        return -1;
    }

    // Read configuration values
    fscanf(file, "%255s", config->rtsp_url);   // RTSP URL
    fscanf(file, "%255s", config->video_file);  // Video file path
    fscanf(file, "%d", &config->port);          // RTSP port
    fscanf(file, "%255s", config->overlay_text); // Overlay text

    fclose(file);
    return 0;
}

// Function to print the configuration
void print_config(const Config *config) {
    printf("RTSP URL: %s\n", config->rtsp_url);
    printf("Video file: %s\n", config->video_file);
    printf("Port: %d\n", config->port);
    printf("Overlay text: %s\n", config->overlay_text);
}
