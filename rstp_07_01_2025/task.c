

    #include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOG_FILE "execution_log.txt"

// Function to get current timestamp
// void get_current_time(char *time_str) {
//     time_t rawtime;
//         struct tm *timeinfo;
//             time(&rawtime);
//                 timeinfo = localtime(&rawtime);
//                     strftime(time_str, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
//                     }
//
//                     // Function to log task details to a log file
//                     void log_execution_details(const char *task_name, double execution_time) {
//                         FILE *log_file = fopen(LOG_FILE, "a");
//                             if (log_file == NULL) {
//                                     printf("Error opening log file.\n");
//                                             exit(1);
//                                                 }
//
//                                                     char timestamp[20];
//                                                         get_current_time(timestamp);
//
//                                                             fprintf(log_file, "Task Name: %s\n", task_name);
//                                                                 fprintf(log_file, "Execution Time: %.2f seconds\n", execution_time);
//                                                                     fprintf(log_file, "Execution Date: %s\n", timestamp);
//                                                                         fprintf(log_file, "-----------------------------------\n");
//
//                                                                             fclose(log_file);
//                                                                             }
//
//                                                                             // Function to perform the task
//                                                                             void perform_task(const char *task_name) {
//                                                                                 // Start time
//                                                                                     clock_t start_time = clock();
//
//                                                                                         // Simulate some task (replace with real task logic)
//                                                                                             printf("Performing task: %s\n", task_name);
//                                                                                                 for (int i = 0; i < 100000000; ++i);  // Simulating delay
//
//                                                                                                     // End time
//                                                                                                         clock_t end_time = clock();
//
//                                                                                                             // Calculate execution time in seconds
//                                                                                                                 double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
//
//                                                                                                                     // Log the execution details
//                                                                                                                         log_execution_details(task_name, execution_time);
//                                                                                                                         }
//
//                                                                                                                         int main() {
//                                                                                                                             // Example: Perform task and log details
//                                                                                                                                 const char *task_name = "Sample Task";
//
//                                                                                                                                     while (1) {
//                                                                                                                                             // Perform task and log execution time
//                                                                                                                                                     perform_task(task_name);
//
//                                                                                                                                                             // Wait for a period (e.g., 10 seconds) before performing the task again
//                                                                                                                                                                     printf("Waiting for next execution...\n");
//                                                                                                                                                                             sleep(10); // Sleep for 10 seconds (replace with auto period calculation logic if needed)
//                                                                                                                                                                                 }
//
//                                                                                                                                                                                     return 0;
//                                                                                                                                                                                     }
//
