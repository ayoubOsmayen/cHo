/* function.h */
#ifndef FUNCTION_H
#define FUNCTION_H

#include <gst/gst.h>
#include "config.h"

void init_gstreamer(Config *config);
void *client_handler(void *arg);
void save_to_mysql(const char *ip, int session_id, int bandwidth);

#endif
