#ifndef BUILTIN_TYPES_H
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
    Kai_Type info;
} builtin_types [] = {
    { .name = KAI_STRING("Type"), .info = (Kai_Type)&kai__type_info_type },
    { .name = KAI_STRING("s8"),   .info = (Kai_Type)&kai__type_info_s8   },
    { .name = KAI_STRING("s16"),  .info = (Kai_Type)&kai__type_info_s16  },
    { .name = KAI_STRING("s32"),  .info = (Kai_Type)&kai__type_info_s32  },
    { .name = KAI_STRING("s64"),  .info = (Kai_Type)&kai__type_info_s64  },
    { .name = KAI_STRING("u8"),   .info = (Kai_Type)&kai__type_info_u8   },
    { .name = KAI_STRING("u16"),  .info = (Kai_Type)&kai__type_info_u16  },
    { .name = KAI_STRING("u32"),  .info = (Kai_Type)&kai__type_info_u32  },
    { .name = KAI_STRING("u64"),  .info = (Kai_Type)&kai__type_info_u64  },
    { .name = KAI_STRING("f32"),  .info = (Kai_Type)&kai__type_info_f32  },
    { .name = KAI_STRING("f64"),  .info = (Kai_Type)&kai__type_info_f64  },
};

#endif // BUILTIN_TYPES_H