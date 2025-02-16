#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "function.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    Config config;
    read_configv1(argv[1], &config);

    // Start GStreamer pipeline
    init_gstreamer(&config);


     int exit_  = -1; 
     
    printf("enter key to quit " ) ;
    scanf("%d", &exit_ ) ;
    if(exit_ !=-1)
    return EXIT_SUCCESS;
}
