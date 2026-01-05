#include <stdio.h>
#include <stdint.h>

#define countof(x) (sizeof(x)/sizeof(x[0]))
#define SLICE(T) struct { size_t count; T* data; }

typedef SLICE(uint8_t) string;
#define S(L) (string){.count = sizeof(L)-1, .data = (uint8_t*)(L)}

typedef struct {
    void* type;
    union {
        int32_t s32;
        float f32;
        char* string;
    } data;
} Any;

#define ANY_S32(N) (Any){.type = (void*)(1), .data.s32 = N}
#define ANY_STRING(L) (Any){.type = (void*)(2), .data.string = L}
#define ANY_F32(N) (Any){.type = (void*)(3), .data.f32 = N}

typedef struct {
    Any value;
    uint32_t offset;
    uint8_t min_count;
    uint8_t fill;
    uint8_t flags; // BASE_16, BASE_2, USE_DEFAULT = 1
} Format_Fragment;

typedef SLICE(Format_Fragment) Format_Fragment_Slice;

typedef struct {
    string format;
    Format_Fragment_Slice fragments;
} Formatted_String;

string string_slice(string s, int start, int end) {
    if (end > s.count) end = s.count;
    return (string){.count = end - start, .data = s.data + start};
}

void print_string(string s) {
    // printf("%.*s", s);
    printf("%.*s", (int)(s.count), s.data);
}

void print_fragment(Format_Fragment fragment) {
    switch ((intptr_t)(fragment.value.type)) {
        case 1: {
            printf("%i", fragment.value.data.s32);
        } break;
        case 2: {
            printf("%s", fragment.value.data.string);
        } break;
        case 3: {
            printf("%f", fragment.value.data.f32);
        } break;
        default: {
            printf("(Invalid Fragment)");
        } break;
    }
}

void print(Formatted_String formatted_string) {
    string format = formatted_string.format;
    Format_Fragment_Slice fragments = formatted_string.fragments;
    int i = 0;
    int s = 0;
    while (i < fragments.count) {
        int offset = fragments.data[i].offset;
        print_string(string_slice(format, s, offset));
        print_fragment(fragments.data[i]);
        i += 1;
        s = offset;
    }
    print_string(string_slice(format, s, INT32_MAX));
}

int main() {
    printf("Format_Fragment: %i bytes\n", (int)sizeof(Format_Fragment));

    Format_Fragment fragments[] = {
        {.value = ANY_F32(3.1415), .offset = 7},
        {.value = ANY_STRING("delicious"), .offset = 12},
    };
    print((Formatted_String){
        .format = S("pie is  and "),
        .fragments = {
            .count = countof(fragments),
            .data = fragments,
        }
    });
    // print($"pie is {noun} and {adjective}")
}
