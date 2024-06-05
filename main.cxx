/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#define FUSE_USE_VERSION 30

#include "prodos.hxx"

#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace prodos;

static char *       mount_dir = nullptr;
static char *       disk_image = nullptr;
static bool         foreground = false;
static bool         use_name  = false;
static int          log_level = LOG_INFO;
static bool         debug = false;
static context_t *  context = nullptr;

typedef std::unordered_map<std::string, std::string>    attributes_t;

enum text_mode_t
{
    text_mode_prodos,
    text_mode_unix
};

static text_mode_t text_mode = text_mode_unix;

//================================================================================================
// Helper functions
//------------------------------------------------------------------------------------------------

static void S_LogMessage(int level, const char *format, ...)
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

static int S_ToError(err_t err)
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

static time_t S_ToUnixTime(const timestamp_t & timestamp)
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

static std::string S_AccessToString(uint8_t access)
{
    std::vector<std::string>    allowed;
    if (AXS_READ(access))       allowed.emplace_back("READ");
    if (AXS_WRITE(access))      allowed.emplace_back("WRITE");
    if (AXS_BACKUP(access))     allowed.emplace_back("BACKUP");
    if (AXS_RENAME(access))     allowed.emplace_back("RENAME");
    if (AXS_DESTROY(access))    allowed.emplace_back("DESTROY");

    // Would be nice if the STL provided a join function.
    std::string str;
    for (const auto & flag : allowed) {
        if (!str.empty()) str += " | ";
        str += flag;
    }

    return str;
}

static std::string S_AuxTypeToString(uint16_t aux_type)
{
    char buffer[16] = {};
    sprintf(buffer, "$%04X", aux_type);
    return { buffer };
}

inline std::string XATTR(const char *name)
{
    return std::string("prodos.") + name;
}

static attributes_t S_GetAttributes(const entry_t * entry)
{
    attributes_t    attributes;

    attributes[XATTR("creation_timestamp")] = entry->CreationTimestamp().AsString();
    attributes[XATTR("access")] = S_AccessToString(entry->Access());

    // TODO need to find a version-number to ProDOS-version map
    attributes[XATTR("version")] = std::to_string(entry->Version());
    attributes[XATTR("min_version")] = std::to_string(entry->MinVersion());

    if (entry->IsFile() || entry->IsDirectory()) {
        auto dirent = (const directory_entry_t *)entry;

        auto info = GetFileTypeInfo(dirent->FileType());
        attributes[XATTR("file_type")] = info->type;
        attributes[XATTR("file_type_name")] = info->name;
        attributes[XATTR("file_type_description")] = info->description;

        attributes[XATTR("aux_type")] = S_AuxTypeToString(dirent->AuxType());

        if (IsAppleWorksFile(dirent->FileType())) {
            auto name = AppleWorksFileName(dirent->FileName(), dirent->AuxType());
            attributes[XATTR("appleworks_filename")] = name;
        }
    }
    else if (entry->IsRoot()) {
        auto volume = (const volume_header_t *)entry;
        attributes[XATTR("volume_name")] = volume->FileName();
        attributes[XATTR("file_count")] = std::to_string(volume->FileCount());
        attributes[XATTR("total_blocks")] = std::to_string(volume->TotalBlocks());
        attributes[XATTR("used_blocks")] = std::to_string(context->CountVolumeBlocksUsed());
        attributes[XATTR("image_file")] = disk_image;
    }
    else {
        throw std::runtime_error("unexpected file type");
    }

    return attributes;
}

//================================================================================================
// FUSE operations
//------------------------------------------------------------------------------------------------

static int prodosfs_getattr(const char *path, struct stat *st, struct fuse_file_info *)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_getattr(\"%s\", %p)", path, st);

    auto entry = context->GetEntry(path);
    if (entry == nullptr) {
        return -S_ToError(context_t::Error());
    }

    st->st_nlink = 1;
    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_blksize = BLOCK_SIZE;
    st->st_mode = S_IRUSR | S_IRGRP;

    // POSIX has no notion of "file creation time" and ProDOS has no notion of
    // "inode change time", so report the creation time as the change time.
    st->st_ctim.tv_sec = S_ToUnixTime(entry->CreationTimestamp());

    if (entry->IsRoot()) {
        st->st_blocks = context->CountVolumeDirectoryBlocks();
        st->st_size = st->st_blocks * st->st_blksize;
        st->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP;

        // ProDOS does not track modification time, so use creation time.
        st->st_mtim.tv_sec = S_ToUnixTime(entry->CreationTimestamp());
    }
    else if (entry->IsFile() || entry->IsDirectory()) {
        auto file = (directory_entry_t *)entry;
        st->st_blocks = file->BlocksUsed();
        st->st_size = file->Eof();
        st->st_mode |= entry->IsFile() ? S_IFREG : S_IFDIR | S_IXUSR | S_IXGRP;
        st->st_mtim.tv_sec = S_ToUnixTime(file->LastModTimestamp());
    }
    else {
        throw std::runtime_error("unexpected storage type");
    }

    return 0;
}

