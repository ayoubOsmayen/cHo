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

/****************
void update_text_overlay(const char *prefix) {
    if (text_overlay) {
        if (strcmp(prefix, "REC") == 0) { // Vérifier si le préfixe est "REC"
            g_object_set(G_OBJECT(text_overlay), "text", "REC", NULL); // Afficher seulement "REC"
   } else if (strcmp(prefix, "ALARME") == 0) { // Vérifier si le préfixe est "ALARME"
            g_object_set(G_OBJECT(text_overlay), "text", initial_text, NULL); // Afficher seulement le texte initial
        } else {
            gchar *final_text = g_strdup_printf("%s : %s", prefix ? prefix : "", initial_text);
            g_object_set(G_OBJECT(text_overlay), "text", final_text, NULL);
            g_free(final_text);
        }
         g_object_set(G_OBJECT(text_overlay), "silent", TRUE, NULL); // Désactiver l'overlay pour les autres codes

  } else {
        g_printerr("L'élément text_overlay n'est pas initialisé.\n");
    }
}
***********/


void update_text_overlay(const char *prefix) {
    if (text_overlay) {
        gchar *final_text = NULL;

        // Logique pour afficher les textes selon le code
        if (g_strcmp0(prefix, "REC") == 0) {
            // Si le préfixe est "REC", on affiche seulement "REC"
            final_text = g_strdup("REC");
        } else if (g_strcmp0(prefix, "ALARME") == 0) {
            // Si le préfixe est "ALARME", on affiche uniquement le texte initial
            final_text = g_strdup(initial_text);
        } else {
            // Pour tous les autres cas, on affiche le texte complet (préfixe + texte initial)
            final_text = g_strdup_printf("%s : %s", prefix, initial_text);
        }

        // Met à jour le texte sur l'overlay
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

        // Met à jour le texte de l'overlay
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

// Fonction pour gérer les connexions des clients
void on_client_connected(GstRTSPServer *server, GstRTSPClient *client, gpointer user_data) {
    g_print("Client connecté: %p\n", client);

    // Connexion des signaux sur le client
    g_signal_connect(client, "closed", G_CALLBACK(on_client_disconnected), NULL);
}

// Fonction pour gérer les déconnexions des clients
void on_client_disconnected(GstRTSPClient *client, gpointer user_data) {
    g_print("Client déconnecté: %p\n", client);
}

// Fonction de gestion du bus pour redémarrer la lecture en cas de fin de flux
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

// Fonction pour démarrer la lecture après un délai
static gboolean start_playback(gpointer user_data) {
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    return FALSE; // Supprimer le timeout après exécution
}

// Création du serveur RTSP
static void create_rtsp_server(const char *rtsp_url, const char *file_uri, const char *port) {
    g_print("Création du serveur RTSP...\n");
    server = gst_rtsp_server_new();

    if (!server) {
        g_printerr("Échec de la création du serveur RTSP\n");
        return;
    }

    // Modifier le port ici
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
//        "( input-selector name=input_selector ! textoverlay name=text_overlay text=\"%s\" font-desc=\"Sans, 32\" ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96 "
         "( input-selector name=input_selector ! textoverlay name=text_overlay text=\"%s\" font-desc=\"Sans, 28\" ! videoconvert ! x264enc bitrate=2048 speed-preset=ultrafast tune=zerolatency ! rtph264pay name=pay0 pt=96 "
        "uridecodebin uri=file:///home/aymen/Video_source/mire.mp4 ! queue ! input_selector.sink_0 "
        "uridecodebin uri=file:///home/aymen/Video_source/fixe2.mp4 ! queue ! input_selector.sink_1 "
        "uridecodebin uri=file:///home/aymen/Video_source/norec.mp4 ! queue ! input_selector.sink_2 "
        "uridecodebin uri=file:///home/aymen/Video_source/nolive.mp4 ! queue ! input_selector.sink_3 "
        "uridecodebin uri=file:///home/aymen/Video_source/norecnolive.mp4 ! queue ! input_selector.sink_4 "
        "uridecodebin uri=%s ! queue ! input_selector.sink_5 "
  "rtspsrc location=%s ! rtpjitterbuffer ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! queue ! input_selector.sink_6 " 
      ")",
        initial_text, file_uri, rtsp_url);

    gst_rtsp_media_factory_set_launch(factory, launch_string);
    g_free(launch_string);

    // Générer le chemin d'URL dynamique basé sur le port
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

// Fonction de configuration des médias
static void on_media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    g_print("Configuration des médias...\n");
    pipeline = gst_rtsp_media_get_element(media);

    input_selector = gst_bin_get_by_name(GST_BIN(pipeline), "input_selector");
    text_overlay = gst_bin_get_by_name(GST_BIN(pipeline), "text_overlay");

    if (!input_selector || !text_overlay) {
        g_printerr("Impossible de récupérer les éléments input_selector ou text_overlay du pipeline.\n");
        return;
    }

    // Ajoute le bus pour écouter les messages
    GstBus *bus = gst_element_get_bus(pipeline);
    gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    // Lire le fichier pour déterminer la caméra initiale
    FILE *file = fopen(file_path, "r");
    if (file) {
        char key = fgetc(file);
        fclose(file);
        if (key >= '1' && key <= '7') {
            switch_camera(key);
        } else {
            switch_camera('1');  // Par défaut, caméra 1
        }
    } else {
        switch_camera('1');  // Par défaut, caméra 1
    }

    // Ajouter un délai avant de démarrer la lecture pour s'assurer que tout est prêt
    g_timeout_add_seconds(1, (GSourceFunc)start_playback, NULL);

    g_print("Médias configurés : input_selector et text_overlay trouvés.\n");
}

// Fonction pour lire le fichier de configuration
static gboolean read_config_file(const char *config_file, gchar **rtsp_url, gchar **file_uri, gchar **port, gchar **text) {
    FILE *file = fopen(config_file, "r");
    if (!file) {
        g_printerr("Erreur lors de l'ouverture du fichier de configuration : %s\n", config_file);
        return FALSE;
    }

    // Lire les 4 lignes du fichier
    size_t len = 0;
    char *line = NULL;

    if (getline(&line, &len, file) != -1) *rtsp_url = g_strdup(g_strstrip(line));
    if (getline(&line, &len, file) != -1) *file_uri = g_strdup(g_strstrip(line));
    if (getline(&line, &len, file) != -1) *port = g_strdup(g_strstrip(line));
    if (getline(&line, &len, file) != -1) *text = g_strdup(g_strstrip(line));

    free(line);
    fclose(file);

    if (!(*rtsp_url && *file_uri && *port && *text)) {
        g_printerr("Le fichier de configuration est incomplet ou incorrect.\n");
        return FALSE;
    }

    return TRUE;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        g_printerr("Usage: %s <Config_File> <File_Code>\n", argv[0]);
        return -1;
    }

    gchar *rtsp_url = NULL, *file_uri = NULL, *port = NULL, *text = NULL;

    // Lire le fichier de configuration
    if (!read_config_file(argv[1], &rtsp_url, &file_uri, &port, &text)) {
        return -1;
    }

    gst_init(&argc, &argv);

    loop = g_main_loop_new(NULL, FALSE);
    if (!loop) {
        g_printerr("Impossible de créer la boucle principale\n");
        return -1;
    }

    file_path = argv[2]; // Fichier à surveiller
    initial_text = text; // Texte à afficher

    // Créer le serveur RTSP avec les URL fournies
    create_rtsp_server(rtsp_url, file_uri, port);

    if (!server || !factory) {
        g_printerr("Échec de l'initialisation du serveur RTSP\n");
        return -1;
    }

    // Connecter le signal "media-configure" pour configurer le pipeline
    g_signal_connect(factory, "media-configure", (GCallback)on_media_configure, NULL);

    // Gérer les connexions des clients
    g_signal_connect(server, "client-connected", G_CALLBACK(on_client_connected), NULL);

    // Commencer à vérifier les mises à jour de code toutes les secondes
    g_timeout_add_seconds(1, check_file_update, NULL);

    // Gestion du signal SIGINT pour arrêter proprement le serveur
    g_unix_signal_add(SIGINT, (GSourceFunc)stop_server, loop);

    g_main_loop_run(loop);

    // Nettoyage des ressources
    g_main_loop_unref(loop);
    if (pipeline) gst_object_unref(pipeline);
    if (input_selector) gst_object_unref(input_selector);
    if (text_overlay) gst_object_unref(text_overlay);
    if (server) g_object_unref(server);
    if (factory) g_object_unref(factory);
    if (rtsp_url) g_free(rtsp_url);
    if (file_uri) g_free(file_uri);
    if (port) g_free(port);
    if (text) g_free(text);

    return 0;
}

