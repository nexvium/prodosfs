/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_DIRECTORY_HXX
#define PRODOSFS_DIRECTORY_HXX

namespace prodos
{

struct directory_block;
struct directory_header;

class volume_t;
class disk_t;
class directory_entry_t;

class directory_handle_t
{
public:
    void                        Close();
    const directory_entry_t *   NextEntry();

private:
    const volume_t *            _context;
    const directory_header *    _header;
    const directory_block *     _block;
    int                         _block_index;
    int                         _entry_index;

    friend class volume_t;

    directory_handle_t(const volume_t * context, const directory_block * key_block);

    void    _Open(const directory_block * key_block);
    void    _Next();
};

} // namespace

#endif // PRODOSFS_DIRECTORY_HXX
