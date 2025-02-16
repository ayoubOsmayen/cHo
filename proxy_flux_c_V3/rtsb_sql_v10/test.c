#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <glib.h>
#include <glib-unix.h>
#include <signal.h>
#include <pthread.h>

static GMainLoop *loop;
static GstElement *pipeline, *input_selector, *text_overlay;
static GstRTSPServer *server;
static GstRTSPMediaFactory *factory;
static const char *file_path;
static gchar *initial_text;

// Déclaration de la fonction on_client_disconnected
void on_client_disconnected(GstRTSPClient *client, gpointer user_data);

// Fonction pour arrêter proprement le serveur RTSP
static gboolean stop_server(gpointer data) {
    g_print("Arrêt du serveur...\n");
    g_main_loop_quit(loop);
    return G_SOURCE_REMOVE;
}

void update_text_overlay(const char *prefix) {
    if (text_overlay) {
        gchar *final_text = NULL;

        if (g_strcmp0(prefix, "REC") == 0) {
            final_text = g_strdup("REC");
        } else if (g_strcmp0(prefix, "ALARME") == 0) {
            final_text = g_strdup(initial_text);
        } else {
            final_text = g_strdup_printf("%s : %s", prefix, initial_text);
        }

        g_object_set(G_OBJECT(text_overlay), "text", final_text, NULL);
        g_free(final_text);
    } else {
        g_printerr("L'élément text_overlay n'est pas initialisé.\n");
    }
}

void switch_camera(char key) {
    GstPad *new_active_pad = NULL;
    const char *prefix = NULL;

    if (!input_selector) {
        g_printerr("Input selector element is not initialized.\n");
        return;
    }

    switch(key) {
        case '1':
            new_active_pad = gst_element_get_static_pad(input_selector, "sink_0");
            prefix = "ON";
            break;
        case '2':
            new_active_pad = gst_element_get_static_pad(input_selector, "sink_1");
            prefix = "ALARME";
            break;
        case '3':
            new_active_pad = gst_element_get_static_pad(input_selector, "sink_2");
            prefix = "NO REC";
            break;
        case '4':
            new_active_pad = gst_element_get_static_pad(input_selector, "sink_3");
            prefix = "NO LIVE";
            break;
        case '5':
            new_active_pad = gst_element_get_static_pad(input_selector, "sink_4");
            prefix = "NO REC NO LIVE";
            break;
        case '6':
            new_active_pad = gst_element_get_static_pad(input_selector, "sink_5");
            prefix = "REC";
            break;
        case '7':
            new_active_pad = gst_element_get_static_pad(input_selector, "sink_6");
            prefix = "LIVE";
            break;
        default:
            g_printerr("Clé invalide: %c\n", key);
            return;
    }

    if (new_active_pad) {
        g_object_set(G_OBJECT(input_selector), "active-pad", new_active_pad, NULL);
        gst_object_unref(new_active_pad); // Libère la référence après utilisation
        update_text_overlay(prefix);
        g_print("Changement de caméra vers %c\n", key);
    } else {
        g_printerr("Impossible de récupérer le pad actif pour la caméra %c\n", key);
    }
}

// Fonction pour vérifier les mises à jour du fichier
gboolean check_file_update(gpointer user_data) {
    static time_t last_mod_time = 0;
    struct stat file_stat;

    if (stat(file_path, &file_stat) == -1) {
        perror("Erreur lors de l'accès au fichier");
        return TRUE; // Continue polling
    }

    if (file_stat.st_mtime > last_mod_time) {
        last_mod_time = file_stat.st_mtime;

        FILE *file = fopen(file_path, "r");
        if (!file) {
            perror("Erreur lors de l'ouverture du fichier");
            return TRUE; // Continue polling
        }

        char key = fgetc(file);
        fclose(file);

        if (key >= '1' && key <= '7') {
            switch_camera(key);
        }
    }

    return TRUE; // Continue polling
}

// Fonction pour démarrer un thread qui vérifie les mises à jour du fichier
gpointer file_check_thread(gpointer data) {
    while (1) {
        check_file_update(NULL);
        g_usleep(1000000); // Sleep for 1 second
    }
    return NULL;
}

