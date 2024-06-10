/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/file.hxx"

#include "prodos/volume.hxx"
#include "prodos/util.hxx"

#include <stdexcept>

#include <string.h>

namespace prodos
{

extern thread_local err_t error;

uint16_t
index_block_t::At(int index) const
{
    if (index < 0 || index > 255) {
        throw std::logic_error("invalid index");
    }

    auto block = (const index_block *)this;
    return block->hi[index] << 8 | block->lo[index];
}

file_handle_t::file_handle_t(const volume_t *context, const directory_entry_t *entry)
    : _context(context), _entry(entry)
{
    switch (entry->StorageType()) {
    case storage_type_seedling_file:
        _data = (uint8_t *)_context->GetBlock(entry->KeyPointer());
        break;
    case storage_type_sapling_file:
        _index = (index_block_t *)_context->GetBlock(entry->KeyPointer());
        _data = (uint8_t *)_context->GetBlock(_index->At(0));
        break;
    case storage_type_tree_file:
        _master = (index_block_t *)_context->GetBlock(entry->KeyPointer());
        _index = (index_block_t *)_context->GetBlock(_master->At(0));
        _data = (uint8_t *)_context->GetBlock(_index->At(0));
        break;
    default:
        throw std::logic_error("unexpected storage type");
    }
}

void
file_handle_t::Close()
{
    _context    = nullptr;
    _entry      = nullptr;
    _master     = nullptr;
    _index      = nullptr;
    _data       = nullptr;
    _position   = 0;
}

uint8_t
file_handle_t::Type() const
{
    return _entry->FileType();
}

bool
file_handle_t::Eof() const
{
    return _position == _entry->Eof();
}

off_t
file_handle_t::Seek(off_t offset, int whence)
{
    if (whence != SEEK_SET) {
        throw std::logic_error("only SEEK_SET currently implemented");
    }

    if (offset < 0 || offset > _entry->Eof()) {
        error = err_position_out_of_range;
        return -1;
    }

    const size_t BYTES_PER_BLOCK = BLOCK_SIZE;
    const size_t BYTES_PER_INDEX_BLOCK = BLOCK_SIZE * 256;

    _position = offset;
    if (_master) {
        int i = offset / BYTES_PER_INDEX_BLOCK;
        offset -= i * BYTES_PER_INDEX_BLOCK;
        _index = (index_block_t *)_context->GetBlock(_master->At(i));
    }
    if (_index) {
        int i = offset / BYTES_PER_BLOCK;
        _data = (uint8_t *)_context->GetBlock(_index->At(i));
    }

    return _position;
}

size_t
file_handle_t::Read(void *buffer, size_t size)
{
    size_t bytes_read = 0;

    while (size > 0) {
        size_t index = _position % BLOCK_SIZE;
        size_t to_copy = std::min(size, BLOCK_SIZE - index);
        memcpy(buffer, _data + index, to_copy);

        bytes_read += to_copy;
        buffer += to_copy;
        size -= to_copy;

        Seek(_position + to_copy, SEEK_SET);
    }

    return bytes_read;
}

} // namespace

// eof
