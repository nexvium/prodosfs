/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/block.hxx"

#include "prodos/context.hxx"
#include "prodos/util.hxx"

#include <stdexcept>

namespace prodos
{
//
//static bool
//L_IsDirKeyBlock(directory_block * block)
//{
//    auto type = block->key.header.storage_type_and_name_length >> 4;
//    return (type == storage_type_subdir_block || type == storage_type_volume_block)
//            ? true
//            : false;
//}
//
//const directory_block *
//directory_block::Create(const void *addr)
//{
//    auto block = (directory_block *)addr;
//    if (LE_Read16(block->prev) != 0 || L_IsDirKeyBlock(block) == false) {
//        return nullptr;
//    }
//
//    return block;
//}
//
//uint16_t
//directory_block::TotalBlocks(void) const
//{
//    return LE_Read16(this->key.header.total_blocks);
//}

} // namespace

// eof
