/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#define FUSE_USE_VERSION 30

#include <stdexcept>
#include <unordered_map>

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "prodos.hxx"

using namespace prodos;

static const char *    disk_image = nullptr;
static const int       log_level = LOG_MAX;

static context_t *     context = nullptr;

enum text_mode_t
{
    text_mode_prodos,
    text_mode_unix
};

static text_mode_t text_mode = text_mode_unix;

static void LogMessage(int level, const char *format, ...)
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

static int ToErrno(err_t err)
{
    static std::unordered_map<err_t, int> errors = {
            { err_none,                     0       },
            { err_position_out_of_range,    EBADF   },
            { err_directory_not_found,      ENOTDIR },
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

static time_t L_ToUnixTime(const timestamp_t & timestamp)
{
    struct tm tm = {};

    tm.tm_min  = timestamp.minute;
    tm.tm_hour = timestamp.hour;
    tm.tm_mday = timestamp.day;
    tm.tm_mon  = timestamp.month - 1;
    tm.tm_year = timestamp.year < 70 ? timestamp.year + 100
                                     : timestamp.year;

    return mktime(&tm);
}

/*=================================================================================================
** FUSE operations.
*/

static int prodosfs_getattr(const char *path, struct stat *st, struct fuse_file_info *fi)
{
    LogMessage(LOG_DEBUG1, "prodosfs_getattr(\"%s\", %p)", path, st);

    auto entry = context->GetEntry(path);
    if (entry == nullptr) {
        return -ToErrno(context_t::Error());
    }

    st->st_nlink = 1;
    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_blksize = BLOCK_SIZE;
    st->st_mode = S_IRUSR | S_IRGRP;

    // POSIX has no notion of "file creation time" and ProDOS has no notion of
    // "inode change time", so report the creation time as the change time.
    st->st_ctim.tv_sec = L_ToUnixTime(entry->CreationTimestamp());

    if (entry->IsRoot()) {
        st->st_blocks = context->CountVolumeDirectoryBlocks();
        st->st_size = st->st_blocks * st->st_blksize;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP;

        // ProDOS does not track modification time, so use creation time.
        st->st_mtim.tv_sec = L_ToUnixTime(entry->CreationTimestamp());
    }
    else if (entry->IsFile() || entry->IsDirectory()) {
        auto file = (directory_entry_t *)entry;
        st->st_blocks = file->BlocksUsed();
        st->st_size = file->Eof();
        st->st_mode |= entry->IsFile() ? S_IFREG : S_IFDIR | S_IXUSR | S_IXGRP;
        st->st_mtim.tv_sec = L_ToUnixTime(file->LastModTimestamp());
    }
    else {
        throw std::runtime_error("unexpected storage type");
    }

    return 0;
}

static int prodosfs_open(const char *path, struct fuse_file_info * fi)
{
    LogMessage(LOG_DEBUG1, "prodosfs_open(\"%s\", %p)", path, fi);

    auto fh = context->OpenFile(path);
    if (fh == nullptr) {
        return -ToErrno(context_t::Error());
    }

    fi->fh = reinterpret_cast<uintptr_t>(fh);

    return 0;
}

static int prodosfs_read(const char *path, char *buf, size_t bufsiz, off_t off, struct fuse_file_info * fi)
{
    LogMessage(LOG_DEBUG1, "prodosfs_read(\"%s\", %zd, %p)", path, off, fi);

    auto fh = reinterpret_cast<file_handle_t *>(fi->fh);
    auto pos = fh->Seek(off, SEEK_SET);
    if (pos < 0) {
        return -ToErrno(context_t::Error());
    }

    size_t n = fh->Read(buf, bufsiz);
    if (n == 0 && fh->Eof() == false) {
        return -ToErrno(context_t::Error());
    }

    if (text_mode == text_mode_unix && fh->Type() == file_type_text) {
        auto text = (char *)buf;
        for (int i = 0; i < n; i++) {
            text[i] &= 0x7f;
            if (text[i] == '\r') {
                text[i] = '\n';
            }
        }
    }

    return (int)n;
}

static int prodosfs_release(const char *path, struct fuse_file_info *fi)
{
    LogMessage(LOG_DEBUG1, "prodosfs_release(\"%s\", %p)", path, fi);

    auto fh = reinterpret_cast<file_handle_t *>(fi->fh);
    fh->Close();
    delete fh;

    return 0;
}

static int prodosfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
    LogMessage(LOG_DEBUG1, "prodosfs_getxattr(\"%s\", \"%s\", %p)", path, name, value);

    // TODO

    return 0;
}

static int prodosfs_listxattr(const char *path, char *buf, size_t size)
{
    LogMessage(LOG_DEBUG1, "prodosfs_listxattr(\"%s\",  %p)", path, buf);

    // TODO

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

    return context;
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
        return -ToErrno(context_t::Error());
    }

    fi->fh = reinterpret_cast<uintptr_t>(dh);

    return 0;
}

static int prodosfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                            fuse_file_info *fi, fuse_readdir_flags fl)
{
    LogMessage(LOG_DEBUG1, "prodos_readdir(\"%s\", %p, %d)", path, buf, offset);

    if (offset == 0) {
        filler(buf, ".", nullptr, 0, FUSE_FILL_DIR_PLUS);
        filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_PLUS);
    }

    auto dh = reinterpret_cast<directory_handle_t *>(fi->fh);
    const entry_t *entry = nullptr;
    while ((entry = dh->NextEntry()) != nullptr) {
        std::string name = entry->FileName();
        LogMessage(LOG_DEBUG2, "found entry: %s", name.c_str());
        if (filler(buf, name.c_str(), nullptr, 0, FUSE_FILL_DIR_PLUS)) {
            LogMessage(LOG_WARNING, "readdir buffer full");
            break;
        }
    }

    if (context_t::Error() != err_end_of_file) {
        return -ToErrno(context_t::Error());
    }

    return 0;
}

static int prodosfs_releasedir(const char *path, struct fuse_file_info *fi)
{
    LogMessage(LOG_DEBUG1, "prodosfs_releasedir(\"%s\", %p)", path, fi);

    auto dh = reinterpret_cast<directory_handle_t *>(fi->fh);
    dh->Close();
    delete dh;

    return 0;
}

static struct fuse_operations operations =
{
    .getattr    = prodosfs_getattr,
    .open       = prodosfs_open,
    .read       = prodosfs_read,
    .release    = prodosfs_release,
    .getxattr   = prodosfs_getxattr,
    .listxattr  = prodosfs_listxattr,
    .opendir    = prodosfs_opendir,
    .readdir    = prodosfs_readdir,
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
        fprintf(stderr, "usage: prodosfs -f <mount_dir> <image_file>\n");
        exit(EXIT_FAILURE);
    }

    disk_image = argv[3];
    argc--;

    SetLogger(LogMessage);

    rv = fuse_main(argc, argv, &operations, nullptr);

    return rv;
}

// eof
