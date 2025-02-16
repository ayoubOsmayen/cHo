/* config.h */
#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char video_url[256];
    char rtsp_mount[256];
    int rtsp_port;
    char overlay_text[256];
} Config;

void read_configv1(const char *filename, Config *config);

#endif