static int prodosfs_open(const char *path, struct fuse_file_info * fi)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_open(\"%s\", %p)", path, fi);

    auto fh = context->OpenFile(path);
    if (fh == nullptr) {
        return -S_ToError(context_t::Error());
    }

    fi->fh = reinterpret_cast<uintptr_t>(fh);

    return 0;
}

static int prodosfs_read(const char *path, char *buf, size_t bufsiz, off_t off, struct fuse_file_info * fi)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_read(\"%s\", %zd, %p)", path, off, fi);

    auto fh = reinterpret_cast<file_handle_t *>(fi->fh);
    auto pos = fh->Seek(off, SEEK_SET);
    if (pos < 0) {
        return -S_ToError(context_t::Error());
    }

    size_t n = fh->Read(buf, bufsiz);
    if (n == 0 && fh->Eof() == false) {
        return -S_ToError(context_t::Error());
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

static int prodosfs_close(const char *path, struct fuse_file_info *fi)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_close(\"%s\", %p)", path, fi);

    auto fh = reinterpret_cast<file_handle_t *>(fi->fh);
    fh->Close();
    delete fh;

    return 0;
}

static int prodosfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_getxattr(\"%s\", \"%s\", %p, %zd)", path, name, value, size);

    auto entry = context->GetEntry(path);
    if (entry == nullptr) {
        return -S_ToError(context_t::Error());
    }

    auto attributes = S_GetAttributes(entry);
    auto itr = attributes.find(name);

    if (itr == attributes.end()) {
        return -ENODATA;
    }

    size_t value_size = itr->second.length() + 1;
    if (size > 0) {
        if (size < value_size) {
            return -ERANGE;
        }
        strcpy(value, itr->second.c_str());
    }

    return (int)value_size;
}

static int prodosfs_listxattr(const char *path, char *buffer, size_t size)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_listxattr(\"%s\", %p, %zd)", path, buffer, size);

    auto entry = context->GetEntry(path);
    if (entry == nullptr) {
        return -S_ToError(context_t::Error());
    }

    char *  position = buffer;
    size_t  remaining = size;

    size_t length = 0;
    auto attributes = S_GetAttributes(entry);
    for (const auto & attr : attributes) {
        auto name_size = attr.first.length() + 1;
        if (size > 0) {
            if (attr.first.length() < remaining) {
                snprintf(position, name_size, "%s", attr.first.c_str());
                position += name_size;
                remaining -= name_size;
            }
            else {
                return -ERANGE;
            }
        }
        length += name_size;
    }

    return (int)length;
}

static void *prodosfs_mount(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_mount()");

    try {
        context = new context_t(disk_image);
    }
    catch (std::exception & ex) {
        S_LogMessage(LOG_CRITICAL, ex.what());
        fuse_exit(fuse_get_context()->fuse);
        exit(EXIT_FAILURE);
    }

    if (mount_dir) {
        S_LogMessage(LOG_INFO, "mounted %s in %s", disk_image, mount_dir);
    }
    else {
        S_LogMessage(LOG_INFO, "mounted volume: %s", context->GetVolumeName().c_str());
    }

    return context;
}

static void prodosfs_umount(void *private_data)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_umount(%p)", private_data);

    auto ctx = (context_t *)private_data;
    auto volume_name = ctx->GetVolumeName();
    delete ctx;

    if (mount_dir) {
        S_LogMessage(LOG_INFO, "unmounted %s in %s", disk_image, mount_dir);
    }
    else {
        S_LogMessage(LOG_INFO, "unmounted volume: %s", context->GetVolumeName().c_str());
    }
}

static int prodosfs_opendir(const char *path, struct fuse_file_info *fi)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_opendir(\"%s\", %p)", path, fi);

    auto dh = context->OpenDirectory(path);
    if (dh == nullptr) {
        return -S_ToError(context_t::Error());
    }

    fi->fh = reinterpret_cast<uintptr_t>(dh);

    return 0;
}

static int prodosfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
                            fuse_file_info *fi, fuse_readdir_flags fl)
{
    S_LogMessage(LOG_DEBUG1, "prodos_readdir(\"%s\", %p, %d)", path, buf, offset);

    if (offset == 0) {
        filler(buf, ".", nullptr, 0, FUSE_FILL_DIR_PLUS);
        filler(buf, "..", nullptr, 0, FUSE_FILL_DIR_PLUS);
    }

    auto dh = reinterpret_cast<directory_handle_t *>(fi->fh);
    const entry_t * entry = nullptr;
    while ((entry = dh->NextEntry()) != nullptr) {
        std::string name = entry->FileName();
        S_LogMessage(LOG_DEBUG2, "found entry: %s", name.c_str());
        if (filler(buf, name.c_str(), nullptr, 0, FUSE_FILL_DIR_PLUS)) {
            S_LogMessage(LOG_WARNING, "readdir buffer full");
            break;
        }
    }

    if (context_t::Error() != err_end_of_file) {
        return -S_ToError(context_t::Error());
    }

    return 0;
}

