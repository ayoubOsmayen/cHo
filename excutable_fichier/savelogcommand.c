a#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_OUTPUT_LENGTH 1024
#define LOG_FILE "command_log.txt"

/**
 * Structure to hold thread arguments.
 */
typedef struct {
    char command[MAX_COMMAND_LENGTH];
} ThreadArgs;

/**
 * Function to execute a shell command and return the output.
 * @param command: The shell command to execute.
 * @param output: Buffer to store the output of the command.
 * @return: 0 on success, -1 on failure.
 */
int executeCommand(const char *command, char *output) {
    FILE *fp;
    char buffer[128];

    // Open the command for reading
    fp = popen(command, "r");
    if (fp == NULL) {
        perror("popen failed");
        return -1;
    }

    // Read the output of the command line by line
    output[0] = '\0'; // Ensure the output string is empty
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncat(output, buffer, MAX_OUTPUT_LENGTH - strlen(output) - 1);
    }

    // Close the file pointer
    if (pclose(fp) == -1) {
        perror("pclose failed");
        return -1;
    }

    return 0;
}

/**
 * Thread function to handle command execution.
 */
void *threadHandler(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    char output[MAX_OUTPUT_LENGTH];

    // Log file to save results
    FILE *logFile = fopen(LOG_FILE, "a");
    if (logFile == NULL) {
        perror("Failed to open log file");
        pthread_exit(NULL);
    }

    // Get process and thread IDs
    pid_t processID = getpid();
    pthread_t threadID = pthread_self();

    // Execute the command
    if (executeCommand(threadArgs->command, output) == 0) {
        fprintf(logFile, "Process ID: %d, Thread ID: %lu\n", processID, threadID);
        fprintf(logFile, "Command: %s\n", threadArgs->command);
        fprintf(logFile, "Output:\n%s\n", output);
        printf("Command executed successfully. Output logged.\n");
    } else {
        fprintf(logFile, "Process ID: %d, Thread ID: %lu\n", processID, threadID);
        fprintf(logFile, "Command: %s\n", threadArgs->command);
        fprintf(logFile, "Failed to execute command.\n");
        printf("Failed to execute command. See log for details.\n");
    }

    fclose(logFile);
    pthread_exit(NULL);
}

int main() {
    char command[MAX_COMMAND_LENGTH];
    printf("Enter a shell command (or type 'exit' to quit):\n");

    while (1) {
        // Read command from the user
        printf("> ");
        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            perror("fgets failed");
            break;
        }

        // Remove the newline character
        command[strcspn(command, "\n")] = '\0';

        // Check for exit condition
        if (strcmp(command, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Create thread arguments
        ThreadArgs *args = malloc(sizeof(ThreadArgs));
        if (args == NULL) {
            perror("Failed to allocate memory for thread arguments");
            continue;
        }
        strncpy(args->command, command, MAX_COMMAND_LENGTH);

        // Create a new thread
        pthread_t thread;
        if (pthread_create(&thread, NULL, threadHandler, args) != 0) {
            perror("Failed to create thread");
            free(args);
            continue;
        }

        // Detach the thread so resources are automatically released
        pthread_detach(thread);
    }

    return 0;
}
