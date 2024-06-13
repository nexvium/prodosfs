/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOS_BLOCK_HXX
#define PRODOS_BLOCK_HXX

/*
** This header describes ProDOS' on-disk block format.
*/

#include <stddef.h>
#include <stdint.h>

namespace prodos
{

const int   BLOCK_SIZE          =      512;
const int   FILENAME_LENGTH     =       15;
const int   ENTRY_LENGTH        =       39;
const int   ENTRIES_PER_BLOCK   =       13;
const int   DATE_TIME_LENGTH    =        4;
const int   FILE_SIZE_MAX       = 0xFFFFFF;

/*
** The directory header is the first entry in a volume or subdirectory key block
** and describes the current directory.
*/
struct directory_header
{
    uint8_t storage_type_and_name_length;
    union
    {
        uint8_t directory_name[15];
        uint8_t volume_name[15];
    };
    uint8_t reserved[8];
    uint8_t creation_date_time[4];
    uint8_t version;
    uint8_t min_version;
    uint8_t access;
    uint8_t entry_length;
    uint8_t entries_per_block;
    uint8_t file_count[2];
    union
    {
        struct
        {
            uint8_t bit_map_pointer[2];
            uint8_t total_blocks[2];
        };
        struct
        {
            uint8_t parent_pointer[2];
            uint8_t parent_entry_number;
            uint8_t parent_entry_length;
        };
    };
};

/*
** A directory entry describes a file or subdirectory in the current directory.
*/
struct directory_entry
{
    uint8_t storage_type_and_name_length;
    uint8_t file_name[15];
    uint8_t file_type;
    uint8_t key_pointer[2];
    uint8_t blocks_used[2];
    uint8_t eof[3];
    uint8_t creation_date_time[4];
    uint8_t version;
    uint8_t min_version;
    uint8_t access;
    uint8_t aux_type[2];
    uint8_t last_mod[4];
    uint8_t header_pointer[2];
};

/*
** A directory block describes the contents of a directory.  The first entry in a key block
** (i.e. the first block for the directory) is a directory header.  All subsequent entries
** are directory entries.
*/
struct directory_block
{
    uint8_t                     prev[2];
    uint8_t                     next[2];
    union
    {
        struct
        {
            directory_header    header;
            directory_entry     entry[14];
        } key;
        struct
        {
            directory_entry     entry[15];
        } any;
    };
    uint8_t                     unused;
};

/*
** An index block contains 16-bit pointers (block numbers) to 256 other blocks.  The pointers are
** split into two arrays of low- and high-order bytes because the 6502 CPU is an 8-bit processor,
** so an index register can only have a value of 0-255.  The 16-bit pointer for the i-th block can
** then be assembled by combining lo[i] and hi[i].
*/
struct index_block
{
    uint8_t lo[256];
    uint8_t hi[256];
};

} // namespace

#endif // PRODOS_BLOCK_HXX
