#include <stdint.h>

// To be continued...

// ... one day

// ... not today

#define kai__arm64_opcode(x) ((x) << 24)

uint32_t kai__arm64_add_u32(uint32_t Rd, uint32_t Rn, uint32_t Rm) {
    return kai__arm64_opcode(0b00001011) | (Rd << 0) | (Rn << 5) | (Rm << 16);
}

uint32_t kai__arm64_add_u64(uint32_t Rd, uint32_t Rn, uint32_t Rm) {
    return kai__arm64_opcode(0b10001011) | (Rd << 0) | (Rn << 5) | (Rm << 16);
}

#define kai__arm64_ret() 0xd65f03c0