void on_client_connected(GstRTSPServer *server, GstRTSPClient *client, gpointer user_data) {
    g_print("Client connecté: %p\n", client);
    g_signal_connect(client, "closed", G_CALLBACK(on_client_disconnected), NULL);
}

void on_client_disconnected(GstRTSPClient *client, gpointer user_data) {
    g_print("Client déconnecté: %p\n", client);
}

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_print("Fin du flux, redémarrage...\n");
            gst_element_set_state(pipeline, GST_STATE_READY);
            gst_element_set_state(pipeline, GST_STATE_PLAYING);
            break;

        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;

            gst_message_parse_error(msg, &error, &debug);
            g_printerr("Erreur : %s\n", error->message);
            g_error_free(error);
            g_free(debug);
            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

static gboolean start_playback(gpointer user_data) {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return FALSE; 
}

static void create_rtsp_server(const char *rtsp_url, const char *file_uri, const char *port) {
    g_print("Création du serveur RTSP...\n");
    server = gst_rtsp_server_new();

    if (!server) {
        g_printerr("Échec de la création du serveur RTSP\n");
        return;
    }

    gst_rtsp_server_set_service(server, port);

    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    if (!mounts) {
        g_printerr("Impossible de récupérer les points de montage RTSP\n");
        return;
    }

    factory = gst_rtsp_media_factory_new();
    if (!factory) {
        g_printerr("Impossible de créer la factory RTSP\n");
        g_object_unref(mounts);
        return;
    }

    gchar *launch_string = g_strdup_printf(
        "( input-selector name=input_selector ! textoverlay name=text_overlay text=\"%s\" font-desc=\"Sans, 28\" ! videoconvert ! x264enc bitrate=2048 speed-preset=ultrafast tune=zerolatency ! rtph264pay name=pay0 pt=96 "
        "uridecodebin uri=file:///home/aymen/Video_source/mire.mp4 ! queue ! input_selector.sink_0 "
        "uridecodebin uri=file:///home/aymen/Video_source/fixe2.mp4 ! queue ! input_selector.sink_1 "
        "uridecodebin uri=file:///home/aymen/Video_source/norec.mp4 ! queue ! input_selector.sink_2 "
        "uridecodebin uri=file:///home/aymen/Video_source/nolive.mp4 ! queue ! input_selector.sink_3 "
        "uridecodebin uri=file:///home/aymen/Video_source/norecnolive.mp4 ! queue ! input_selector.sink_4 "
        "uridecodebin uri=%s ! queue ! input_selector.sink_5 "
        "rtspsrc location=%s ! rtpjitterbuffer ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! queue ! input_selector.sink_6 )",
        initial_text, file_uri, rtsp_url);

    gst_rtsp_media_factory_set_launch(factory, launch_string);
    g_free(launch_string);

    gchar *mount_path = g_strdup_printf("/video%s", port);
    gst_rtsp_mount_points_add_factory(mounts, mount_path, factory);
    g_free(mount_path);

    g_object_unref(mounts);

    if (gst_rtsp_server_attach(server, NULL) == 0) {
        g_printerr("Échec de l'attachement du serveur RTSP\n");
    } else {
        g_print("Serveur RTSP en cours d'exécution à rtsp://127.0.0.1:%s/video%s\n", port, port);
    }
}

static void on_media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    g_print("Configuration des médias...\n");
    pipeline = gst_rtsp_media_get_element(media);

    input_selector = gst_bin_get_by_name(GST_BIN(pipeline), "input_selector");
    text_overlay = gst_bin_get_by_name(GST_BIN(pipeline), "text_overlay");

    GstBus *bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    if (argc != 4) {
        g_printerr("Usage: %s <initial_text> <file_path> <rtsp_url>\n", argv[0]);
        return -1;
    }

    initial_text = g_strdup(argv[1]);
    file_path = argv[2];
    const char *rtsp_url = argv[3];

    loop = g_main_loop_new(NULL, FALSE);

    create_rtsp_server(rtsp_url, "video_source", "8554");

    // Start the thread for checking file updates
    GThread *thread = g_thread_new("file_check_thread", file_check_thread, NULL);

    g_print("Démarrage du serveur RTSP...\n");
    g_timeout_add_seconds(1, start_playback, NULL);
    g_main_loop_run(loop);

    g_thread_join(thread);
    g_free(initial_text);

    return 0;
}
