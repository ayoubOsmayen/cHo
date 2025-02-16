#ifndef CONFIG_RTSP_H
#define CONFIG_RTSP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 256

typedef struct {
    char rtsp_url[MAX_STRING_LENGTH];  // RTSP URL
    char video_file[MAX_STRING_LENGTH]; // Local video file
    int port;                           // Port
    char overlay_text[MAX_STRING_LENGTH]; // Initial overlay text
} Config;

// Function prototypes
int load_config(const char *config_file, Config *config);
void print_config(const Config *config);

#endif // CONFIG_H
