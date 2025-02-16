#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <gst/gst.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define CONFIG_FILE "config.txt"
#define SOCKET_PORT 12345
#define MAX_CLIENTS 10

// Structure pour la configuration
typedef struct {
    char video_url[256];
    char rtsp_mount[128];
    int rtsp_port;
    char display_text[256];
} Config;

Config config;

// Fonction de lecture du fichier de configuration
Config read_config(const char *config_file) {
    Config config;
    FILE *file = fopen(config_file, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de configuration");
        exit(1);
    }
    fgets(config.video_url, sizeof(config.video_url), file);
    config.video_url[strcspn(config.video_url, "\n")] = 0;
    fgets(config.rtsp_mount, sizeof(config.rtsp_mount), file);
    config.rtsp_mount[strcspn(config.rtsp_mount, "\n")] = 0;
    fscanf(file, "%d\n", &config.rtsp_port);
    fgets(config.display_text, sizeof(config.display_text), file);
    config.display_text[strcspn(config.display_text, "\n")] = 0;
    fclose(file);
    return config;
}

// Fonction pour créer le pipeline GStreamer
void create_pipeline(const char *video_url, const char *display_text) {
    gst_init(NULL, NULL);

    GstElement *pipeline, *video_source, *text_overlay, *rtsp_sink;
    pipeline = gst_pipeline_new("video-pipeline");

    video_source = gst_element_factory_make("uridecodebin", "video_source");
    g_object_set(video_source, "uri", video_url, NULL);

    text_overlay = gst_element_factory_make("textoverlay", "text_overlay");
    g_object_set(text_overlay, "text", display_text, NULL);
    g_object_set(text_overlay, "valignment", 2, NULL); // Centrage vertical

    rtsp_sink = gst_element_factory_make("rtspclientsink", "rtsp_sink");
    g_object_set(rtsp_sink, "location", "rtsp://localhost:5267", NULL);

    gst_bin_add_many(GST_BIN(pipeline), video_source, text_overlay, rtsp_sink, NULL);
    gst_element_link_many(video_source, text_overlay, rtsp_sink, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
}

// Surveillance du fichier de configuration avec inotify
void* watch_config_file(void *arg) {
    int fd, wd;
    char buffer[1024];
    fd = inotify_init();
    if (fd == -1) {
        perror("inotify_init");
        exit(1);
    }
    wd = inotify_add_watch(fd, CONFIG_FILE, IN_MODIFY);
    if (wd == -1) {
        perror("inotify_add_watch");
        exit(1);
    }

    while (1) {
        int length = read(fd, buffer, sizeof(buffer));
        if (length < 0) {
            perror("read");
        }
        for (int i = 0; i < length; i += sizeof(struct inotify_event) + ((struct inotify_event*)&buffer[i])->len) {
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            if (event->mask & IN_MODIFY) {
                printf("Fichier de configuration modifié\n");
                config = read_config(CONFIG_FILE); // Recharger la config
            }
        }
    }

    close(fd);
    return NULL;
}

// Fonction pour démarrer un serveur socket
void* start_socket_server(void *arg) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Échec de la création du socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SOCKET_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Échec du binding du socket");
        exit(1);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Échec de l'écoute du socket");
        exit(1);
    }

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Échec de l'acceptation du socket");
            continue;
        }

        printf("Nouveau client connecté : %s\n", inet_ntoa(client_addr.sin_addr));
        // Gérer la communication client dans un thread séparé
        close(client_fd);
    }

    close(server_fd);
    return NULL;
}

// Fonction principale
int main() {
    // Charger la configuration
    config = read_config(CONFIG_FILE);

    // Créer le pipeline GStreamer
    create_pipeline(config.video_url, config.display_text);

    // Créer les threads pour surveiller les fichiers et gérer le serveur socket
    pthread_t config_thread, server_thread;
    pthread_create(&config_thread, NULL, watch_config_file, NULL);
    pthread_create(&server_thread, NULL, start_socket_server, NULL);

    // Attendre la fin des threads
    pthread_join(config_thread, NULL);
    pthread_join(server_thread, NULL);

    return 0;
}
