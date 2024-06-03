/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/entry.hxx"

#include "prodos/context.hxx"
#include "prodos/util.hxx"

#include <stdexcept>

#include <string.h>

using namespace prodos;

static void S_DecodeTimestamp(const uint8_t * ptr, timestamp_t * timestamp)
{
    auto date = LE_Read16(ptr);
    timestamp->day   = (date & 0b00000000'00011111) >> 0;
    timestamp->month = (date & 0b00000001'11100000) >> 5;
    timestamp->year  = (date & 0b11111110'00000000) >> 9;

    timestamp->minute   = ptr[2] & 0b00111111;
    timestamp->hour     = ptr[3] & 0b00011111;
}

namespace prodos
{

bool
IsValidName(const std::string & name)
{
    if (name.length() < 1 || name.length() > 15) {
        return false;
    }

    if (isalpha(name[0]) == false) {
        return false;
    }

    for (auto i = 1; i < name.length(); i++) {
        if (isalnum(name[i]) == false && name[i] != '.') {
            return false;
        }
    }

    return true;
}

//================================================================================================
// timestamp_t
//------------------------------------------------------------------------------------------------

std::string
timestamp_t::AsString() const
{
    const size_t bufsiz = 64;
    static thread_local char buffer[bufsiz] = {};
    static const char * months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                     "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

    sprintf(buffer, "%02d-%3s-%02d %02d:%02d %s",
                    day, months[month - 1], year, hour, minute, hour < 12 ? "AM" : "PM");

    return std::string(buffer);
}

//================================================================================================
// entry_t
//------------------------------------------------------------------------------------------------

uint8_t
entry_t::StorageType() const
{
    auto entry = (const directory_entry *)this;
    return entry->storage_type_and_name_length >> 4;
}

uint8_t
entry_t::NameLength() const
{
    auto entry = (const directory_entry *)this;
    return entry->storage_type_and_name_length & 0x0F;
}

std::string
entry_t::FileName() const
{
    auto entry = (const directory_entry *)this;
    return { (char *)entry->file_name, NameLength() };
}

timestamp_t
entry_t::CreationTimestamp() const
{
    auto entry = (const directory_entry *)this;

    timestamp_t timestamp = {};
    S_DecodeTimestamp(entry->creation_date_time, &timestamp);

    return timestamp;
}

uint8_t
entry_t::Version() const
{
    auto entry = (const directory_entry *)this;
    return entry->version;
}

uint8_t
entry_t::MinVersion() const
{
    auto entry = (const directory_entry *)this;
    return entry->min_version;
}

uint8_t
entry_t::Access() const
{
    auto entry = (const directory_entry *)this;
    return entry->access;

}

bool
entry_t::IsHeader() const
{
    switch (StorageType()) {
        case storage_type_subdir_block:
        case storage_type_volume_block:
            return true;
    }

    return false;
}

bool
entry_t::IsFile() const
{
    switch (StorageType()) {
        case storage_type_seedling_file:
        case storage_type_sapling_file:
        case storage_type_tree_file:
            return true;
    }

    return false;
}

bool
entry_t::IsDirectory() const
{
    return StorageType() == storage_type_subdirectory;
}

bool
entry_t::IsInactive() const
{
    return StorageType() == storage_type_none;
}

bool
entry_t::IsRoot() const
{
    return StorageType() == storage_type_volume_block;
}

bool
entry_t::NameMatches(const std::string & name) const
{
    return strcasecmp(name.c_str(), FileName().c_str()) == 0;
}

//================================================================================================
// directory_entry_t
//------------------------------------------------------------------------------------------------

uint8_t
directory_entry_t::FileType() const
{
    auto entry = (const directory_entry *)this;
    return entry->file_type;
}

uint16_t
directory_entry_t::BlocksUsed() const
{
    auto entry = (const directory_entry *)this;
    return LE_Read16(entry->blocks_used);
}

uint16_t
directory_entry_t::KeyPointer() const
{
    auto entry = (const directory_entry *)this;
    return LE_Read16(entry->key_pointer);
}

uint32_t
directory_entry_t::Eof() const
{
    auto entry = (const directory_entry *)this;
    return LE_Read24(entry->eof);
}

uint16_t
directory_entry_t::AuxType() const
{
    auto entry = (const directory_entry *)this;
    return LE_Read16(entry->aux_type);
}

timestamp_t
directory_entry_t::LastModTimestamp() const
{
    auto entry = (const directory_entry *)this;

    timestamp_t timestamp = {};
    S_DecodeTimestamp(entry->last_mod, &timestamp);

    return timestamp;
}

//================================================================================================
// directory_header_t
//------------------------------------------------------------------------------------------------

const directory_header_t *
directory_header_t::Create(const directory_header *header)
{
    if (header->storage_type_and_name_length >> 4 != storage_type_subdirectory) {
       throw std::runtime_error("not a subdirectory");
    }

    return (const directory_header_t *)header;
}

//================================================================================================
// volume_header_t
//------------------------------------------------------------------------------------------------

const volume_header_t *
volume_header_t::Create(const void * block)
{
    auto header = (const directory_header *)block;
    if (header->storage_type_and_name_length >> 4 != storage_type_volume_block) {
       throw std::runtime_error("not a volume block");
    }

    return (const volume_header_t *)header;
}

uint8_t
volume_header_t::EntryLength() const
{
    auto entry = (const directory_header *)this;
    return entry->entry_length;
}

uint8_t
volume_header_t::EntriesPerBlock() const
{
    auto entry = (const directory_header *)this;
    return entry->entries_per_block;
}

uint16_t
volume_header_t::FileCount() const
{
    auto entry = (const directory_header *)this;
    return LE_Read16(entry->file_count);
}

uint16_t
volume_header_t::TotalBlocks() const
{
    auto entry = (const directory_header *)this;
    return LE_Read16(entry->total_blocks);
}

} // namespace

// eof
