/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/file.hxx"

#include <stdexcept>

namespace prodos
{

off_t
file_handle_t::Seek(off_t offset, int whence)
{
    throw std::logic_error("not implemented");
}

void
file_handle_t::Close()
{
    throw std::logic_error("not implemented");
}

size_t
file_handle_t::Read(void *buffer, size_t size) const
{
    throw std::logic_error("not implemented");
}

bool
file_handle_t::Eof() const
{
    throw std::logic_error("not implemented");
}

} // namespace

// eof
