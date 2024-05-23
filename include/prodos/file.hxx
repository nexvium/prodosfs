/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_FILE_HXX
#define PRODOSFS_FILE_HXX

#include <sys/types.h>

namespace prodos
{

class file_handle_t
{
public:
    void                Close();
    size_t              Read(void *buffer, size_t size) const;
    bool                Eof() const;
    off_t               Seek(off_t offset, int whence);
};

} // namespace

#endif // PRODOSFS_FILE_HXX
