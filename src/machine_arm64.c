#include <stdint.h>

/*
extern inline uint32_t kai__arm64_add(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b0001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_sub(uint32_t Rd, uint32_t Rn, uint32_t Rm, uint8_t sf) { return (sf << 31) | (0b1001011 << 24) | (Rn << 5) | (Rm << 16) | Rd; }
extern inline uint32_t kai__arm64_subs(uint32_t imm12, uint32_t Rn, uint8_t sf) { return (sf << 31) | (0b11100010 << 23) | (imm12 << 10) | (Rn << 5) | 0b11111; }
extern inline uint32_t kai__arm64_movz(uint32_t Rd, uint32_t imm16, uint8_t sf) { return (sf << 31) | (0b00100101 << 23) | (imm16 << 5) | Rd; }
extern inline uint32_t kai__arm64_bl(uint32_t imm26) { return (0b100101 << 26) | imm26; }
extern inline uint32_t kai__arm64_ret() { return 0xd65f03c0; }
*/
