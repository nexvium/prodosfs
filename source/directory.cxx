/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/directory.hxx"

#include "prodos/volume.hxx"
#include "prodos/util.hxx"

#include <stdexcept>

namespace prodos
{

extern thread_local err_t error;

directory_handle_t::directory_handle_t(const volume_t * context, const directory_block * key_block)
    : _context(context), _header(nullptr), _block(nullptr), _block_index(1), _entry_index(0)
{
    _Open(key_block);
}

const directory_entry_t *
directory_handle_t::NextEntry()
{
    auto file_count = LE_Read16(_header->file_count);
    if (_entry_index == file_count) {
        error = err_end_of_file;
        return nullptr;
    }

    auto entry = (const directory_entry_t *)&_block->any.entry[_block_index];
    while (entry->IsInactive()) {
        _Next();
        entry = (directory_entry_t *)&_block->any.entry[_block_index];
    }
    _entry_index++;
    _Next();

    return entry;
}

void
directory_handle_t::Close()
{
    _header = nullptr;
    _block = nullptr;
    _block_index = 0;
    _entry_index = 0;
}

void
directory_handle_t::_Open(const directory_block * key_block)
{
    int type = key_block->key.header.storage_type_and_name_length >> 4;
    if (type != storage_type_volume_block && type != storage_type_subdir_block) {
        throw std::runtime_error("not a directory key block");
    }

    _header = &key_block->key.header;
    _block = key_block;
    _block_index = 1;
    _entry_index = 0;
}

void
directory_handle_t::_Next()
{
    if (_block_index < _header->entries_per_block) {
        _block_index++;
    }
    else {
        auto next_block = LE_Read16(_block->next);
        if (next_block == 0) {
            throw std::runtime_error("no more directory blocks");
        }

        _block = (const directory_block *)_context->GetBlock(next_block);
        _block_index = 0;
    }
}

} // namespace

// eof
