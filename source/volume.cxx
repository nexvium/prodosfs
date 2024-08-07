/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/volume.hxx"

#include "prodos/block.hxx"
#include "prodos/directory.hxx"
#include "prodos/filetype.hxx"
#include "prodos/util.hxx"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <vector>

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

namespace prodos
{

typedef std::vector<std::string>    strings_t;

thread_local err_t error = err_none;

static bool
S_IsVolumeDirectoryBlock(const void *addr)
{
    auto block = (const directory_block *)addr;
    return LE_Read16(block->prev) == 0 &&
           block->key.header.storage_type_and_name_length >> 4 == storage_type_volume_block;
}

static void
S_Deobfuscate(const void *src_blk, void * dst_blk)
{
    char *  pw_file = getenv("PRODOSFS_PASSWORD_FILE");
    if (pw_file == nullptr) {
        LOG(LOG_DEBUG1, "PRODOSFS_PASSWORD_FILE env var not set");
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
    else {
        if (passwd[n - 1] == '\n') {
            passwd[n - 1] = 0;
            n--;
        }
        if (n < 2) {
            LOG(LOG_ERROR, "password is too short");
            return;
        }
    }

    auto cipher = (const unsigned char *)src_blk;
    auto plain = (unsigned char *)dst_blk;
    auto ptr = passwd;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        plain[i] = (char)(cipher[i] ^ (*ptr++ ^ 0x7F));

        // There's an off-by-one error in the program I wrote 30+ years
        // ago, so the last character of the password is not used.
        if (*(ptr + 1) == 0) {
            ptr = passwd;
        }
    }

    if (S_IsVolumeDirectoryBlock(plain)) {
        return;
    }
}

volume_t::volume_t(const std::string & pathname)
    : _disk(pathname)
{
    _root = _GetVolumeDirectoryBlock();
    if (_root == nullptr) {
        throw std::runtime_error("unable to find volume directory block");
    }

    auto volume = volume_header_t::Create(&_root->key.header);
    if (volume->MinVersion() != 0) {
        throw std::runtime_error("unexpected minimum prodos version");
    }
    else if (volume->EntriesPerBlock() != ENTRIES_PER_BLOCK) {
        throw std::runtime_error("unexpected number of entries per block");
    }
    else if (volume->EntryLength() != sizeof(directory_entry)) {
        throw std::runtime_error("unexpected entry length");
    }
    else if (volume->TotalBlocks() != _disk.NumBlocks()) {
        throw std::runtime_error("unexpected total blocks");
    }
}

err_t
volume_t::Error()
{
    return error;
}

void
volume_t::ClearError()
{
    error = err_none;
}

directory_block *
volume_t::_GetVolumeDirectoryBlock()
{
    const void *    block   = _disk.ReadBlock(2);

    if (S_IsVolumeDirectoryBlock(block)) {
        return (directory_block *)block;
    }

    char tmp_blk[BLOCK_SIZE];
    S_Deobfuscate(block, tmp_blk);

    if (S_IsVolumeDirectoryBlock(tmp_blk)) {
        LOG(LOG_INFO, "deobfuscated protected disk");
        _disk.WriteBlock(2, tmp_blk);
        return (directory_block *)_disk.ReadBlock(2);
    }

    block = _disk.ReadTrackSector(0, 11);
    if (S_IsVolumeDirectoryBlock(block)) {
        LOG(LOG_INFO, "converting track-and-sector disk to block disk");
        _disk.Convert(disk_t::RWTS_TO_BLOCK);
        return (directory_block *)_disk.ReadBlock(2);
    }

    S_Deobfuscate(block, tmp_blk);
    if (S_IsVolumeDirectoryBlock(tmp_blk)) {
        LOG(LOG_INFO, "converting track-and-sector disk to block disk");
        _disk.Convert(disk_t::RWTS_TO_BLOCK);

        block = _disk.ReadBlock(2);
        S_Deobfuscate(block, tmp_blk);
        LOG(LOG_INFO, "deobfuscated protected disk");
        _disk.WriteBlock(2, tmp_blk);
        return (directory_block *)_disk.ReadBlock(2);
    }

    return nullptr;
}

std::string
volume_t::Name() const
{
    auto volume = volume_header_t::Create(&_root->key.header);
    return volume->FileName();
}

int
volume_t::FileCount() const
{
    auto volume = volume_header_t::Create(&_root->key.header);
    return volume->FileCount();
}

int
volume_t::TotalBlocks() const
{
    auto volume = volume_header_t::Create(&_root->key.header);
    return volume->TotalBlocks();
}

static strings_t
S_SplitPath(const std::string & pathname)
{
    strings_t path;

    size_t start = pathname[0] == '/' ? 1 : 0;
    size_t pos = 0;
    while ((pos = pathname.find('/', start)) != std::string::npos) {
        path.push_back(pathname.substr(start, pos - 1));
        start = pos + 1;
    }
    path.push_back(pathname.substr(start));

    return path;
}

const entry_t *
volume_t::GetEntry(const std::string & pathname) const
{
    if (pathname == "/") {
        return (entry_t *)&_root->key.header;
    }

    auto path = S_SplitPath(pathname);
    auto itr = path.begin();
    auto handle = new directory_handle_t(this, _root);
    auto entry = handle->NextEntry();

    std::unique_ptr<directory_handle_t> defer_cleanup(handle);

    while (entry != nullptr && itr != path.end()) {
        if (entry->NameMatches(*itr)) {
            if (++itr == path.end()) {
                return entry;
            }
            else if (entry->IsDirectory() == false) {
                error = err_directory_not_found;
                return nullptr;
            }

            auto key_pointer = entry->KeyPointer();
            auto key_block = (directory_block *)_disk.ReadBlock(key_pointer);
            handle->_Open(key_block);
        }

        entry = handle->NextEntry();
    }

    error = err_file_not_found;

    return nullptr;
}

file_handle_t *
volume_t::OpenFile(const std::string & pathname) const
{
    auto entry = GetEntry(pathname);
    if (entry == nullptr) {
        return nullptr;
    }
    else if (entry->IsFile() == false) {
        error = err_unsupported_storage_type;
        return nullptr;
    }

    return new file_handle_t(this, (const directory_entry_t *)entry);
}

directory_handle_t *
volume_t::OpenDirectory(const std::string & pathname) const
{
    if (pathname == "/") {
        return new directory_handle_t(this, _root);
    }

    auto entry = GetEntry(pathname);
    if (entry == nullptr) {
        return nullptr;
    }

    if (entry->IsDirectory() == false) {
        error = err_directory_not_found;
        return nullptr;
    }

    auto dirent = (const directory_entry_t *)entry;
    auto pointer = dirent->KeyPointer();
    auto key_block = (const directory_block *)_disk.ReadBlock(pointer);

    return new directory_handle_t(this, key_block);
}

const void *
volume_t::GetBlock(int index) const
{
    static uint8_t sparse_block[BLOCK_SIZE] = {};
    return index ? _disk.ReadBlock(index) : sparse_block;
}

int
volume_t::CountBlocksUsed() const
{
    uint16_t pointer = LE_Read16(_root->key.header.bit_map_pointer);
    auto blocks = _disk.NumBlocks();
    auto used = 0;

    while (blocks > 0) {
        auto bitmap = (const uint8_t *)_disk.ReadBlock(pointer++);
        for (auto i = 0; i < BLOCK_SIZE && blocks > 0; i++) {
            used += sizeof(uint8_t) - __builtin_popcount(bitmap[i]);
            blocks -= sizeof(uint8_t);
        }
    }

    return used;
}

int
volume_t::CountRootDirectoryBlocks() const
{
    int num_blocks = 1;

    const directory_block * block = _root;
    uint16_t pointer = LE_Read16(block->next);
    while (pointer != 0) {
        num_blocks++;
        block = (const directory_block *)_disk.ReadBlock(pointer);
        pointer = LE_Read16(block->next);
    }

    return num_blocks;
}

std::string *
volume_t::Catalog(const std::string & pathname) const
{
    auto pathdir = std::filesystem::path(pathname).parent_path();
    auto dh = OpenDirectory(pathdir);
    if (dh == nullptr) {
        return nullptr;
    }

    auto output = new std::string();

    char line[128] = {};
    sprintf(line, "\n/%s%s\n\n", Name().c_str(), pathdir == "/" ? "" : pathdir.c_str());
    output->append(line);

    sprintf(line, " %-15s %4s  %6s  %-15s  %-15s  %7s  %7s\n\n",
                  "NAME", "TYPE", "BLOCKS", "MODIFIED", "CREATED", "ENDFILE", "SUBTYPE");
    output->append(line);

    const directory_entry_t * entry = nullptr;
    while ((entry = dh->NextEntry()) != nullptr) {
        char subtype[16] = {};
        uint8_t type = entry->FileType();
        switch (type) {
            case file_type_binary:
                if (entry->AuxType() != 0) {
                    sprintf(subtype, "A=$%04X", entry->AuxType());
                }
                break;
            default:
                subtype[0] = 0;
        }
        sprintf(line, " %-15s  %3s  %6d  %-15s  %-15s  %7d  %7s\n",
                entry->FileName().c_str(),
                GetFileTypeInfo(entry->FileType())->name.c_str(),
                entry->BlocksUsed(),
                entry->LastModTimestamp().AsString().c_str(),
                entry->CreationTimestamp().AsString().c_str(),
                entry->Eof(),
                subtype);
        output->append(line);
    }

    auto total_blocks = TotalBlocks();
    auto blocks_used = CountBlocksUsed();
    sprintf(line, "\nBLOCKS FREE: %4d          BLOCKS USED: %4d          TOTAL BLOCKS: %4d\n\n",
                  total_blocks - blocks_used, blocks_used, total_blocks);
    output->append(line);

    dh->Close();
    delete dh;

    return output;
}

bool volume_t::Rename(const std::string & name)
{
    if (!IsValidName(name)) {
        return false;
    }

    std::string s = name;
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });

    memset(_root->key.header.volume_name, 0, FILENAME_LENGTH);
    memcpy(_root->key.header.volume_name, s.c_str(), FILENAME_LENGTH);
    _root->key.header.storage_type_and_name_length &= 0b1111'0000;
    _root->key.header.storage_type_and_name_length |= s.length();
    _disk.WriteBlock(2, _root);

    return true;
}

} // namespace

// eof
