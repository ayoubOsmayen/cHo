gcc -o rtspserver rtspserver.c $(pkg-config --cflags --libs gstreamer-1.0 gstreamer-rtsp-server-1.0 glib-2.0) -lpthread
