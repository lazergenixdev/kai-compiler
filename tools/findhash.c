#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../3rd-party/nob.h"
#include "stdio.h"

#define len(A) (int)(sizeof(A)/sizeof(A[0]))
#define BOOL(B) ((B)? 1 : 0)

const char* keywords[] = {
    "break",
    "case",
    "cast",
    "continue",
    "defer",
    "else",
    "enum",
    "false",
    "for",
    "if",
    "null",
    "ret",
    "struct",
    "then",
    "true",
    "union",
    "using",
    "while",
};

typedef struct {
    int* data;
    int count;
} int_array;

typedef struct {
    uint32_t* data;
    int count;
} uint_array;

bool FindInt(int_array array, int num)
{
    for (int i = 0; i < array.count; ++i)
    {
        if (array.data[i] == num)
            return true;
    }
    return false;
}

void PrintStringAsBinary(const char* string, int_array colored)
{
    uint8_t* bytes = (uint8_t*)string;
    int length = strlen(string);
    for (int i = 0; i < length; ++i)
    {
        uint8_t byte = bytes[i];
        for (int k = 0; k < 8; ++k)
        {
            uint8_t bit = 1 << (8 - k);
            bool found = FindInt(colored, i * 8 + k);
            if (found) {
                printf("\x1b[91m");
            }
            else {
                printf("\x1b[0m");
            }
            putchar(byte & bit ? '1' : '0');
        }
    }
}

bool InsertUintUnique(uint_array* array, uint32_t value)
{
    for (int i = 0; i < array->count; ++i)
    {
        if (array->data[i] == value)
            return false;
    }
    array->data[array->count++] = value;
    return true;
}

uint32_t Hash1(const char* string, int param)
{
    (void)param;
    uint32_t hash = 5381;
    int length = strlen(string);
    for (int i = 0; i < length; ++i)
        hash = ((hash << 5) + hash) + string[i]; /* hash * 33 + c */
    return hash;
}

uint32_t Hash2(const char* string, int param)
{
    (void)param;
    return (uint32_t)strlen(string)*3 + ((uint32_t)string[0]) + ((uint32_t)string[strlen(string)-1]) * 13;
}

uint32_t Hash3(const char* string, int param)
{
    (void)param;
    uint32_t hash = 1;
    int length = strlen(string);
    for (int i = 0; i < length && i < 8; ++i) {
        uint32_t value = string[i];
        hash ^= (value & 0xF) << i;
    }
    return hash;
}

uint32_t Hash4(const char* string, int param)
{
    (void)param;
    uint32_t hash = string[0];
    int length = strlen(string);
    for (int i = 1; i < length && i < 8; ++i) {
        uint32_t value = string[i];
        hash ^= (value & 0xF) << (i+6);
    }
    return hash;
}

int FindMinimumHashTableSize(uint32_t (*hash)(const char*, int))
{
    int size = len(keywords);
    int p = 0;
    uint_array unique = {.data = malloc(1000)};
    for (; size <= 256; size += 1)
    {
        unique.count = 0;
        for (int i = 0; i < len(keywords); ++i)
        {
            const char* s = keywords[i];
            uint32_t h = hash(s,p) % size;
            if (!InsertUintUnique(&unique, h))
            {
                break;
            }
        }

        if (unique.count == len(keywords))
            return size;
    }

    return -1;
}

typedef struct {
    int a, b, c, d, e, f;
} parameters;

#define for_constant_range(i) for (int i = 0; i < 16; ++i)

uint32_t BruteForceHash(const char* string, parameters bits)
{
    int length = strlen(string);
    #define assign(VAR) uint32_t VAR = (((bits.VAR / 4) < length) ? (string[bits.VAR / 4] >> (3 - bits.VAR%4)) & 1 : 0)
    
    assign(a);
    assign(b);
    assign(c);
    assign(d);
    assign(e);
    assign(f);

    return (a << 5) | (b << 4) | (c << 3) | (d << 2) | (e << 1) | f;
}

typedef struct {
    parameters bits;
    int size;
} brute_force_hash;

