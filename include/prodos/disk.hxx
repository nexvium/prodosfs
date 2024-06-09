/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_DISK_HXX
#define PRODOSFS_DISK_HXX

#include <string>

namespace prodos
{

/*
** The disk class deals only with the physical layout of the disk, sectors and blocks, and 
** does not know anything about their contents.
*/
class disk_t
{
public:
    const size_t SECTOR_SIZE = 256;
    const size_t BLOCK_SIZE = SECTOR_SIZE * 2;

    disk_t(const std::string & pathname);
    disk_t(const disk_t &)                  = delete;

    virtual ~disk_t();

    disk_t &    operator=(const disk_t &)   = delete;

    // ProDOS works with sequentially numbered blocks, each of which consists of
    // two not-necessarily sequential sectors.
    const void *    ReadBlock(int index) const;
    void            WriteBlock(int index, const void * block);

    // This returns an individual sector, which is half of some block.
    const void *    ReadTrackSector(int track, int sector) const;

    unsigned        NumBlocks(void) const
    {
        return  _num_blocks;
    }

    // Some disk images are in the older DOS 3.3 track-and-sector format. This converts
    // the image in memory to the block-addressable format that ProDOS expects.
    // 
    // Conversion is only supported in one direction until there's a need for the other.
    enum convert_t { RWTS_TO_BLOCK };
    void Convert(convert_t direction);

    // Given a memory address, this returns the image byte offset, or -1 if not in the image.
    // Used for logging and debugging.
    ssize_t ToOffset(const void * addr) const;

private:
    void *      _base;
    size_t      _size;
    unsigned    _num_blocks;
    bool        _converted;

    void _ReadRwtsBlock(size_t index, void * block);
};

} // namespace

#endif // PRODOSFS_DISK_HXX
