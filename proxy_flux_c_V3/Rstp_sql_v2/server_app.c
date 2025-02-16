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
#include <pthread.h>

typedef struct Node {
    int val;
    int vid_id;
    int status[7];
    char condition[120];
    struct Node *next;
    struct Node *previous;
} Node;

static GMainLoop *loop;
static GstElement *pipeline, *input_selector, *text_overlay;
static GstRTSPServer *server;
static GstRTSPMediaFactory *factory;
static const char *file_path;
static gchar *initial_text;
Node *head = NULL;
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void add_node(int val, int vid_id, const char *condition);
void update_text_overlay(const char *prefix);
void switch_camera(int vid_id);
gpointer file_check_thread(gpointer data);

void add_node(int val, int vid_id, const char *condition) {
    pthread_mutex_lock(&list_mutex);
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->val = val;
    new_node->vid_id = vid_id;
    strncpy(new_node->condition, condition, sizeof(new_node->condition));
    new_node->next = head;
    new_node->previous = NULL;
    if (head) {
        head->previous = new_node;
    }
    head = new_node;
    pthread_mutex_unlock(&list_mutex);
}

void update_text_overlay(const char *prefix) {
    if (text_overlay) {
        gchar *final_text = g_strdup_printf("%s : %s", prefix, initial_text);
        g_object_set(G_OBJECT(text_overlay), "text", final_text, NULL);
        g_free(final_text);
    }
}

void switch_camera(int vid_id) {
    pthread_mutex_lock(&list_mutex);
    Node *current = head;
    while (current) {
        if (current->vid_id == vid_id) {
            GstPad *new_active_pad = gst_element_get_request_pad(input_selector, "sink_%d", current->val);
            if (new_active_pad) {
                g_object_set(G_OBJECT(input_selector), "active-pad", new_active_pad, NULL);
                gst_object_unref(new_active_pad);
                update_text_overlay(current->condition);
                g_print("Switched to camera: %d\n", vid_id);
            }
            break;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&list_mutex);
}

gpointer file_check_thread(gpointer data) {
    while (1) {
        static time_t last_mod_time = 0;
        struct stat file_stat;

        if (stat(file_path, &file_stat) == -1) {
            perror("Error accessing file");
            g_usleep(1000000);
            continue;
        }

        if (file_stat.st_mtime > last_mod_time) {
            last_mod_time = file_stat.st_mtime;

            FILE *file = fopen(file_path, "r");
            if (!file) {
                perror("Error opening file");
                g_usleep(1000000);
                continue;
            }

            int vid_id;
            fscanf(file, "%d", &vid_id);
            fclose(file);

            switch_camera(vid_id);
        }

        g_usleep(1000000);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    if (argc != 3) {
        g_printerr("Usage: %s <initial_text> <file_path>\n", argv[0]);
        return -1;
    }

    initial_text = g_strdup(argv[1]);
    file_path = argv[2];
    loop = g_main_loop_new(NULL, FALSE);

    // Initialize linked list with dummy data
    add_node(0, 1, "Stream 1");
    add_node(1, 2, "Stream 2");
    add_node(2, 3, "Stream 3");

    // Start file check thread
    pthread_t thread;
    pthread_create(&thread, NULL, file_check_thread, NULL);

    g_print("RTSP Server running...\n");
    g_main_loop_run(loop);

    pthread_join(thread, NULL);
    g_free(initial_text);
    return 0;
}
