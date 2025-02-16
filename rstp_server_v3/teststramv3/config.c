#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void read_configv1(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    fscanf(file, "%255s\n%255s\n%d\n%255[^\n]", 
           config->video_url, 
           config->rtsp_mount, 
           &config->rtsp_port, 
           config->overlay_text);
    fclose(file);

    printf("Config loaded: Video URL=%s, Mount=%s, Port=%d, Text=%s\n",
           config->video_url, config->rtsp_mount, config->rtsp_port, config->overlay_text);
}
