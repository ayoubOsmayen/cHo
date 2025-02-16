#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#include <glib.h>
#include <sys/inotify.h>
#include <errno.h>
#include "mysqlconn.h"
#include <sys/time.h>
#include <limits.h>
#include <mariadb/mysql.h> 

GMutex config_mutex;
char *current_video_source = NULL, *current_mount_point = NULL, *current_destination_port = NULL, *current_text = NULL;

/// Client tracking methods
int INSERT_DB(ClientConnection *client, int status);
int get_current_time(char *timeBuff, int buffSize);
void rtsp_client_disconnect(GstRTSPClient *client, gpointer user_data);
void rtsp_client_connected (GstRTSPServer *gstrtspserver, GstRTSPClient *client, gpointer user_data);
ClientConnection* create_client_connection(const gchar *ip_address, const char *timeBuff);
void free_client_connection(ClientConnection *conStr);

/// Configuration reading and update
int lire_configuration(const char *fichier, char **video_source, char **mount_point, char **destination_port, char **text) {
    FILE *fp = fopen(fichier, "r");
    if (fp == NULL) {
        g_printerr("Impossible d'ouvrir le fichier de configuration.\n");
        return -1;
    }

    if (fscanf(fp, "%ms\n%ms\n%ms\n%ms", video_source, mount_point, destination_port, text) != 4) {
        g_printerr("Erreur de lecture du fichier de configuration.\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    g_print("Valeurs lues dans le fichier de configuration:\n");
    g_print("Video source: %s\n", *video_source);
    g_print("Mount point: %s\n", *mount_point);
    g_print("Destination port: %s\n", *destination_port);
    g_print("Text: %s\n", *text);

    return 0;
}

GstRTSPFilterResult client_filter(GstRTSPServer *server, GstRTSPClient *client, gpointer user_data) {
    gst_rtsp_client_close(client);
    return GST_RTSP_FILTER_REMOVE;
}

gboolean config_file_changed(gpointer data) {
    GstRTSPServer *server = (GstRTSPServer *)data;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    char *video_source = NULL, *mount_point = NULL, *destination_port = NULL, *text = NULL;

    const char *config_file = (const char *)g_object_get_data(G_OBJECT(server), "config_file");

    if (lire_configuration(config_file, &video_source, &mount_point, &destination_port, &text) != 0) {
        return G_SOURCE_CONTINUE;
    }

    g_mutex_lock(&config_mutex);

    // Check if the configuration has changed
    gboolean config_changed = FALSE;
    if (g_strcmp0(video_source, current_video_source) != 0 ||
        g_strcmp0(mount_point, current_mount_point) != 0 ||
        g_strcmp0(destination_port, current_destination_port) != 0 ||
        g_strcmp0(text, current_text) != 0) {
        config_changed = TRUE;
    }

    if (config_changed) {
        // Clean up previous factory and mounts
        mounts = gst_rtsp_server_get_mount_points(server);
        gst_rtsp_mount_points_remove_factory(mounts, current_mount_point);
        g_object_unref(mounts);

        factory = gst_rtsp_media_factory_new();
        gchar *launch_string = g_strdup_printf(
            "( uridecodebin uri=%s ! videoconvert ! textoverlay text=\"%s\" font-desc=\"Sans 32px\" shaded-background=true ! x264enc ! rtph264pay name=pay0 pt=96 )",
            video_source, text);
        gst_rtsp_media_factory_set_launch(factory, launch_string);
        g_free(launch_string);

        mounts = gst_rtsp_server_get_mount_points(server);
        gst_rtsp_mount_points_add_factory(mounts, mount_point, factory);
        g_object_unref(mounts);

        gst_rtsp_server_client_filter(server, client_filter, NULL);

        g_free(current_video_source);
        g_free(current_mount_point);
        g_free(current_destination_port);
        g_free(current_text);

        current_video_source = video_source;
        current_mount_point = mount_point;
        current_destination_port = destination_port;
        current_text = text;

        g_print("Configuration mise à jour.\n");
    } else {
        g_free(video_source);
        g_free(mount_point);
        g_free(destination_port);
        g_free(text);
    }

    g_mutex_unlock(&config_mutex);

    return G_SOURCE_CONTINUE;
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    int inotify_fd, wd;

    setlocale(LC_ALL, "");

    if (argc < 2) {
        g_printerr("Usage: %s <config_file>\n", argv[0]);
        return -1;
    }

    const char *config_file = argv[1];

    gst_init(&argc, &argv);

    if (access(config_file, F_OK) == -1) {
        g_printerr("Le fichier de configuration '%s' n'existe pas.\n", config_file);
        return -1;
    }

    if (lire_configuration(config_file, &current_video_source, &current_mount_point, &current_destination_port, &current_text) != 0) {
        return -1;
    }

    loop = g_main_loop_new(NULL, FALSE);
    server = gst_rtsp_server_new();

    g_object_set(server, "service", current_destination_port, NULL);
    mounts = gst_rtsp_server_get_mount_points(server);

    factory = gst_rtsp_media_factory_new();
    gchar *launch_string = g_strdup_printf(
        "( uridecodebin uri=%s ! videoconvert ! textoverlay text=\"%s\" font-desc=\"Sans 32px\" shaded-background=true ! x264enc ! rtph264pay name=pay0 pt=96 )",
        current_video_source, current_text);

    gst_rtsp_media_factory_set_launch(factory, launch_string);
    g_free(launch_string);

    gst_rtsp_mount_points_add_factory(mounts, current_mount_point, factory);
    g_object_unref(mounts);

    gst_rtsp_server_attach(server, NULL);

    g_object_set_data(G_OBJECT(server), "config_file", (gpointer)config_file);
    //client connection callback
    g_signal_connect(server, "client-connected", G_CALLBACK(rtsp_client_connected), NULL);

    g_print("Serveur RTSP démarré à l'adresse rtsp://127.0.0.1:%s%s\n", current_destination_port, current_mount_point);

    inotify_fd = inotify_init();
    if (inotify_fd == -1) {
        g_printerr("Erreur lors de l'initialisation d'inotify: %s\n", strerror(errno));
        return -1;
    }

    wd = inotify_add_watch(inotify_fd, config_file, IN_MODIFY);
    if (wd == -1) {
        g_printerr("Erreur lors de l'ajout d'un watch sur le fichier de configuration: %s\n", strerror(errno));
        close(inotify_fd);
        return -1;
    }

    g_mutex_init(&config_mutex);

    g_timeout_add_seconds(1, config_file_changed, server);

    g_main_loop_run(loop);

    inotify_rm_watch(inotify_fd, wd);
    close(inotify_fd);

    g_free(current_video_source);
    g_free(current_mount_point);
    g_free(current_destination_port);
    g_free(current_text);
    g_main_loop_unref(loop);
    g_object_unref(server);
    g_object_unref(factory);

    g_mutex_clear(&config_mutex);

    return 0;
}

// Get current time in a formatted string
int get_current_time(char *timeBuff, int buffSize) {
    if (timeBuff == NULL || buffSize <= 0) {
        g_printerr("Invalid buffer or buffer size.\n");
        return -1;
    }

    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) {
        g_printerr("Failed to get current time");
        return -1;
    }

    struct tm *tm_info = localtime(&tv.tv_sec);
    if (tm_info == NULL) {
        g_printerr("Failed to convert time to local time");
        return -1;
    }

    if (strftime(timeBuff, buffSize, "%Y-%m-%d %H:%M:%S", tm_info) == 0) {
        g_printerr("Failed to format time string. Buffer size might be too small\n");
        return -1;
    }

    return 0;
}

// Client disconnection callback
void rtsp_client_disconnect(GstRTSPClient *client, gpointer user_data) {
    char timeBuff[30];
    if (get_current_time(timeBuff, sizeof(timeBuff)) == -1) {
        g_printerr("Failed to retrieve timestamp\n");
        return;
    }

    GstRTSPConnection *connection = gst_rtsp_client_get_connection(client);
    if (!connection) {
        g_printerr("Failed client connection");
        return;
    }
    const gchar *ip_address = gst_rtsp_connection_get_ip(connection);
    if (!ip_address) {
        g_printerr("Failed to retrieve IP address from RTSP connection\n");
        return;
    }
    g_message("Client disconnected: IP %s at %s\n", ip_address, timeBuff);
    ClientConnection *conStr = create_client_connection(ip_address, timeBuff);
    if (!conStr) {
        return;
    }
    // DB update connection
    INSERT_DB(conStr, 0);
    // Free memory
    free_client_connection(conStr);
}

// Client connection callback
void rtsp_client_connected(GstRTSPServer *gstrtspserver, GstRTSPClient *client, gpointer user_data) {
    char timeBuff[30];
    if (get_current_time(timeBuff, sizeof(timeBuff)) == -1) {
        g_printerr("Failed to retrieve timestamp\n");
        return;
    }
    GstRTSPConnection *connection = gst_rtsp_client_get_connection(client);
    if (!connection) {
        g_printerr("Failed client connection");
        return;
    }
    const gchar *ip_address = gst_rtsp_connection_get_ip(connection);
    if (!ip_address) {
        g_printerr("Failed to retrieve IP address from RTSP connection\n");
        return;
    }
    g_message("Client connected: IP %s at %s\n", ip_address, timeBuff);
    ClientConnection *conStr = create_client_connection(ip_address, timeBuff);
    if (!conStr) {
        return;
    }
    g_signal_connect(client, "closed", G_CALLBACK(rtsp_client_disconnect), user_data);

    // DB update connection
    INSERT_DB(conStr, 1);
    // Free memory
    free_client_connection(conStr);
}

// Insert DB entry
int INSERT_DB(ClientConnection *client, int status) {
    MYSQL *conn;
    char query[512];
    MYSQL_RES *res;
    MYSQL_ROW row;
    conn = mysql_init(NULL);
    int port = 0;
    if (current_destination_port) {
        char *end;
        errno = 0;
        long l = strtol(current_destination_port, &end, 10);
        if (errno || *end != '\0' || l < INT_MIN || l > INT_MAX) {
        } else {
            port = l;
        }
    }
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        g_printerr("DB error: %s\n", mysql_error(conn));
        return -1;
    }

    snprintf(query, sizeof(query), "SELECT 1 FROM flux_RSTP WHERE IP_address='%s' AND ZP_PORT=%d", client->ip_address, port);
    if (mysql_query(conn, query)) {
        g_printerr("DB error  : %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    res = mysql_store_result(conn);
    const char *dateField = status ? "UP_DATE" : "DOWN_DATE";

    // Check if the IP address exists
    if ((row = mysql_fetch_row(res))) {
        snprintf(query, sizeof(query), "UPDATE flux_RSTP SET %s='%s', count_nbr_of_clients=%d, ZP_PORT=%d WHERE IP_address='%s'",
                 dateField, client->timeBuff, status, port, client->ip_address);
    } else {
        snprintf(query, sizeof(query), "INSERT INTO flux_RSTP (%s, count_nbr_of_clients, IP_address, ZP_PORT) VALUES ('%s', %d, '%s', %d)",
                 dateField, client->timeBuff, status, client->ip_address, port);
    }
    if (mysql_query(conn, query)) {
        g_printerr("DB error: %s\n", mysql_error(conn));
        mysql_free_result(res);
        mysql_close(conn);
        return -1;
    }

    mysql_free_result(res);
    mysql_close(conn);
    return 0;
}

// Create client connection
ClientConnection* create_client_connection(const gchar *ip_address, const char *timeBuff) {
    ClientConnection *conStr = (ClientConnection *)malloc(sizeof(ClientConnection));
    if (!conStr) {
        g_printerr("Failed to allocate memory for ClientConnection\n");
        return NULL;
    }

    conStr->ip_address = strdup(ip_address);
    if (!conStr->ip_address) {
        g_printerr("Failed to allocate memory for IP address\n");
        free(conStr);
        return NULL;
    }

    conStr->timeBuff = strdup(timeBuff);
    if (!conStr->timeBuff) {
        g_printerr("Failed to allocate memory for timeBuff\n");
        free(conStr->ip_address);
        free(conStr);
        return NULL;
    }
    return conStr;
}

void free_client_connection(ClientConnection *conStr) {
    if (conStr) {
        free(conStr->ip_address);
        free(conStr->timeBuff);
        free(conStr);
    }
}
