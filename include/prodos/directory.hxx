/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_DIRECTORY_HXX
#define PRODOSFS_DIRECTORY_HXX

namespace prodos
{

struct directory_block;
struct directory_header;

class context_t;
class disk_t;
class directory_entry_t;

class directory_handle_t
{
public:
    void                    Close();

    const directory_entry_t *    NextEntry();

private:
    const context_t *           _context;
    const directory_header *    _header;
    const directory_block *     _block;
    int                         _block_index;
    int                         _entry_index;

    friend class context_t;

    directory_handle_t(const context_t * context, const directory_block * key_block);

    void    Open(const directory_block * key_block);
};

} // namespace

#endif // PRODOSFS_DIRECTORY_HXX
