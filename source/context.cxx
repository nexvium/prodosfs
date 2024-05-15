/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/context.hxx"

namespace prodos
{

context_t::context_t(const std::string & pathname)
    : _disk(pathname)
{

}

} // namespace

// eof
