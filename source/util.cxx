/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/util.hxx"

namespace prodos
{

Logger LOG = nullptr;

void SetLogger(prodos::Logger func)
{
    LOG = func;
}

} // namespace

// eof
