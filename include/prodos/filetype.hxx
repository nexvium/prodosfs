/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_FILETYPE_HXX
#define PRODOSFS_FILETYPE_HXX

#include <string>

#include <stdint.h>

namespace prodos
{

struct file_type_info_t
{
    const std::string     type;
    const std::string     name;
    const std::string     description;
};

// Create constants for the most common/important file types.
enum file_type_t
{
    file_type_none              = 0x00,
    file_type_text              = 0x04,
    file_type_binary            = 0x06,
    file_type_directory         = 0x0F,
    file_type_appleworks_db     = 0x19,
    file_type_appleworks_wp     = 0x1A,
    file_type_appleworks_ss     = 0x1B,
    file_type_pascal            = 0xEF,
    file_type_command           = 0xF0,
    file_type_integer_basic     = 0xFA,
    file_type_applesoft_basic   = 0xFC,
    file_type_prodos_system     = 0xFF,
};


const file_type_info_t *    GetFileTypeInfo(uint8_t type);

bool                        IsAppleWorksFile(uint8_t type);

}

#endif // PRODOSFS_FILETYPE_HXX
