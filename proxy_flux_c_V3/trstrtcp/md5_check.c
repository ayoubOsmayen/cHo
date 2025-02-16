

   #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

// Function to compute MD5 hash of a file
// void compute_md5(const char *filename, unsigned char *md5_hash) {
//     FILE *file = fopen(filename, "rb");
//         if (!file) {
//                 perror("Unable to open file");
//                         exit(1);
//                             }
//
//                                 MD5_CTX md5_ctx;
//                                     MD5_Init(&md5_ctx);
//
//                                         unsigned char buffer[1024];
//                                             size_t bytes;
//                                                 while ((bytes = fread(buffer, 1, sizeof(buffer), file)) != 0) {
//                                                         MD5_Update(&md5_ctx, buffer, bytes);
//                                                             }
//
//                                                                 MD5_Final(md5_hash, &md5_ctx);
//
//                                                                     fclose(file);
//                                                                     }
//
//                                                                     // Function to convert a byte array to a hex string
//                                                                     void bytes_to_hex(unsigned char *md5_hash, char *output) {
//                                                                         for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
//                                                                                 sprintf(output + (i * 2), "%02x", md5_hash[i]);
//                                                                                     }
//                                                                                     }
//
//                                                                                     // Function to read MD5 hash and filename from a file
//                                                                                     int read_md5_from_file(const char *file_name, char *expected_md5, char *actual_file) {
//                                                                                         FILE *file = fopen(file_name, "r");
//                                                                                             if (!file) {
//                                                                                                     perror("Unable to open finalp.txt");
//                                                                                                             return -1;
//                                                                                                                 }
//
//                                                                                                                     // Read the MD5 hash and the filename
//                                                                                                                         if (fscanf(file, "%s %s", expected_md5, actual_file) != 2) {
//                                                                                                                                 perror("Invalid format in finalp.txt");
//                                                                                                                                         fclose(file);
//                                                                                                                                                 return -1;
//                                                                                                                                                     }
//
//                                                                                                                                                         fclose(file);
//                                                                                                                                                             return 0;
//                                                                                                                                                             }
//
//                                                                                                                                                             int main() {
//                                                                                                                                                                 char expected_md5[MD5_DIGEST_LENGTH * 2 + 1]; // Buffer for the expected MD5 hash
//                                                                                                                                                                     char actual_file[256]; // Buffer for the filename
//                                                                                                                                                                         unsigned char computed_md5[MD5_DIGEST_LENGTH]; // Array to store the computed MD5 hash
//                                                                                                                                                                             char computed_md5_str[MD5_DIGEST_LENGTH * 2 + 1]; // String to hold the computed MD5 hash as a hex string
//
//                                                                                                                                                                                 // Step 1: Read MD5 hash and filename from finalp.txt
//                                                                                                                                                                                     if (read_md5_from_file("finalp.txt", expected_md5, actual_file) != 0) {
//                                                                                                                                                                                             return 1;
//                                                                                                                                                                                                 }
//
//                                                                                                                                                                                                     printf("Expected MD5: %s\n", expected_md5);
//                                                                                                                                                                                                         printf("File to check: %s\n", actual_file);
//
//                                                                                                                                                                                                             // Step 2: Compute MD5 hash of the file
//                                                                                                                                                                                                                 compute_md5(actual_file, computed_md5);
//
//                                                                                                                                                                                                                     // Step 3: Convert the computed MD5 hash to a hex string
//                                                                                                                                                                                                                         bytes_to_hex(computed_md5, computed_md5_str);
//                                                                                                                                                                                                                             computed_md5_str[MD5_DIGEST_LENGTH * 2] = '\0'; // Null-terminate the string
//
//                                                                                                                                                                                                                                 // Step 4: Compare the expected MD5 hash with the computed one
//                                                                                                                                                                                                                                     if (strcmp(expected_md5, computed_md5_str) == 0) {
//                                                                                                                                                                                                                                             printf("The MD5 hash matches!\n");
//                                                                                                                                                                                                                                                 } else {
//                                                                                                                                                                                                                                                         printf("The MD5 hash does NOT match!\n");
//                                                                                                                                                                                                                                                             }
//
//                                                                                                                                                                                                                                                                 return 0;
//                                                                                                                                                                                                                                                                 }
//
