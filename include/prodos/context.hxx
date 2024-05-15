/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_SYSTEM_HXX
#define PRODOSFS_SYSTEM_HXX

#include "prodos/disk.hxx"

#include <string>

namespace prodos
{

const size_t    NAME_MAX_LEN    = 15;

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

    virtual ~context_t();

    std::string     GetVolumeName(void) const;

    entry_t *       GetEntry(const std::string & pathname) const;

    directory_t *   OpenDirectory(const std::string & pathname) const;

    file_t *        OpenFile(const std::string & pathname) const;

    err_t           Error(void) const;

private:
    disk_t  _disk;
};

} // namespace

#endif // PRODOSFS_SYSTEM_HXX
