


    #include <gst/gst.h>
#include <glib.h>

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
	    GMainLoop *loop = (GMainLoop *)data;

	        switch (GST_MESSAGE_TYPE(msg)) {
			        case GST_MESSAGE_EOS:
					            g_print("End of stream\n");
						                g_main_loop_quit(loop);
								            break;

									            case GST_MESSAGE_ERROR: {
														                gchar *debug;
																            GError *error;

																	                gst_message_parse_error(msg, &error, &debug);
																			            g_free(debug);

																				                g_printerr("Error: %s\n", error->message);
																						            g_error_free(error);

																							                g_main_loop_quit(loop);
																									            break;
																										            }

													            default:
													                break;
															    }

		    return TRUE;
}

int main(int argc, char *argv[]) {
	    GMainLoop *loop;
	        GstElement *pipeline, *source, *depay, *decode, *convert, *textoverlay, *sink;
		    GstBus *bus;

		        gst_init(&argc, &argv);

			    loop = g_main_loop_new(NULL, FALSE);

			        // Create GStreamer elements
				//     pipeline = gst_pipeline_new("rtsp-pipeline");
				//         source = gst_element_factory_make("rtspsrc", "source");
				//             depay = gst_element_factory_make("rtph264depay", "depay");
				//                 decode = gst_element_factory_make("avdec_h264", "decode");
				//                     convert = gst_element_factory_make("videoconvert", "convert");
				//                         textoverlay = gst_element_factory_make("textoverlay", "textoverlay");
				//                             sink = gst_element_factory_make("autovideosink", "sink");
				//
				//                                 if (!pipeline || !source || !depay || !decode || !convert || !textoverlay || !sink) {
				//                                         g_printerr("Failed to create elements\n");
				//                                                 return -1;
				//                                                     }
				//
				//                                                         // Set RTSP source properties
				//                                                             g_object_set(G_OBJECT(source), "location", "rtsp://152.228.142.4:5327/video5327", NULL);
				//
				//                                                                 // Set text overlay properties
				//                                                                     g_object_set(G_OBJECT(textoverlay), "text", "Alert", "valignment", 2, "halignment", 2, NULL);
				//
				//                                                                         // Add elements to the pipeline
				//                                                                             gst_bin_add_many(GST_BIN(pipeline), source, depay, decode, convert, textoverlay, sink, NULL);
				//
				//                                                                                 // Link the elements
				//                                                                                     if (!gst_element_link_many(depay, decode, convert, textoverlay, sink, NULL)) {
				//                                                                                             g_printerr("Failed to link elements\n");
				//                                                                                                     gst_object_unref(pipeline);
				//                                                                                                             return -1;
				//                                                                                                                 }
				//
				//                                                                                                                     // Connect signal for the "pad-added" event on rtspsrc
				//                                                                                                                         g_signal_connect(source, "pad-added", G_CALLBACK(gst_element_link_pads_simple), depay);
				//
				//                                                                                                                             // Add bus to monitor messages
				//                                                                                                                                 bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
				//                                                                                                                                     gst_bus_add_watch(bus, bus_call, loop);
				//                                                                                                                                         gst_object_unref(bus);
				//
				//                                                                                                                                             // Start playback
				//                                                                                                                                                 gst_element_set_state(pipeline, GST_STATE_PLAYING);
				//
				//                                                                                                                                                     // Run the main loop
				//                                                                                                                                                         g_print("Running...\n");
				//                                                                                                                                                             g_main_loop_run(loop);
				//
				//                                                                                                                                                                 // Clean up
				//                                                                                                                                                                     gst_element_set_state(pipeline, GST_STATE_NULL);
				//                                                                                                                                                                         gst_object_unref(GST_OBJECT(pipeline));
				//                                                                                                                                                                             g_main_loop_unref(loop);
				//
				//                                                                                                                                                                                 return 0;
				//                                                                                                                                                                                 }
