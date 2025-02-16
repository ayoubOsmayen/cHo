#include <sys/inotify.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define CONFIG_FILE "conf.txt"
#define RTSP_EXECUTABLE "./rtspprdynalic"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

pthread_t monitor_thread;
volatile sig_atomic_t keep_running = 1;

// Signal handler for clean exit
void handle_sigterm(int sig) {
    keep_running = 0;
}

// Function to monitor configuration file changes
void *monitor_config(void *arg) {
    int fd, wd;
    char buffer[BUF_LEN];

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init");
        return NULL;
    }

    wd = inotify_add_watch(fd, CONFIG_FILE, IN_MODIFY);
    if (wd < 0) {
        perror("inotify_add_watch");
        close(fd);
        return NULL;
    }

    while (keep_running) {
        int length = read(fd, buffer, BUF_LEN);
        if (length < 0) {
            perror("read");
            break;
        }

        int i = 0;
        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->mask & IN_MODIFY) {
                printf("Configuration file modified. Restarting RTSP server...\n");
                system("pkill rtsp_signal_server");
                if (system(RTSP_EXECUTABLE " " CONFIG_FILE " &") != 0) {
                    perror("Failed to restart RTSP server");
                }
            }
            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
    return NULL;
}

// Main function
int main() {
    signal(SIGINT, handle_sigterm);
    signal(SIGTERM, handle_sigterm);

    printf("Monitoring configuration file '%s' for changes...\n", CONFIG_FILE);

    if (pthread_create(&monitor_thread, NULL, monitor_config, NULL) != 0) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    pthread_join(monitor_thread, NULL);
    printf("Exiting...\n");

    return EXIT_SUCCESS;
}
