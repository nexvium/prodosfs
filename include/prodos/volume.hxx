/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_CONTEXT_HXX
#define PRODOSFS_CONTEXT_HXX

#include "prodos/directory.hxx"
#include "prodos/disk.hxx"
#include "prodos/entry.hxx"
#include "prodos/file.hxx"

namespace prodos
{

/*
** ProDOS error codes. Not all errors may be defined, just the ones this software may encounter and report.
*/
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

/*
** The volume encapsulates an "on-line" (mounted) ProDOS volume.
*/
class volume_t
{
public:
    explicit volume_t(const std::string & pathname);
    volume_t(const volume_t &)                = delete;
    ~volume_t()                               = default;

    std::string     Name()          const;
    int             FileCount()     const;
    int             TotalBlocks()   const;

    // Returns the directory entry for the given pathname, if found,
    // EXCEPT when the pathname is "/", in which case the root
    // directory header is returned.
    const entry_t *         GetEntry(const std::string & pathname) const;

    directory_handle_t *    OpenDirectory(const std::string & pathname) const;

    file_handle_t *         OpenFile(const std::string & pathname) const;

    // Gets the block specified in the index, EXCEPT when the index is 0,
    // in which case it returns a block containing only zeros. This is used
    // when reading sparse files.
    //
    // Block 0 is supposed to contains the ProDOS bootloader, not user data,
    // so it should not be necessary to read the real block.
    const void *    GetBlock(int index) const;

    // These are not stored as data fields, so they really have to be counted.
    int     CountBlocksUsed()           const;
    int     CountRootDirectoryBlocks()  const;

    // Return or clear the last ProDOS error that occurred in the calling thread.
    static err_t            Error();
    static void             ClearError();

private:
    disk_t                      _disk;
    const directory_block *     _root;

    const directory_block *     _GetVolumeDirectoryBlock();
};

} // namespace

#endif // PRODOSFS_CONTEXT_HXX
