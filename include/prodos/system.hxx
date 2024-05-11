/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_SYSTEM_HXX
#define PRODOSFS_SYSTEM_HXX

#include "prodos/disk.hxx"

#include <string>

namespace prodos
{

class context_t
{
public:
    context_t(const std::string & pathname);
    virtual ~context_t();

    std::string     GetVolumeName(void) const;

private:
    disk_t  _disk;
};

} // namespace

#endif // PRODOSFS_SYSTEM_HXX
