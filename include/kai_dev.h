/**
 *  FILE IS ONLY MEANT FOR DEVELOPING THE KAI LANGUAGE,
 *  THIS IS NOT AN OFFICIAL PART OF THE API!
 */
#ifndef KAI_DEV__H
#define KAI_DEV__H
#define KAI_USE_DEBUG_API
#include "kai.h"

static void dev_dump_memory(Kai_Debug_String_Writer* writer, void* data, Kai_u32 count)
{
    Kai_u8* bytes = data;
    Kai_u32 k = 0;
    for (;;) {
        for (int i = 0; i < 16; ++i) {
            if (k >= count)
                return;

            char map[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                          '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            writer->write_char(writer->user, map[(bytes[k] >> 4) & 0x0F]);
            writer->write_char(writer->user, map[(bytes[k] >> 0) & 0x0F]);
            writer->write_char(writer->user, ' ');

            k += 1;
        }
        writer->write_char(writer->user, '\n');
    }
}

#endif //KAI_DEV__H
