/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_SYSTEM_HXX
#define PRODOSFS_SYSTEM_HXX

#include "prodos/disk.hxx"
#include "block.hxx"

#include <string>

namespace prodos
{

enum err_t
{
    err_none                        = 0x00,
    err_input_output                = 0x27,
    err_no_device_connected         = 0x28,
    err_disk_switched               = 0x2E,
    err_invalid_pathname            = 0x40,
    err_invalid_reference_number    = 0x43,
    err_directory_not_found         = 0x44,
    err_volume_not_found            = 0x45,
    err_file_not_found              = 0x46,
    err_unsupported_storage_type    = 0x4B,
    err_end_of_file                 = 0x4C,
    err_position_out_of_range       = 0x4D,
    err_directory_structure_damaged = 0x51,
    err_file_structure_damaged      = 0x54,
};

enum storage_type_t
{
    storage_type_none           = 0x0,
    storage_type_seedling_file  = 0x1,
    storage_type_sapling_file   = 0x2,
    storage_type_tree_file      = 0x3,
    storage_type_pascal_area    = 0x4,
    storage_type_subdirectory   = 0xD,
    storage_type_subdir_block   = 0xE,
    storage_type_volume_block   = 0xF,
};

class entry_t
{
public:
    std::string FileName(void) const;
};

class directory_t
{
public:
    void                Close(void);

    const entry_t *     NextEntry() const;
};

class file_t
{
public:
    void                Close(void);
    size_t              Read(void *buffer, size_t size) const;
    bool                Eof(void) const;
    off_t               Seek(off_t offset, int whence);
};

class context_t
{
public:
    context_t(const std::string & pathname);
    context_t(const context_t &)                = delete;
    context_t(const context_t &&)               = delete;

    ~context_t();

    std::string     GetVolumeName(void) const;

    entry_t *       GetEntry(const std::string & pathname) const;

    directory_t *   OpenDirectory(const std::string & pathname) const;

    file_t *        OpenFile(const std::string & pathname) const;

    err_t           Error(void) const;

private:
    disk_t  _disk;

    const directory_block_t *   _GetVolumeDirectoryBlock(void);
};

} // namespace

#endif // PRODOSFS_SYSTEM_HXX
