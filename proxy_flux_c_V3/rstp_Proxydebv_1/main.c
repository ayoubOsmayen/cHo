#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include "functions.h"



int main (int argc , const char *argv[])


{


    gst_init(NULL, NULL);






    pthread_t thread_id ;


    printf("before threading") ; 
    run(argc ,argv);

    
    char _m = getch();
    if _m == '0'  ||  _m == 'e' exit(5);
    sleep(50);



    pthread_create(&thread_id,NULL,serverThread ,NULL);
    pthread_join(thread_id,NULL);

    printf("exit ");

    sleep (100);

    exit(1);



 

   

    return 0;
}