#if 0 // dump bytes
inline void dev_dump_memory(Kai_Writer* writer, void* data, Kai_u32 count)
{
    Kai_u8* bytes = data;
    Kai_u32 k = 0;
    for (;;) {
        for (int i = 0; i < 16; ++i) {
            if (k >= count)
                return;

            char map[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                          '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            kai__write_char(map[(bytes[k] >> 4) & 0x0F]);
            kai__write_char(map[(bytes[k] >> 0) & 0x0F]);
            kai__write_char(' ');

            k += 1;
        }
        kai__write_char('\n');
    }
}
#endif

#if 0 // some kind of script manager to simplify hot reloading
typedef struct {
    Kai_s64 last_modified;
    Kai_string path;
    Kai_string source_code;
} Kai_Script_Status;

typedef struct {
    Kai_Allocator                 allocator;
    KAI__ARRAY(Kai_Script_Status) scripts;
} Kai_Script_Manager;
#endif