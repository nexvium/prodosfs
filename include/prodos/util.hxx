/*
** prodosfs - A read-only ProDOS 8 implementation as a FUSE filesystem.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_UTIL_HXX
#define PRODOSFS_UTIL_HXX

namespace prodos
{

template <typename T, typename D>
class liberator_t
{
public:
    liberator_t(T value, D liberator): _value(value), _liberator(liberator) { }
    ~liberator_t() { _liberator(_value); }
    liberator_t(const liberator_t &)                = delete;
    liberator_t &   operator=(const liberator_t &)  = delete;

private:
    T   _value;
    D   _liberator;
};

enum log_level_t
{
    LOG_CRITICAL,   // 0
    LOG_ERROR,      // 1
    LOG_WARNING,    // 2
    LOG_INFO,       // 3
    LOG_VERBOSE,    // 4
    LOG_DEBUG1,     // 5: per fuse call
    LOG_DEBUG2,     // 6: per disk file
    LOG_DEBUG3,     // 7: > per disk file
    LOG_MAX = 99
};

typedef void (*Logger)(int level, const char *format, ...);

extern Logger LOG;

void SetLogger(Logger func);

} // namespace

#endif // PRODOSFS_UTIL_HXX