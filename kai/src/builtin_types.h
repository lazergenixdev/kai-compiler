#ifndef BUILTIN_TYPES_H
#include <kai/core.h>

static Kai_Type_Info _type_type_info = {.type = KAI_TYPE_TYPE};

#define make_int(NAME, BITS, SIGNED) \
    static Kai_Type_Info_Integer _##NAME##_type_info = { \
              .type = KAI_TYPE_INTEGER, \
              .bits = BITS, \
              .is_signed = SIGNED, \
          }

make_int(u8 ,  8, 0);
make_int(u16, 16, 0);
make_int(u32, 32, 0);
make_int(u64, 64, 0);
make_int(s8 ,  8, 1);
make_int(s16, 16, 1);
make_int(s32, 32, 1);
make_int(s64, 64, 1);

#define make_float(NAME, BITS) \
    static Kai_Type_Info_Float _##NAME##_type_info = { \
              .type = KAI_TYPE_FLOAT, \
              .bits = BITS, \
          }

make_float(f32, 32);
make_float(f64, 64);


typedef struct {
    Kai_str  name;
    Kai_Type info;
} Builtin_Type;

#define KAI_STRL(S) {.count = sizeof(S)-1, .data = (Kai_u8*)S}

#define make_type(NAME) \
    {.name = KAI_STRL(#NAME), .info = (Kai_Type)&_##NAME##_type_info}

static Builtin_Type builtin_types[] = {
    {.name = KAI_STRL("Type"), .info = &_type_type_info},
    make_type(u8 ),
    make_type(u16),
    make_type(u32),
    make_type(u64),
    make_type(s8 ),
    make_type(s16),
    make_type(s32),
    make_type(s64),
    make_type(f32),
    make_type(f64),
};

#undef make_int
#undef make_float
#undef make_type
#endif // BUILTIN_TYPES_H
