#include <gst/gst.h>

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *inputselector, *textoverlay, *sink;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);

    // Créer le pipeline
    pipeline = gst_pipeline_new("video-pipeline");

    // Créer les éléments
    source = gst_element_factory_make("videotestsrc", "source");
    inputselector = gst_element_factory_make("inputselector", "inputselector");
    textoverlay = gst_element_factory_make("textoverlay", "textoverlay");
    sink = gst_element_factory_make("autovideosink", "sink");

    if (!pipeline || !source || !inputselector || !textoverlay || !sink) {
        g_printerr("Échec de la création des éléments\n");
        return -1;
    }

    // Configurer le texte overlay
    g_object_set(textoverlay, "text", "Overlay Dynamique", NULL);

    // Ajouter les éléments au pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, inputselector, textoverlay, sink, NULL);

    // Lier les éléments
    gst_element_link(source, inputselector);
    gst_element_link(inputselector, textoverlay);
    gst_element_link(textoverlay, sink);

    // Démarrer l'exécution du pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Attendre un message d'arrêt
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