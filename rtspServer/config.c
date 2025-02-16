#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int lire_configuration(const char *fichier, char **video_source, char **mount_point, char **destination_port, char **text) {
    FILE *fp = fopen(fichier, "r");
    if (fp == NULL) {
        fprintf(stderr, "Impossible d'ouvrir le fichier de configuration.\n");
        return -1;
    }

    if (fscanf(fp, "%ms\n%ms\n%ms\n%ms", video_source, mount_point, destination_port, text) != 4) {
        fprintf(stderr, "Erreur de lecture du fichier de configuration.\n");
        fclose(fp);
        return -1;
    }
    fclose(fp);

    printf("Configuration lue avec succès:\n");
    printf("Source vidéo: %s\n", *video_source);
    printf("Point de montage: %s\n", *mount_point);
    printf("Port de destination: %s\n", *destination_port);
    printf("Texte: %s\n", *text);

    return 0;
}
