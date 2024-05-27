/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_SYSTEM_HXX
#define PRODOSFS_SYSTEM_HXX

#include "prodos/directory.hxx"
#include "prodos/disk.hxx"
#include "prodos/entry.hxx"
#include "prodos/file.hxx"

//#include <string>

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
    err_file_access_error           = 0x4E,
    err_directory_structure_damaged = 0x51,
    err_file_structure_damaged      = 0x54,

    err_prodosfs_not_a_directory    = 0xFF,
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

enum file_type_t
{
    file_type_none              = 0x00,
    file_type_text              = 0x04,
    file_type_binary            = 0x06,
    file_type_directory         = 0x0F,
    file_type_appleworks_db     = 0x19,
    file_type_appleworks_wp     = 0x20,
    file_type_appleworks_ss     = 0x21,
    file_type_integer_basic     = 0xFA,
    file_type_applesoft_basic   = 0xFC,
    file_type_prodos_system     = 0xFF,
};

class context_t
{
public:
    explicit context_t(const std::string & pathname);
    context_t(const context_t &)                = delete;
    context_t(const context_t &&)               = delete;

    ~context_t()                                = default;

    std::string             GetVolumeName() const;

    // Returns the directory entry for the given pathname, if found,
    // EXCEPT when the pathname is "/", in which case the volume
    // directory header is returned.
    const entry_t *         GetEntry(const std::string & pathname) const;

    directory_handle_t *    OpenDirectory(const std::string & pathname) const;

    file_handle_t *         OpenFile(const std::string & pathname) const;

    static err_t            Error();
    const void *            GetBlock(int index) const;
    int                     GetBlocksUsed(const entry_t * entry) const;

    int                     CountVolumeBlocksUsed()         const;
    int                     CountVolumeDirectoryBlocks()    const;

private:
    disk_t                      _disk;
    const directory_block *     _root;

    const directory_block *     _GetVolumeDirectoryBlock();
};

} // namespace

#endif // PRODOSFS_SYSTEM_HXX
