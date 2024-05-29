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

const file_type_info_t *    GetFileTypeInfo(uint8_t type);

}

#endif // PRODOSFS_FILETYPE_HXX
