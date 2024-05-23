/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/directory.hxx"

#include "prodos/context.hxx"
#include "prodos/util.hxx"

#include <stdexcept>

namespace prodos
{

directory_handle_t::directory_handle_t(const context_t * context, const directory_block * key_block)
    : _context(context), _header(nullptr), _block(nullptr), _block_index(1), _entry_index(0)
{
    Open(key_block);
}

void
directory_handle_t::Open(const directory_block * key_block)
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

const directory_entry_t *
directory_handle_t::NextEntry()
{
    auto file_count = LE_Read16(_header->file_count);
    if (_entry_index == file_count) {
        return nullptr;
    }

    auto entry = (directory_entry_t *)&_block->any.entry[_block_index];
    _entry_index++;
    if (_entry_index == file_count) {
        return nullptr;
    }

    do {
        if (_block_index < ENTRIES_PER_BLOCK) {
            _block_index++;
        }
        else {
            auto next_block = LE_Read16(_block->next);
            if (next_block == 0 && _entry_index != file_count) {
                throw std::runtime_error("no more directory blocks");
            }

            _block = (directory_block *)_context->GetBlock(next_block);
            _block_index = 0;
        }
    } while (_block->any.entry[_block_index].storage_type_and_name_length >> 4 == 0);

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

} // namespace

// eof
