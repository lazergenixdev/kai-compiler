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

#define __temp(X) \
  { X, sizeof(X) - 1 }

static struct {
    Kai_str  name;
    Kai_Type info;
} builtin_types [] = {
    { .name = __temp("Type"), .info = (Kai_Type)&kai__type_info_type },
    { .name = __temp("s8"),   .info = (Kai_Type)&kai__type_info_s8   },
    { .name = __temp("s16"),  .info = (Kai_Type)&kai__type_info_s16  },
    { .name = __temp("s32"),  .info = (Kai_Type)&kai__type_info_s32  },
    { .name = __temp("s64"),  .info = (Kai_Type)&kai__type_info_s64  },
    { .name = __temp("u8"),   .info = (Kai_Type)&kai__type_info_u8   },
    { .name = __temp("u16"),  .info = (Kai_Type)&kai__type_info_u16  },
    { .name = __temp("u32"),  .info = (Kai_Type)&kai__type_info_u32  },
    { .name = __temp("u64"),  .info = (Kai_Type)&kai__type_info_u64  },
    { .name = __temp("f32"),  .info = (Kai_Type)&kai__type_info_f32  },
    { .name = __temp("f64"),  .info = (Kai_Type)&kai__type_info_f64  },
};

#endif // BUILTIN_TYPES_H