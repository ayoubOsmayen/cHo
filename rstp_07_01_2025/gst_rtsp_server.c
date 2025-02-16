

   #include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>

static GstRTSPServer *server = NULL;
static GstRTSPMediaFactory *factory = NULL;
static GstElement *pipeline = NULL;

static gboolean
bus_callback (GstBus * bus, GstMessage * msg, gpointer data)
{
	  GError *err;
	    gchar *debug_info;

	      switch (GST_MESSAGE_TYPE (msg)) {
		          case GST_MESSAGE_ERROR:
				        gst_message_parse_error (msg, &err, &debug_info);
					      g_printerr ("Error: %s\n", err->message);
					            g_error_free (err);
						          g_free (debug_info);
							        return FALSE;
								    case GST_MESSAGE_EOS:
								      g_print ("End of stream\n");
								            return FALSE;
									        default:
									          break;
										    }
	        return TRUE;
}

int
main (int argc, char *argv[])
{
	  GstBus *bus;
	    GMainLoop *loop;

	      /* Initialize GStreamer */
	      gst_init (&argc, &argv);

	        /* Create the RTSP server */
	        server = gst_rtsp_server_new ();
		  g_object_set (server, "service", "8554", NULL); // RTSP Server on port 8554

		    /* Create a media factory */
		    factory = gst_rtsp_media_factory_new ();

		      /* Create a pipeline (this is a simple test pipeline) */
		      pipeline = gst_parse_launch ("videotestsrc ! x264enc ! rtph264pay name=pay0 pt=96", NULL);

		        /* Attach the pipeline to the factory */
		        gst_rtsp_media_factory_set_launch (factory, "videotestsrc ! x264enc ! rtph264pay name=pay0 pt=96");

			  /* Attach the factory to the server */
			  gst_rtsp_server_attach (server, NULL);

			    /* Start streaming */
			    g_print ("RTSP server is running on rtsp://127.0.0.1:8554/test\n");

			      /* Start the event loop */
			      loop = g_main_loop_new (NULL, FALSE);
			        bus = gst_element_get_bus (GST_ELEMENT (server));
				  gst_bus_add_watch (bus, bus_callback, loop);
				    g_main_loop_run (loop);

				      /* Cleanup */
				      g_object_unref (server);
				        g_main_loop_unref (loop);

					  return 0;
}

