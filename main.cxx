/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#define FUSE_USE_VERSION 30

#include <stdexcept>
#include <unordered_map>

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>

#include "prodos.hxx"

using namespace prodos;

const char *    disk_image = nullptr;
const int       log_level = LOG_MAX;

context_t *     context = nullptr;

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

int ToErrno(err_t err)
{
    static std::unordered_map<err_t, int> errors = {
            { err_none,                     0       },
            { err_position_out_of_range,    EBADF   },
            { err_directory_not_found,      ENOENT  },
            { err_volume_not_found,         ENOENT  },
            { err_file_not_found,           ENOENT  },
            { err_end_of_file,              0       },
            { err_position_out_of_range,    ENXIO   },
    };

    auto itr = errors.find(err);
    if (itr == errors.end()) {
        return EIO;
    }

    return itr->second;
}

/*=================================================================================================
** FUSE operations.
*/

static int prodosfs_getattr(const char *path, struct stat *st, struct fuse_file_info *fi)
{
    LogMessage(LOG_DEBUG1, "prodosfs_getattr(\"%s\", %p)", path, st);

    auto entry = context->GetEntry(path);
    if (entry == nullptr) {
        return -ToErrno(context->Error());
    }

    return 0;
}

static void *prodosfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    LogMessage(LOG_DEBUG1, "prodosfs_init()");

    try {
        context = new context_t(disk_image);
    }
    catch (std::exception & ex) {
        LogMessage(LOG_CRITICAL, ex.what());
        fuse_exit(fuse_get_context()->fuse);
        exit(EXIT_FAILURE);
    }

    LogMessage(LOG_INFO, "mounted volume: %s", context->GetVolumeName().c_str());

    return nullptr;
}

static void prodosfs_destroy(void *private_data)
{
    LogMessage(LOG_DEBUG1, "prodosfs_init(%p)", private_data);

    auto ctx = (context_t *)private_data;
    auto volume_name = ctx->GetVolumeName();
    delete ctx;

    LogMessage(LOG_INFO, "unmounted volume: %s", volume_name.c_str());
}

static int prodosfs_opendir(const char *path, struct fuse_file_info *fi)
{
    LogMessage(LOG_DEBUG1, "prodosfs_opendir(\"%s\", %p)", path, fi);

    auto dh = context->OpenDirectory(path);
    if (dh == nullptr) {
        return -ToErrno(context->Error());
    }

    fi->fh = reinterpret_cast<uintptr_t>(dh);

    return 0;
}

static int prodos_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                          fuse_file_info *fi, fuse_readdir_flags fl)
{
    LogMessage(LOG_DEBUG1, "prodos_readdir(\"%s\", %p, %d)", path, buf, offset);

    if (offset == 0) {
        filler(buf, ".",  nullptr, 0, FUSE_FILL_DIR_PLUS);
        filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_PLUS);
    }

    auto dh = reinterpret_cast<directory_t *>(fi->fh);
    const entry_t *entry = nullptr;
    while ((entry = dh->NextEntry()) != nullptr) {
        std::string name = entry->FileName();
        LogMessage(LOG_DEBUG2, "found entry: %s", name.c_str());
        if (filler(buf, name.c_str(), nullptr, 0, FUSE_FILL_DIR_PLUS)) {
            LogMessage(LOG_WARNING, "readdir buffer full");
            break;
        }
    }

    if (context->Error()) {
        return -ToErrno(context->Error());
    }

    return 0;
}

static int prodosfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    LogMessage(LOG_DEBUG1, "prodosfs_releasedir(\"%s\", %p)", path, fi);

    auto dh = reinterpret_cast<directory_t *>(fi->fh);
    dh->Close();
    delete dh;

    return 0;
}


static struct fuse_operations operations =
{
    .getattr    = prodosfs_getattr,
    .opendir    = prodosfs_opendir,
    .readdir    = prodos_readdir,
    .releasedir = prodosfs_releasedir,
    .init       = prodosfs_init,
    .destroy    = prodosfs_destroy,
};

/*=================================================================================================
** Main
*/

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
