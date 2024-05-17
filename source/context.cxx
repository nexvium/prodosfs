/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/context.hxx"

#include <stdexcept>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "prodos/block.hxx"
#include "prodos/util.hxx"

namespace prodos
{

static thread_local err_t error = err_none;

static void
L_Deobfuscate(const void *src_blk, void * dst_blk)
{
    char *  pw_file = getenv("PRODOSFS_PASSWORD_FILE");
    if (pw_file == nullptr) {
        return;
    }

    int fd = open(pw_file, O_APPEND | O_CLOEXEC);
    if (fd < 0) {
        LOG(LOG_ERROR, "unable to open password file");
        return;
    }

    liberator_t<int, int (*)(int)> closer(fd, close);

    struct stat st = {};
    if (fstat(fd, &st) < 0) {
        LOG(LOG_ERROR, "unable to stat password file");
        return;
    }
    else if ((st.st_mode & S_IFMT) != S_IFREG) {
        LOG(LOG_ERROR, "password file is not a regular file");
        return;
    }
    else if ((st.st_mode & (S_IRWXG | S_IRWXO)) != 0) {
        LOG(LOG_WARNING, "password file should only be readable/writable and only by owner");
    }

    const int buflen = 32;
    char passwd[buflen] = {};
    auto n = read(fd, passwd, buflen - 1);
    if (n < 0) {
        LOG(LOG_ERROR, "unable to read from password file");
        return;
    }
    else if (n == 0) {
        LOG(LOG_ERROR, "password file is empty");
        return;
    }
    else if (n < 2) {
        LOG(LOG_ERROR, "password is too short");
        return;
    }

    auto cipher = (const char *)src_blk;
    auto plain = (char *)dst_blk;
    auto ptr = passwd;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        plain[i] = (char)(cipher[i] ^ *ptr++);

        // There's an off-by-one error in the program I wrote 30+ years
        // ago, so the last character of the password is not used.
        if (*(ptr + 1) == 0) {
            ptr = passwd;
        }
    }

    if (plain[0] != 0 || plain[1] != 0 || plain[4] >> 4 != storage_type_volume_block) {
        LOG(LOG_DEBUG1, "disk is not protected with password");
    }

    return;
}

context_t::context_t(const std::string & pathname)
    : _disk(pathname)
{
    const directory_block_t *   dir_blk = _GetVolumeDirectoryBlock();
    if (dir_blk == nullptr) {
        throw std::runtime_error("unable to find volume directory block");
    }

}

const directory_block_t *
context_t::_GetVolumeDirectoryBlock(void)
{
    const void *                block   = _disk.ReadBlock(2);
    const directory_block_t *   dir_blk = nullptr;

    dir_blk = directory_block_t::Create(block);
    if (dir_blk) {
        return dir_blk;
    }

    char tmp_blk[BLOCK_SIZE];
    L_Deobfuscate(block, tmp_blk);

    dir_blk = directory_block_t::Create(tmp_blk);
    if (dir_blk) {
        LOG(LOG_INFO, "deobfuscated protected disk");
        _disk.WriteBlock(2, tmp_blk);
        return dir_blk;
    }

    dir_blk = directory_block_t::Create(_disk.ReadTrackSector(0, 11));
    if (dir_blk) {
        LOG(LOG_INFO, "converting track-and-sector disk to block disk");
        _disk.Convert(disk_t::RWTS_TO_BLOCK);
        return directory_block_t::Create(_disk.ReadBlock(2));
    }

    return nullptr;
}

} // namespace

// eof
