/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#define FUSE_USE_VERSION 30

#include <stdexcept>

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>

#include "prodos.hxx"

using namespace prodos;

const char *    disk_image = nullptr;
const int       log_level = LOG_MAX;

context_t *     ctx = nullptr;

void LogMessage(int level, const char *format, ...)
{
    if (level <= log_level) {
        va_list ap;
        fprintf(stderr, "%s[%02d]: ", "prodosfs", level);
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
        fprintf(stderr, "\n");
    }
}

static void *prodosfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    try {
        ctx = new context_t(disk_image);
    }
    catch (std::exception & ex) {
        LogMessage(LOG_CRITICAL, ex.what());
        fuse_exit(fuse_get_context()->fuse);
        exit(EXIT_FAILURE);
    }

    LogMessage(LOG_INFO, "mounted volume: %s", ctx->GetVolumeName().c_str());

    return nullptr;
}

static void prodosfs_destroy(void *private_data)
{
    auto ctx = (context_t *)private_data;
    auto volume_name = ctx->GetVolumeName();
    delete ctx;
    LogMessage(LOG_INFO, "unmounted volume: %s", volume_name.c_str());
}

static struct fuse_operations operations =
{
    .init = prodosfs_init,
    .destroy = prodosfs_destroy,
};

int main(int argc, char *argv[])
{
    int rv = 0;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    if (argc != 4) {
        fprintf(stderr, "usage: prodosfs -f <mount_dir> <image_file>");
        exit(EXIT_FAILURE);
    }

    disk_image = argv[3];
    argc--;

    prodos::SetLogger(LogMessage);

    rv = fuse_main(argc, argv, &operations, nullptr);

    return rv;
}
