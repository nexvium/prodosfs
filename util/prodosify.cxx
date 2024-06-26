/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos.hxx"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "usage: prodosify <image_in> <image_out>\n");
        return EXIT_FAILURE;
    }

    prodos::volume_t volume(argv[1]);
    if (volume.IsDirty() == false) {
        fprintf(stderr, "prodosify: volume is already normal prodos disk\n");
        return EXIT_FAILURE;
    }

    if (volume.Save(argv[2]) == false) {
        fprintf(stderr, "prodosify: error %d\n", errno);
        return EXIT_FAILURE;
    }

    printf("prodosify: wrote normalized prodos disk\n");

    return EXIT_SUCCESS;
}
