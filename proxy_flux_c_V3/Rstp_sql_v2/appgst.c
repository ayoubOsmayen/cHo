#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <glib.h>

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
static gchar *initial_text;
Node *head = NULL;
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void add_node(int val, int vid_id, const char *condition);
void update_text_overlay(const char *prefix);
void switch_camera(int vid_id);

typedef struct Config {
    const char *welcome_message;
    const char *stream_url;
    const char *stream_resolution;
    const char *stream_bitrate;
    const char *stream_codec;
    const char *user_messages[2];
} Config;

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
            char pad_name[16];
            snprintf(pad_name, sizeof(pad_name), "sink_%d", current->val);

            GstPad *new_active_pad = gst_element_get_request_pad(input_selector, pad_name);
            if (new_active_pad) {
                g_object_set(G_OBJECT(input_selector), "active-pad", new_active_pad, NULL);
                gst_object_unref(new_active_pad);
                update_text_overlay(current->condition);
                g_print("Switched to camera: %d\n", vid_id);
            } else {
                g_printerr("Failed to get request pad: %s\n", pad_name);
            }
            break;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&list_mutex);
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    if (argc != 2) {
        g_printerr("Usage: %s <initial_text>\n", argv[0]);
        return -1;
    }

    initial_text = g_strdup(argv[1]);
    loop = g_main_loop_new(NULL, FALSE);

    // Define the embedded configuration
    Config config = {
        .welcome_message = "Welcome to the RTSP Server!",
        .stream_url = "rtsp://127.0.0.1:8554",
        .stream_resolution = "1920x1080",
        .stream_bitrate = "4000kbps",
        .stream_codec = "H264",
        .user_messages = {
            "Enjoy the high-quality stream!",
            "Stay tuned for more updates."
        }
    };

    // Print the configuration for verification
    g_print("%s\n", config.welcome_message);
    g_print("Streaming URL: %s\n", config.stream_url);
    g_print("Stream Resolution: %s\n", config.stream_resolution);
    g_print("Stream Bitrate: %s\n", config.stream_bitrate);
    g_print("Stream Codec: %s\n", config.stream_codec);
    g_print("User Message 1: %s\n", config.user_messages[0]);
    g_print("User Message 2: %s\n", config.user_messages[1]);

    // Initialize linked list with dummy data
    add_node(0, 1, "Stream 1");
    add_node(1, 2, "Stream 2");
    add_node(2, 3, "Stream 3");

    g_print("RTSP Server running...\n");
    g_main_loop_run(loop);

    g_free(initial_text);
    return 0;
}
