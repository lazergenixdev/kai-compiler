#pragma once
#include <kai/core.h>


struct Builtin_Type {
    kai_str        name;
    kai_Type_Info* info;    
};


kai_Type_Info _type_type_info{kai_Type_Type};

#define make_int(Name, Bits, Signed) \
kai_Type_Info_Integer                \
_ ## Name ## _type_info = {          \
	.type = kai_Type_Integer,        \
	.bits = Bits,                    \
	.is_signed = Signed,             \
}

make_int(s32, 32, true);
make_int(s64, 64, true);
make_int(u32, 32, false);
make_int(u64, 64, false);

#define make_float(Name, Bits) \
kai_Type_Info_Float            \
_ ## Name ## _type_info = {    \
	.type = kai_Type_Float,    \
	.bits = Bits,              \
}

make_float(f32, 32);
make_float(f64, 64);


#define make_type(Name) \
{ KAI_STR(#Name), (kai_Type)&_ ## Name ## _type_info }

Builtin_Type __builtin_types[] = {
    {KAI_STR("type"), &_type_type_info},
    make_type(s64),
    make_type(f64),
};

#undef make_float
#undef make_int
#undef make_type
