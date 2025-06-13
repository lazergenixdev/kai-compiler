#include "../include/kai.h"
#ifndef X86_64
// Each x86 64-bit instruction can be up to 16 bytes (or more possibly.. idk)
// so the caller will need to make sure that at least this amount of
// space is available before each function call

// ------------------------------------ Windows -- x86_64 Calling Convention ------------------------------------
// Ref: https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170
// 
// First 4 arguments are always put in registers
//    -> RCX, RDX, R8, R9        (integers)
//    -> XMM0, XMM1, XMM2, XMM3  (floats)
// Rest of the arguments (5+) are pushed onto the stack
// 
// If return value is more than 64-bit, then caller allocates
//     space for it and passes pointer as the first argument
//
// Caller saved:
//   RAX, RCX, RDX, R8, R9, R10, R11, XMM0-XMM5
// Callee saved:
//   RBX, RBP, RDI, RSI, RSP, R12, R13, R14, R15, XMM6-XMM15
// --------------------------------------------------------------------------------------------------------------
#endif

#ifndef ARM64
extern inline uint32_t kai__arm64_add(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b0001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_sub(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b1001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_subs(uint32_t imm12, uint32_t Rn, uint8_t sf) { return (sf << 31) | (0b11100010 << 23) | (imm12 << 10) | (Rn << 5) | 0b11111; }
extern inline uint32_t kai__arm64_movz(uint32_t Rd, uint32_t imm16, uint8_t sf) { return (sf << 31) | (0b00100101 << 23) | (imm16 << 5) | Rd; }
extern inline uint32_t kai__arm64_bl(uint32_t imm26) { return (0b100101 << 26) | imm26; }
extern inline uint32_t kai__arm64_ret() { return 0xd65f03c0; }
#endif
