/*
** prodosfs - A mountable read-only filesystem for Apple II ProDOS 8 disk images.
**
** Copyright 2024 by Javier Alvarado.
*/

#include "prodos/util.hxx"

#include "prodos/block.hxx"

#include <stdio.h>

namespace prodos
{

static void LogNothing(int level, const char *format, ...) { }

Logger LOG = LogNothing;

void SetLogger(Logger func)
{
    LOG = func;
}

[[maybe_unused]] void DumpBlock(const void *ptr)
{
    auto addr = (const char *)ptr;
    char buffer[128] = {};
    for (int i = 0; i < BLOCK_SIZE; i += 16) {
        fprintf(stderr, "%p | %03x:  ", addr + i, i);
        auto pos = buffer;
        for (int j = 0; j < 16; j++) {
            if (j > 0 && j % 2 == 0)
                sprintf(pos++, " ");
            sprintf(pos, "%02x", addr[i + j]);
            pos += 2;
        }

        sprintf(pos, "  ");
        pos += 2;

        for (int j = 0; j < 16; j++) {
            if (j > 0 && j % 8 == 0)
                sprintf(pos++, " ");

            auto ch = addr[i + j];
            if (ch < 33 || ch > 126)
                sprintf(pos++, ".");
            else
                sprintf(pos++, "%c", ch);
        }

        fprintf(stderr, "%s\n", buffer);
    }
    fprintf(stderr, "\n");
}

} // namespace

// eof
