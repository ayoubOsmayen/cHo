#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *inputselector, *textoverlay, *sink;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);

    // Cr�er le pipeline
    pipeline = gst_pipeline_new("video-pipeline");

    // Cr�er les �l�ments
    source = gst_element_factory_make("videotestsrc", "source");
    inputselector = gst_element_factory_make("inputselector", "inputselector");
    textoverlay = gst_element_factory_make("textoverlay", "textoverlay");
    sink = gst_element_factory_make("autovideosink", "sink");

    if (!pipeline || !source || !inputselector || !textoverlay || !sink) {
        g_printerr("�chec de la cr�ation des �l�ments\n");
        return -1;
    }

    // Configurer le texte overlay
    g_object_set(textoverlay, "text", "Overlay Dynamique", NULL);

    // Ajouter les �l�ments au pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, inputselector, textoverlay, sink, NULL);

    // Lier les �l�ments
    gst_element_link(source, inputselector);
    gst_element_link(inputselector, textoverlay);
    gst_element_link(textoverlay, sink);

    // D�marrer l'ex�cution du pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Attendre un message d'arr�t
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_poll(bus, GST_MESSAGE_ERROR | GST_MESSAGE_EOS, GST_CLOCK_TIME_NONE);
    if (msg != NULL) {
        gst_message_unref(msg);
    }

    // Nettoyer
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}