#include "../include/kai.h"
#ifndef X86_64
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

#ifndef ARM64
extern inline uint32_t kai__arm64_add(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b0001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_sub(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b1001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_subs(uint32_t imm12, uint32_t Rn, uint8_t sf) { return (sf << 31) | (0b11100010 << 23) | (imm12 << 10) | (Rn << 5) | 0b11111; }
extern inline uint32_t kai__arm64_movz(uint32_t Rd, uint32_t imm16, uint8_t sf) { return (sf << 31) | (0b00100101 << 23) | (imm16 << 5) | Rd; }
extern inline uint32_t kai__arm64_bl(uint32_t imm26) { return (0b100101 << 26) | imm26; }
extern inline uint32_t kai__arm64_ret() { return 0xd65f03c0; }
#endif