brute_force_hash FindBestHashBruteForce(void)
{
    uint_array unique = {.data = malloc(1000)};
    int min_size = 10000;
    parameters min_bits = {0};

    for_constant_range(a)
    for_constant_range(b)
    for_constant_range(c)
    for_constant_range(d)
    for_constant_range(e)
    for_constant_range(f)
    {
        parameters bits = {a, b, c, d, e, f};
        for (int size = len(keywords); size <= 32; ++size)
        {
            unique.count = 0;
            for (int i = 0; i < len(keywords); ++i)
            {
                const char* s = keywords[i];
                uint32_t h = BruteForceHash(s, bits) % size;
                if (!InsertUintUnique(&unique, h))
                {
                    break;
                }
            }

            if (unique.count == len(keywords) && size <= 21)
            printf(": found size=%i  parameters: %i:%i, %i:%i, %i:%i, %i:%i, %i:%i, %i:%i\n", size,
                bits.a/4, bits.b/4, bits.c/4, bits.d/4, bits.e/4, bits.f/4,
                bits.a%4, bits.b%4, bits.c%4, bits.d%4, bits.e%4, bits.f%4
            );

            if (unique.count == len(keywords) && size <= min_size)
            {
                min_size = size;
                min_bits = bits;
            }
        }
    }

    if (min_size <= 32)
    {
        return (brute_force_hash){
            .bits = min_bits,
            .size = min_size
        };
    }

    return (brute_force_hash){0};
}

// FOUND via FindBestHashBruteForce
uint32_t best_keyword_hash(const char* string)
{
    int length = strlen(string);

    // 11, 9, 4, 1, 8, 15
    // 18
    uint32_t hash = 0;
    if (0 < length) hash |= ((string[0] >> 2) & 1) << 2; // d
    if (1 < length) hash |= ((string[1] >> 3) & 1) << 3; // c
    if (2 < length) hash |= ((string[2] >> 3) & 1) << 1; // e
    if (2 < length) hash |= ((string[2] >> 2) & 1) << 4; // b
    if (2 < length) hash |= ((string[2]) & 1) << 5; // a
    if (3 < length) hash |= (string[3] & 1); // f
    return hash % 18; 
}

int main(int argc, char** argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    int_array colored = {
        .data = malloc(10000)
    };

    int max_length = 0;
    for (int i = 0; i < len(keywords); ++i)
    {
        int length = strlen(keywords[i]);
        max_length = length > max_length ? length : max_length;
    }

    for (int i = 0; i < max_length * 8; ++i)
    {
        int found = -1;
        for (int j = 0; j < len(keywords); ++j)
        {
            int length = strlen(keywords[j]);
            if (i/8 >= length) continue;

            uint8_t byte = ((uint8_t*)keywords[j])[i/8];
            uint8_t bit = 1 << (8 - (i%8));

            if (found == -1) {
                found = BOOL(byte & bit);
            }
            else if (found != BOOL(byte & bit))
            {
                colored.data[colored.count++] = i;
                break;
            }
        }
    }

    for (int i = 0; i < len(keywords); ++i)
    {
        const char* s = keywords[i];
        printf("\x1b[0m%10s ", s);
        PrintStringAsBinary(s, colored);
        putchar('\n');
    }
    printf("\n");

    for (int i = 0; i < len(keywords); ++i)
    {
        const char* s = keywords[i];
        printf("\x1b[0m%10s %2i %08x %08x %08x\n", s, best_keyword_hash(s), Hash1(s,0), Hash2(s,0), Hash3(s,0));
    }
    printf("\n");

    #define TEST_HASH(H) printf("%s minimum lookup size: %i\n", #H, FindMinimumHashTableSize(H));

    printf("Best possible lookup size: %i\n", len(keywords));
    TEST_HASH(Hash1);
    TEST_HASH(Hash2);
    TEST_HASH(Hash3);
    TEST_HASH(Hash4);

    printf("\n");
    const char* strings[18] = {0};

    for (int i = 0; i < len(keywords); ++i)
    {
        const char* s = keywords[i];
        uint32_t index = best_keyword_hash(s);
        strings[index] = s;
    }

    for (int i = 0; i < len(strings); ++i)
    {
        const char* s = strings[i];
        printf("\"%s\",\n", s);
    }
    
#if 0
    printf("\n");
    brute_force_hash bfh = FindBestHashBruteForce();
    printf("Best brute-force-hash lookup size: %i\n", bfh.size);
    printf("    parameters: %i, %i, %i, %i, %i, %i\n",
        bfh.bits.a, bfh.bits.b, bfh.bits.c, bfh.bits.d, bfh.bits.e, bfh.bits.f);
#endif
}
