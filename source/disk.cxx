/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/disk.hxx"

#include <stdexcept>

#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "prodos/util.hxx"

namespace prodos
{

const int   TRACKS_PER_DISK     = 35;
const int   SECTORS_PER_TRACK   = 16;
const int   BLOCKS_PER_TRACK    = SECTORS_PER_TRACK / 2;

#define BLOCK_ADDR(i)   ((char *)_base + (i) * BLOCK_SIZE)
#define SECTOR_ADDR(i)  ((char *)_base + (i) * SECTOR_SIZE)

disk_t::disk_t(const std::string & pathname)
    : _base(nullptr), _size(0), _num_blocks(0), _converted(false)
{
    int fd = open(pathname.c_str(), O_RDONLY);
    if (fd < 0) {
        throw std::runtime_error(std::string("unable to open image file: ") + strerror(errno));
    }

    struct stat st = {};
    fstat(fd, &st);
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        throw std::runtime_error("image is not a regular file");
    }

    if (st.st_size % BLOCK_SIZE != 0) {
        close(fd);
        throw std::runtime_error("image size is not a multiple of block size");
    }

    _base = mmap(nullptr, st.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (_base == nullptr) {
        close(fd);
        throw std::runtime_error("unable to memory map image file");
    }

    _size = st.st_size;
    _num_blocks = _size / BLOCK_SIZE;

    close(fd);
}

disk_t::~disk_t()
{
    if (_converted) {
        operator delete (_base);
    }
    else {
        munmap(_base, _size);
    }
}

const void *
disk_t::ReadBlock(int index) const
{
    if (index < 0 || index >= _num_blocks) {
        throw std::runtime_error("invalid block number");
    }

    return BLOCK_ADDR(index);
}

void
disk_t::WriteBlock(int index, const void * block)
{
    if (index < 0 || index >= _num_blocks) {
        throw std::runtime_error("invalid block number");
    }

    memcpy(BLOCK_ADDR(index), block, BLOCK_SIZE);
}

const void *
disk_t::ReadTrackSector(int track, int sector) const
{
    if (track < 0 || track >= TRACKS_PER_DISK) {
        throw std::runtime_error("invalid track number");
    }
    else if (sector < 0 || sector >= SECTORS_PER_TRACK) {
        throw std::runtime_error("invalid sector number");
    }

    auto index = track * SECTORS_PER_TRACK + sector;

    return SECTOR_ADDR(index);
}

void
disk_t::_ReadRwtsBlock(size_t index, void * block)
{
    static int map1[] = {  0, 13, 11, 9 ,7, 5, 3,  1 };
    static int map2[] = { 14, 12, 10, 8, 6, 4, 2, 15 };

    auto track = index / BLOCKS_PER_TRACK;
    auto sector1 = map1[index % BLOCKS_PER_TRACK];
    auto sector2 = map2[index % BLOCKS_PER_TRACK];
    auto blk_offset = index * BLOCK_SIZE;
    auto src1_offset = (track * SECTORS_PER_TRACK + sector1) * SECTOR_SIZE;
    auto src2_offset = (track * SECTORS_PER_TRACK + sector2) * SECTOR_SIZE;
    LOG(LOG_DEBUG3, "assembling block %03lu [%06lx] from track %02lu, sectors %02d [%06zx] and %02d [%06zx]",
                    index, blk_offset, track, sector1, src1_offset, sector2, src2_offset);

    memcpy(block, _base + src1_offset, SECTOR_SIZE);
    memcpy(block + SECTOR_SIZE, _base + src2_offset, SECTOR_SIZE);
}

void
disk_t::Convert(convert_t direction)
{
    if (direction != RWTS_TO_BLOCK) {
        throw std::runtime_error("invalid convert direction");
    }

    auto base = operator new (_size);
    for (int i = 0; i < _num_blocks; i++) {
        _ReadRwtsBlock(i, (uint8_t *)base + i * BLOCK_SIZE);
    }

    munmap(_base, _size);
    _base = base;
    _converted = true;
}

ssize_t
disk_t::ToOffset(const void * addr) const
{
    auto offset = (uint8_t *)addr - (uint8_t *)_base;

    if (offset < 0 || offset > _size) {
        throw std::runtime_error("invalid address");
    }

    return offset;
}

} // namespace

// eof
