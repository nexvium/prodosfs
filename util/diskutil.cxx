/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos.hxx"

#include <filesystem>
#include <iostream>

#include <string.h>

static auto S_Normalize(int argc, char *argv[]) -> int
{
    if (argc != 4) {
        fprintf(stderr, "usage: diskutil normalize <image_in> <image_out>\n");
        return EXIT_FAILURE;
    }

    prodos::volume_t volume(argv[2]);
    if (volume.IsDirty() == false) {
        fprintf(stderr, "diskutil: volume is already normal prodos disk\n");
        return EXIT_FAILURE;
    }

    if (volume.Save(argv[3]) == false) {
        fprintf(stderr, "diskutil: error %d\n", errno);
        return EXIT_FAILURE;
    }

    printf("diskutil: wrote normalized prodos disk\n");

    return EXIT_SUCCESS;
}

static auto S_OpenVolume(const char *path) -> prodos::volume_t *
{
    prodos::volume_t *volume = nullptr;
    try {
        volume = new prodos::volume_t(path);
    }
    catch (const std::exception & ex) {
        fprintf(stderr, "diskutil: %s\n", ex.what());
        exit(EXIT_FAILURE);
    }

    return volume;
}

static auto S_Catalog(int argc, char *argv[]) -> int
{
    if (argc != 3) {
        fprintf(stderr, "usage: diskutil catalog <image_in>\n");
        return EXIT_FAILURE;
    }

    prodos::volume_t *volume = S_OpenVolume(argv[2]);
    printf("%s", volume->Catalog("/")->c_str());

    delete volume;

    return EXIT_SUCCESS;
}

static auto S_Rename(int argc, char *argv[]) -> int
{
    if (argc != 3) {
        fprintf(stderr, "usage: diskutil rename <image_in>\n");
        return EXIT_FAILURE;
    }

    prodos::volume_t *volume = S_OpenVolume(argv[2]);
    printf("%s", volume->Catalog("/")->c_str());

    std::filesystem::path pathname = argv[2];

    bool done = false;
    while (!done) {
        printf("%11s: %s\n", "Image name", pathname.filename().c_str());
        printf("%11s: %s\n", "Volume name", volume->Name().c_str());
        printf("\n");
        printf("Rename 1) Volume, 2) Image name to volume name, or 3) exit? ");

        std::string input;
        std::cin >> input;

        if (input == "1") {
            printf("New volume name? ");
            std::cin >> input;
            if (!input.empty()) {
                if (!prodos::IsValidName(input)) {
                    fprintf(stderr, "diskutil: invalid name - %s\n", input.c_str());
                    continue;
                }

                volume->Rename(input);
                if (!volume->Save(pathname)) {
                     fprintf(stderr, "diskutil: %s\n", strerror(errno));
                     exit(EXIT_FAILURE);
                }
            }
        }
        else if (input == "2") {
            std::string new_name = pathname.remove_filename().string() + volume->Name() + ".po";
            if (rename(pathname.c_str(), new_name.c_str())) {
                fprintf(stderr, "diskutil: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else if (input == "3") {
            done = true;
        }
        else {
            fprintf(stderr, "diskutil: invalid option - %s\n", input.c_str());
        }

        printf("\n");
    }
    delete volume;

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: diskutil <cmd> [args]\n");
        return EXIT_FAILURE;
    }

    std::string cmd = argv[1];
    int ev = 0;
    if (cmd == "catalog") {
        ev = S_Catalog(argc, argv);
    }
    else if (cmd == "normalize") {
        ev = S_Normalize(argc, argv);
    }
    else if (cmd == "rename") {
        ev = S_Rename(argc, argv);
    }
    else {
        fprintf(stderr, "diskutil: unrecognized command -- %s\n", cmd.c_str());
        return EXIT_FAILURE;
    }

    return ev;
}
