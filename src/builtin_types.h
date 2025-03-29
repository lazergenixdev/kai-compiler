#ifndef BUILTIN_TYPES__H
#define BUILTIN_TYPES__H
#include "config.h"

static Kai_Type_Info         kai__type_info_type = { .type = KAI_TYPE_TYPE };
static Kai_Type_Info_Integer kai__type_info_s8   = { .type = KAI_TYPE_INTEGER, .bits = 8,  .is_signed = KAI_TRUE };
static Kai_Type_Info_Integer kai__type_info_s16  = { .type = KAI_TYPE_INTEGER, .bits = 16, .is_signed = KAI_TRUE };
static Kai_Type_Info_Integer kai__type_info_s32  = { .type = KAI_TYPE_INTEGER, .bits = 32, .is_signed = KAI_TRUE };
static Kai_Type_Info_Integer kai__type_info_s64  = { .type = KAI_TYPE_INTEGER, .bits = 64, .is_signed = KAI_TRUE };
static Kai_Type_Info_Integer kai__type_info_u8   = { .type = KAI_TYPE_INTEGER, .bits = 8,  .is_signed = KAI_FALSE };
static Kai_Type_Info_Integer kai__type_info_u16  = { .type = KAI_TYPE_INTEGER, .bits = 16, .is_signed = KAI_FALSE };
static Kai_Type_Info_Integer kai__type_info_u32  = { .type = KAI_TYPE_INTEGER, .bits = 32, .is_signed = KAI_FALSE };
static Kai_Type_Info_Integer kai__type_info_u64  = { .type = KAI_TYPE_INTEGER, .bits = 64, .is_signed = KAI_FALSE };
static Kai_Type_Info_Float   kai__type_info_f32  = { .type = KAI_TYPE_FLOAT, .bits = 32 };
static Kai_Type_Info_Float   kai__type_info_f64  = { .type = KAI_TYPE_FLOAT, .bits = 64 };

static struct {
    Kai_str  name;
    Kai_Type type;
} kai__builtin_types [] = {
    { KAI_CONSTANT_STRING("Type"), (Kai_Type)&kai__type_info_type },
    { KAI_CONSTANT_STRING("s8"),   (Kai_Type)&kai__type_info_s8   },
    { KAI_CONSTANT_STRING("s16"),  (Kai_Type)&kai__type_info_s16  },
    { KAI_CONSTANT_STRING("s32"),  (Kai_Type)&kai__type_info_s32  },
    { KAI_CONSTANT_STRING("s64"),  (Kai_Type)&kai__type_info_s64  },
    { KAI_CONSTANT_STRING("u8"),   (Kai_Type)&kai__type_info_u8   },
    { KAI_CONSTANT_STRING("u16"),  (Kai_Type)&kai__type_info_u16  },
    { KAI_CONSTANT_STRING("u32"),  (Kai_Type)&kai__type_info_u32  },
    { KAI_CONSTANT_STRING("u64"),  (Kai_Type)&kai__type_info_u64  },
    { KAI_CONSTANT_STRING("f32"),  (Kai_Type)&kai__type_info_f32  },
    { KAI_CONSTANT_STRING("f64"),  (Kai_Type)&kai__type_info_f64  },
};

#endif // BUILTIN_TYPES__H