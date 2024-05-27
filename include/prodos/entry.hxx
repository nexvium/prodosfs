/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#ifndef PRODOSFS_ENTRY_HXX
#define PRODOSFS_ENTRY_HXX

#include "prodos/block.hxx"

#include <string>

namespace prodos
{

class context_t;

struct timestamp_t
{
    int     year;
    int     month;
    int     day;
    int     hour;
    int     minute;
};

class entry_t
{
public:
    uint8_t         StorageType()           const;
    uint8_t         NameLength()            const;
    std::string     FileName()              const;
    timestamp_t     CreationTimestamp()     const;
    uint8_t         Version()               const;
    uint8_t         MinVersion()            const;
    uint8_t         Access()                const;

    bool            IsFile()                            const;
    bool            IsDirectory()                       const;
    bool            IsHeader()                          const;
    bool            IsInactive()                        const;
    bool            IsRoot()                            const;

    bool            NameMatches(const std::string & name)   const;
};

class directory_entry_t : public entry_t
{
public:
    uint8_t         FileType()          const;
    uint16_t        KeyPointer()        const;
    uint16_t        BlocksUsed()        const;
    uint32_t        Eof()               const;
    uint16_t        AuxType()           const;
    timestamp_t     LastModTimestamp()  const;
};

class directory_header_t : public entry_t
{
public:
    uint8_t     EntryLength()       const;
    uint8_t     EntriesPerBlock()   const;
    uint16_t    FileCount()         const;

    static const directory_header_t *    Create(const directory_header * header);
};

class volume_header_t : public entry_t
{
public:
    uint8_t     EntryLength()       const;
    uint8_t     EntriesPerBlock()   const;
    uint16_t    FileCount()         const;
    uint16_t    TotalBlocks()       const;

    static const volume_header_t *   Create(const void * header);
};


} // namespace

#endif // PRODOSFS_ENTRY_HXX
