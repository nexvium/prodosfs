/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/filetype.hxx"

#include <unordered_map>

namespace prodos
{

typedef std::unordered_map<uint8_t, const file_type_info_t>  file_type_table_t;

// Forward declaration for internal table so that it can be
// referenced before it is defined, since it is long.
namespace { extern file_type_table_t  file_type_table; }

const file_type_info_t *
GetFileTypeInfo(uint8_t type)
{
    return &file_type_table[type];
}

bool IsAppleWorksFile(uint8_t type)
{
    return type == file_type_appleworks_wp
        || type == file_type_appleworks_ss
        || type == file_type_appleworks_db;
}

static file_type_info_t
S_CreateFileTypeInfo(uint8_t type, const char * name = nullptr, const char * description = nullptr)
{
    char type_buffer[8] = {};
    sprintf(type_buffer, "$%02X", type);

    char name_buffer[8] = {};
    if (name == nullptr) {
        sprintf(name_buffer, "$%02X", type);
        name = name_buffer;
    }

    char description_buffer[32] = {};
    if (description == nullptr) {
        sprintf(description_buffer, "File type %s file", type_buffer);
        description = description_buffer;
    }

    return file_type_info_t{type_buffer, name, description};
}


namespace
{

// Define the constant file type table last so that it is out of the way of
// the functions that use it.

#define ADD_FILE_TYPE(code,type,desc)   { code, S_CreateFileTypeInfo(code, type, desc) }
file_type_table_t  file_type_table =
{
    ADD_FILE_TYPE(0x00,  "---",     "Typeless file"),
    ADD_FILE_TYPE(0x01,  "BAD",     "Bad block file"),
    ADD_FILE_TYPE(0x02,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x03,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x04,  "TXT",     "Text file"),
    ADD_FILE_TYPE(0x05,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x06,  "BIN",     "Binary file"),
    ADD_FILE_TYPE(0x07,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x08,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x09,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x0A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x0B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x0C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x0D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x0E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x0F,  "DIR",     "Directory file"),
    ADD_FILE_TYPE(0x10,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x11,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x12,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x13,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x14,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x15,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x16,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x17,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x18,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x19,  "ADB",     "AppleWorks database file"),
    ADD_FILE_TYPE(0x1A,  "AWP",     "AppleWorks word processor file"),
    ADD_FILE_TYPE(0x1B,  "ASP",     "AppleWorks spreadsheet file"),
    ADD_FILE_TYPE(0x1C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x1D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x1E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x1F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x20,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x21,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x22,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x23,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x24,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x25,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x26,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x27,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x28,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x29,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x2A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x2B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x2C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x2D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x2E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x2F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x30,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x31,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x32,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x33,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x34,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x35,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x36,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x37,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x38,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x39,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x3A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x3B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x3C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x3D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x3E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x3F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x40,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x41,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x42,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x43,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x44,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x45,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x46,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x47,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x48,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x49,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x4A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x4B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x4C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x4D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x4E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x4F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x50,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x51,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x52,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x53,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x54,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x55,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x56,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x57,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x58,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x59,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x5A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x5B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x5C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x5D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x5E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x5F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x60,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x61,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x62,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x63,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x64,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x65,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x66,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x67,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x68,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x69,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x6A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x6B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x6C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x6D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x6E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x6F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x70,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x71,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x72,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x73,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x74,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x75,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x76,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x77,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x78,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x79,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x7A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x7B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x7C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x7D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x7E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x7F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x80,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x81,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x82,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x83,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x84,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x85,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x86,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x87,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x88,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x89,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x8A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x8B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x8C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x8D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x8E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x8F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x90,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x91,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x92,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x93,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x94,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x95,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x96,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x97,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x98,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x99,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x9A,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x9B,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x9C,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x9D,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x9E,  nullptr,   nullptr),
    ADD_FILE_TYPE(0x9F,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA0,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA1,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA2,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA3,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA4,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA5,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA6,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA7,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA8,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xA9,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xAA,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xAB,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xAC,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xAD,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xAE,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xAF,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB0,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB1,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB2,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB3,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB4,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB5,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB6,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB7,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB8,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xB9,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xBA,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xBB,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xBC,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xBD,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xBE,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xBF,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC0,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC1,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC2,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC3,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC4,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC5,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC6,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC7,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC8,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xC9,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xCA,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xCB,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xCC,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xCD,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xCE,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xCF,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD0,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD1,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD2,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD3,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD4,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD5,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD6,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD7,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD8,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xD9,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xDA,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xDB,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xDC,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xDD,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xDE,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xDF,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE0,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE1,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE2,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE3,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE4,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE5,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE6,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE7,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE8,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xE9,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xEA,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xEB,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xEC,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xED,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xEE,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xEF,  "PAS",     "Pascal file"),
    ADD_FILE_TYPE(0xF0,  "CMD",     "Command file"),
    ADD_FILE_TYPE(0xF1,  "UD1",     "User defined type 1 file"),
    ADD_FILE_TYPE(0xF2,  "UD2",     "User defined type 2 file"),
    ADD_FILE_TYPE(0xF3,  "UD3",     "User defined type 3 file"),
    ADD_FILE_TYPE(0xF4,  "UD4",     "User defined type 4 file"),
    ADD_FILE_TYPE(0xF5,  "UD5",     "User defined type 5 file"),
    ADD_FILE_TYPE(0xF6,  "UD6",     "User defined type 6 file"),
    ADD_FILE_TYPE(0xF7,  "UD7",     "User defined type 7 file"),
    ADD_FILE_TYPE(0xF8,  "UD8",     "User defined type 8 file"),
    ADD_FILE_TYPE(0xF9,  nullptr,   nullptr),
    ADD_FILE_TYPE(0xFA,  "INT",     "Integer BASIC file"),
    ADD_FILE_TYPE(0xFB,  "IVR",     "Integer BASIC variables file"),
    ADD_FILE_TYPE(0xFC,  "BAS",     "Applesoft BASIC file"),
    ADD_FILE_TYPE(0xFD,  "VAR",     "Applesoft BASIC variables file"),
    ADD_FILE_TYPE(0xFE,  "REL",     "Relocatable code file"),
    ADD_FILE_TYPE(0xFF,  "SYS",     "System file"),
};

} // anonymous namespace

} // prodos namespace

// eof
