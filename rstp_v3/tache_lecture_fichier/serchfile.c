#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

// Function to replace all occurrences of 'src' with 'replace' in 'line'
void replace_all(char *line, const char *src, const char *replace) {
    char buffer[MAX_LINE_LENGTH];
    char *pos;
    int src_len = strlen(src);
    int replace_len = strlen(replace);

    buffer[0] = '\0'; // Clear the buffer

    // Iterate through the line and perform replacements
    while ((pos = strstr(line, src)) != NULL) {
        // Copy everything before 'src' in the line to buffer
        strncat(buffer, line, pos - line);

        // Append 'replace' in place of 'src'
        strcat(buffer, replace);

        // Move the line pointer after the occurrence of 'src'
        line = pos + src_len;
    }

    // Append the remaining part of the line after the last occurrence of 'src'
    strcat(buffer, line);

    // Copy the buffer back to line
    strcpy(line, buffer);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <file_name> <search_string> <replace_string>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r+");  // Open file in read+write mode
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    long pos;
    
    // Read and process each line from the file
    while (fgets(line, sizeof(line), file) != NULL) {
        // Replace all occurrences of the search string with the replacement
        replace_all(line, argv[2], argv[3]);

        // Store the current file pointer position
        pos = ftell(file);

        // Move to the start of the line for writing
        fseek(file, pos - strlen(line), SEEK_SET);

        // Write the modified line back to the file
        fputs(line, file);
        
        // Flush the file stream to ensure data is written


        fflush(file);
    }

    fclose(file);
    printf("Replacement complete.\n");

    return 0;
}
