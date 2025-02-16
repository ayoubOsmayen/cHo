#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_OUTPUT_LENGTH 1024

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
    output[0] = '\0';  // Ensure the output string is empty
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

int main() {
    char command[MAX_COMMAND_LENGTH];
    char output[MAX_OUTPUT_LENGTH];

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

        // Execute the command and print the output
        if (executeCommand(command, output) == 0) {
            printf("Command Output:\n%s\n", output);
        } else {
            printf("Failed to execute command: %s\n", command);
        }
    }

    return 0;
}
