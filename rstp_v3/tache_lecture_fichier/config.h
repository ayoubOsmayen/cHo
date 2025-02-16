#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 256

typedef struct {
    char rtsp_url[MAX_STRING_LENGTH];  // RTSP URL
    char video_file[MAX_STRING_LENGTH]; // Local video file
    int port;                           // Port
    int overlay_enabled;                // Overlay enabled/disabled
    char overlay_text[MAX_STRING_LENGTH]; // Overlay text
} Config;

// Function prototypes
int load_config(const char *config_file, Config *config);
void print_config(const Config *config);
long get_file_size(const char *filename);

#endif // CONFIG_H
