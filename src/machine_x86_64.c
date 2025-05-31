#include "kai.h"

#if defined(KAI__MACHINE_X86_64) && 0
// Each x86 64-bit instruction can be up to 16 bytes (or more possibly.. idk)
// so the caller will need to make sure that at least this amount of
// space is available before each function call

Kai_u32 kai__amd64_mov(Kai_u8* dst)
{
    return 0;
}

Kai_u32 kai__amd64_add(Kai_u8* dst)
{
    return 0;
}

Kai_u32 kai__amd64_sub(Kai_u8* dst)
{
    return 0;
}

Kai_u32 kai__amd64_call(Kai_u8* dst, void* address)
{
    return 0;
}

Kai_u32 kai__amd64_ret(Kai_u8* dst)
{
    dst[0] = 0xC3; // TODO: look into using CB (far return)
    return 1;
}
#endif
