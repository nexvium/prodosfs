/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOS_BLOCK_HXX
#define PRODOS_BLOCK_HXX

#include <stddef.h>
#include <stdint.h>

namespace prodos
{
const int   BLOCK_SIZE          = 512;

struct directory_header_t
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

struct file_entry_t
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

struct directory_block_t
{
    uint8_t                     prev[2];
    uint8_t                     next[2];
    union
    {
        struct
        {
            directory_header_t  header;
            file_entry_t        entry[14];
        } key;
        struct
        {
            file_entry_t        entry[15];
        } any;
    };
    uint8_t                     unused;

public:
    static const directory_block_t *  Create(const void *addr);

    directory_block_t(void)                                     = delete;
    directory_block_t(const directory_block_t &)                = delete;
    directory_block_t & operator=(const directory_block_t &)    = delete;
    ~directory_block_t()                                        = delete;
};

} // namespace

#endif // PRODOS_BLOCK_HXX
