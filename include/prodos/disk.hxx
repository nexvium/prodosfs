/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_DISK_HXX
#define PRODOSFS_DISK_HXX

#include <string>

namespace prodos
{

class disk_t
{
public:
    const size_t SECTOR_SIZE = 256;
    const size_t BLOCK_SIZE = SECTOR_SIZE * 2;

    disk_t(const std::string &pathname);
    disk_t(const disk_t &)                  = delete;
    disk_t(const disk_t &&)                 = delete;

    virtual ~disk_t();

    disk_t &    operator=(const disk_t &)   = delete;

    const void *GetBlock(int index) const;

    const void *GetTrackSector(int track, int sector) const;

    enum convert_t { RWTS_TO_BLOCK };
    void Convert(convert_t direction);

    ssize_t ToOffset(const void *addr) const;

private:
    void *      _base;
    size_t      _size;
    unsigned    _num_blocks;
    bool        _converted;

    void _ReadRwtsBlock(size_t index, void *block);
};

} // namespace

#endif // PRODOSFS_DISK_HXX