static int prodosfs_closedir(const char *path, struct fuse_file_info *fi)
{
    S_LogMessage(LOG_DEBUG1, "prodosfs_closedir(\"%s\", %p)", path, fi);

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
    .release    = prodosfs_close,
    .getxattr   = prodosfs_getxattr,
    .listxattr  = prodosfs_listxattr,
    .opendir    = prodosfs_opendir,
    .readdir    = prodosfs_readdir,
    .releasedir = prodosfs_closedir,
    .init       = prodosfs_mount,
    .destroy    = prodosfs_umount,
};

//================================================================================================
// Main
//------------------------------------------------------------------------------------------------

bool S_UpdateMountDirectory()
{
    size_t path_len = strlen(mount_dir);
    try {
        const context_t ctx = context_t(disk_image);
        std::string vol_name = ctx.GetVolumeName();
        if (IsValidName(vol_name) == false) {
            fprintf(stderr, "prodosfs: invalid ProDOS volume name -- \"%s\"\n", vol_name.c_str());
            return false;
        }

        // Add enough space for the volume name and a potential "-N" suffix.
        mount_dir = (char *)realloc(mount_dir, path_len + 1 + vol_name.length() + 2 + 1);
        strcat(mount_dir, "/");
        strcat(mount_dir, vol_name.c_str());
    }
    catch (std::exception & ex) {
        fprintf(stderr, "prodosfs: not a ProDOS disk image -- %s\n", disk_image);
        return false;
    }

    struct stat st = {};
    if (stat(mount_dir, &st) == 0 || errno != ENOENT) {
        path_len = strlen(mount_dir);
        for (int i = 1; i < 10 && errno != ENOENT; i++) {
            mount_dir[path_len + 0] = '-';
            mount_dir[path_len + 1] = '0' + i;
            mount_dir[path_len + 2] = 0;

            errno = 0;
            stat(mount_dir, &st);
        }
        if (errno == 0) {
            mount_dir[path_len] = 0;
            fprintf(stderr, "prodosfs: too many directories with volume name -- %s\n", mount_dir);
            return false;
        }
    }

    return true;
}

static void S_HandleOptions(int argc, char *argv[])
{
    opterr = 0;
    int c = 0;
    while ((c = getopt(argc, argv, "dfhl:n")) != -1) {
        switch (c) {
        case 'd':
            debug = true;
            break;
        case 'f':
            foreground = true;
            break;
        case 'h':
            fprintf(stdout, "usage: prodosfs [-l N] [-d] [-f] [-n] <mount dir> <image file>\n");
            exit(EXIT_SUCCESS);
            break;
        case 'l':
            log_level = atoi(optarg);
            if (log_level < LOG_CRITICAL || log_level > LOG_MAX) {
                fprintf(stderr, "prodosfs: log level must be 0-9 -- %s\n", optarg);
                exit(EXIT_FAILURE);
            }
            break;
        case 'n':
            use_name = true;
            break;
        case '?':
        default:
            fprintf(stderr, "prodosfs: invalid option -- %c\n", (char)optopt);
            exit(EXIT_FAILURE);
        }
    }

    return;
}

int main(int argc, char *argv[])
{
    S_HandleOptions(argc, argv);

    if (argc - optind < 2) {
        fprintf(stderr, "usage: prodosfs [opts] <mount_dir> <image_file>\n");
        exit(EXIT_FAILURE);
    }

    mount_dir = realpath(argv[optind], nullptr);
    disk_image = realpath(argv[optind + 1], nullptr);

    SetLogger(S_LogMessage);

    bool cleanup = false;
    if (use_name == true) {
        if (S_UpdateMountDirectory() == false) {
            exit(EXIT_FAILURE);
        }

        // WARNING! does not do all checks mount should do
        cleanup = true;
        if (mkdir(mount_dir, 0777)) {
            fprintf(stderr, "prodosfs: unable to create mount directory -- %s\n", mount_dir);
            exit(EXIT_FAILURE);
        }
    }

    // Build arguments vector for FUSE's main.
    fuse_args args = FUSE_ARGS_INIT(0, nullptr);
    fuse_opt_add_arg(&args, "prodosfs");
    fuse_opt_add_arg(&args, "-oauto_unmount");
    if (foreground) {
        fuse_opt_add_arg(&args, "-f");
    }
    if (debug) {
        fuse_opt_add_arg(&args, "-d");
    }
    fuse_opt_add_arg(&args, mount_dir);

    // Enter FUSE main loop.
    int rv = fuse_main(args.argc, args.argv, &operations, nullptr);

    fuse_opt_free_args(&args);
    if (cleanup) {
        rmdir(argv[2]);
    }

    free(disk_image);
    free(mount_dir);

    return rv;
}

// eof
