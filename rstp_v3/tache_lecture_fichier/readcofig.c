#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char video_url[256];
    char rtsp_mount[256];
    int rtsp_port;
    char display_text[256];
} Config;

Config read_config(const char *config_file) {
    Config config;
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        perror("Error opening configuration file");
        exit(1);
    }

    // Read the URL or RTSP URL
    fgets(config.video_url, sizeof(config.video_url), file);
    config.video_url[strcspn(config.video_url, "\n")] = 0;  // Remove newline

    // Read RTSP mount path
    fgets(config.rtsp_mount, sizeof(config.rtsp_mount), file);
    config.rtsp_mount[strcspn(config.rtsp_mount, "\n")] = 0;  // Remove newline

    // Read RTSP port
    fscanf(file, "%d\n", &config.rtsp_port);

    // Read display text
    fgets(config.display_text, sizeof(config.display_text), file);
    config.display_text[strcspn(config.display_text, "\n")] = 0;  // Remove newline

    fclose(file);
    return config;
}
int main (int argc , const char *argv[])
{
     char filename[1024];
     strcpy(filename, argv[1]);
     //sudo apt-get install libgstreamer1.0-dev gstreamer1.0-rtsp-server


     Config conf = read_config (argv[1]);
     printf("%s %s"  , conf.video_url,conf.rtsp_mount);


    return  1;
}