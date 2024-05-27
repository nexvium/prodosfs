/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_FILE_HXX
#define PRODOSFS_FILE_HXX

#include <stdint.h>
#include <sys/types.h>

namespace prodos
{

class context_t;
class directory_entry_t;

class index_block_t
{
public:
    uint16_t    At(int index) const;
    uint16_t    operator[](int index)   const   { return At(index); }
};

class file_handle_t
{
public:
    void                Close();
    uint8_t             Type() const;
    bool                Eof() const;
    off_t               Seek(off_t offset, int whence);
    size_t              Read(void *buffer, size_t size);

private:
    const context_t *           _context{};
    const directory_entry_t *   _entry{};
    index_block_t *             _master{};
    index_block_t *             _index{};
    uint8_t *                   _data{};
    off_t                       _position{};

    file_handle_t(const context_t * context, const directory_entry_t * entry);

    friend class context_t;
};

} // namespace

#endif // PRODOSFS_FILE_HXX
