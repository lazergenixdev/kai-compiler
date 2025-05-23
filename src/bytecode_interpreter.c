#include "bytecode.h"
//#include <stdio.h>

#define bci__for(N) for (int i = 0; i < (int)N; ++i)

uint8_t bc__type_to_size [16] = {
#define X(name, value, type) [value] = sizeof(type),
    BC_X_PRIMITIVE_TYPES
#undef X
};

int bci__next_u8(Bc_Interpreter* interp, uint8_t* out) {
    if (interp->pc + sizeof(uint8_t) > interp->count) {
        interp->flags |= BC_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    *out = interp->bytecode[interp->pc++];
    return 0;
}
int bci__next_u32(Bc_Interpreter* interp, uint32_t* out) {
    if (interp->pc + sizeof(uint32_t) > interp->count) {
        interp->flags |= BC_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    *out = *(uint32_t*)(interp->bytecode + interp->pc);
    interp->pc += sizeof(uint32_t);
    return 0;
}
int bci__next_address(Bc_Interpreter* interp, uintptr_t* out) {
    if (interp->pc + sizeof(uintptr_t) > interp->count) {
        interp->flags |= BC_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    *out = *(uintptr_t*)(interp->bytecode + interp->pc);
    interp->pc += sizeof(uintptr_t);
    return 0;
}
int bci__next_value(Bc_Interpreter* interp, uint8_t type, Bc_Value* out) {
    uint8_t size = bc__type_to_size[type];
    if (interp->pc + size > interp->count) {
        interp->flags |= BC_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    uint8_t* u8_out = (uint8_t*)(out);
    for (int i = 0; i < size; ++i) {
        u8_out[i] = interp->bytecode[interp->pc + i];
    }
    interp->pc += size;
    return 0;
}
int bci__next_reg(Bc_Interpreter* interp, Bc_Reg* out) {
    if (interp->pc + sizeof(Bc_Reg) > interp->count) {
        interp->flags |= BC_INTERP_FLAGS_INCOMPLETE;
        return 1;
    }
    Bc_Reg reg = *(Bc_Reg*)(interp->bytecode + interp->pc);
    // TODO: cannot do bounds check here
    if (reg >= interp->max_register_count) {
        interp->flags |= BC_INTERP_FLAGS_INVALID;
        return 1;
    }
    *out = reg;
    interp->pc += sizeof(Bc_Reg);
    return 0;
}

#define bci__use_reg(interp, reg) \
    if (interp->frame_max_register_written < reg) \
        interp->frame_max_register_written = reg

int bci__read_register(Bc_Interpreter* interp, Bc_Reg reg, Bc_Value* value) {
    Bc_Reg base = interp->call_stack[interp->call_stack_count - 1].base_register;
    if (base + reg >= interp->max_register_count) return 1;
    *value = interp->registers[base + reg];
    return 0;
}
int bci__write_register(Bc_Interpreter* interp, Bc_Reg reg, Bc_Value value) {
    Bc_Reg base = interp->call_stack[interp->call_stack_count - 1].base_register;
    if (base + reg >= interp->max_register_count) return 1;
    bci__use_reg(interp, reg);
    interp->registers[base + reg] = value;
    return 0;
}

uintptr_t bci__call_native_procedure(uintptr_t address, uintptr_t* args, int arg_count) {
    uintptr_t result = 0;
#if defined(__arm64__)
    if (arg_count-- < 0) goto call; asm ( "mov x0, %0" :: "r" (args[0]) : "x0" );
    if (arg_count-- < 0) goto call; asm ( "mov x1, %0" :: "r" (args[1]) : "x1" );
    if (arg_count-- < 0) goto call; asm ( "mov x2, %0" :: "r" (args[2]) : "x2" );
    if (arg_count-- < 0) goto call; asm ( "mov x3, %0" :: "r" (args[3]) : "x3" );
    if (arg_count-- < 0) goto call; asm ( "mov x4, %0" :: "r" (args[4]) : "x4" );
    if (arg_count-- < 0) goto call; asm ( "mov x5, %0" :: "r" (args[5]) : "x5" );
    if (arg_count-- < 0) goto call; asm ( "mov x6, %0" :: "r" (args[6]) : "x6" );
    if (arg_count-- < 0) goto call; asm ( "mov x7, %0" :: "r" (args[7]) : "x7" );

    // TODO: push onto stack with >8 arguments
    if (arg_count > 0) printf("ERROR: Too many arguments!");

call:
    asm ( "blr %0"      :: "r" (address) : "x30" );
    asm ( "mov %0, x0"  : "=r" (result) );
#else
	sizeof(address, args, arg_count);
    bc__debug_log("what is this now?");
//#   error "Dynamic call not implemented for this architecture!"
#endif
    return result;
}

Bc_Value bci__compute_math(uint8_t type, uint8_t op, Bc_Value a, Bc_Value b) {
#   define BCI__MATH_OPERATION(NAME,A,B)                    \
    switch (op) {                                           \
        case BC_OP_ADD: return (Bc_Value) {.NAME = A + B};  \
        case BC_OP_SUB: return (Bc_Value) {.NAME = A - B};  \
        case BC_OP_MUL: return (Bc_Value) {.NAME = A * B};  \
        case BC_OP_DIV: return (Bc_Value) {.NAME = A / B};  \
    }
    switch (type) {
#       define X(NAME, VALUE, TYPE) case VALUE: BCI__MATH_OPERATION(NAME, a.NAME, b.NAME)
        BC_X_PRIMITIVE_TYPES
#       undef X
    }
    return (Bc_Value) {0};
#   undef BCI__MATH_OPERATION
}

uint8_t bci__compute_comparison(uint8_t type, uint8_t comp, Bc_Value a, Bc_Value b) {
#   define BCI__COMPARE(A,B)             \
    switch (comp) {                      \
        case BC_CMP_LT:  return A < B;   \
        case BC_CMP_GTE: return A >= B;  \
        case BC_CMP_GT:  return A > B;   \
        case BC_CMP_LTE: return A <= B;  \
        case BC_CMP_EQ:  return A == B;  \
        case BC_CMP_NE:  return A != B;  \
    }
    switch (type) {
#       define X(NAME, VALUE, TYPE) case VALUE: BCI__COMPARE(a.NAME, b.NAME)
        BC_X_PRIMITIVE_TYPES
#       undef X
    }
    return 0;
#   undef BCI__COMPARE
}

uint32_t bci_required_memory_size(Bc_Interpreter_Setup* info) {
    return info->max_register_count     * sizeof(Bc_Value)
        +  info->max_call_stack_count   * sizeof(Bc_Procedure_Frame)
        +  info->max_return_value_count * sizeof(Bc_Reg)
        +  256                          * sizeof(Bc_Reg);
}

int bci_create(Bc_Interpreter* interp, Bc_Interpreter_Setup* info)
{
    if (info->memory == 0) return 1;
    uint8_t* base = info->memory;
    interp->registers = (Bc_Value*)base;
    base += info->max_register_count * sizeof(Bc_Value);
    interp->call_stack = (Bc_Procedure_Frame*)base;
    base += info->max_call_stack_count * sizeof(Bc_Procedure_Frame);
    interp->return_registers = (Bc_Reg*)base;
    base += info->max_return_value_count * sizeof(Bc_Reg);
    interp->scratch_buffer = (uint32_t*)base;

    interp->bytecode = 0;
    interp->count = 0;
    interp->flags = 0;
    interp->pc = 0;
    interp->call_stack_count = 0;
    interp->return_registers_count = 0;
    interp->max_register_count = info->max_register_count;
    interp->max_call_stack_count = info->max_call_stack_count;
    interp->max_return_values_count = info->max_return_value_count;
    return 0;
}

void bci_load_from_stream(Bc_Interpreter* interp, Bc_Stream* stream)
{
    interp->bytecode = stream->data;
    interp->count    = stream->count;
}

void bci_load_from_memory(Bc_Interpreter* interp, void* code, uint32_t size)
{	
    interp->bytecode = code;
    interp->count    = size;
}

void bci_reset(Bc_Interpreter* interp, uint32_t location)
{
    interp->pc = location;
    interp->flags = 0;
    interp->return_registers_count = 0;
	interp->frame_max_register_written = 0;
	interp->stack_size = 0;

	// setup stack for execution
    interp->call_stack_count = 1;
	interp->call_stack[0].base_register = 0;
	interp->call_stack[0].return_address = 0; // final return will end exection, not used.
}

void bci_set_input(Bc_Interpreter* interp, uint32_t index, Bc_Value value)
{
	interp->registers[index] = value;
}

void bci_push_output(Bc_Interpreter* interp, Bc_Reg reg)
{
	interp->return_registers[interp->return_registers_count++] = reg;
}


#define BC_X_INSTRUCTIONS  \
    X(BC_OP_NOP          ) \
    X(BC_OP_LOAD_CONSTANT) \
    X(BC_OP_ADD          ) \
    X(BC_OP_SUB          ) \
    X(BC_OP_MUL          ) \
    X(BC_OP_DIV          ) \
    X(BC_OP_COMPARE      ) \
    X(BC_OP_BRANCH       ) \
    X(BC_OP_JUMP         ) \
    X(BC_OP_CALL         ) \
    X(BC_OP_RETURN       ) \
    X(BC_OP_NATIVE_CALL  ) \
    X(BC_OP_LOAD         ) \
    X(BC_OP_STORE        ) \
    X(BC_OP_STACK_ALLOC  ) \
    X(BC_OP_STACK_FREE   ) \
    X(BC_OP_CHECK_ADDRESS)

char const* bc__op_to_name[] = {   
#define X(NAME) [NAME] = #NAME,
    BC_X_INSTRUCTIONS
#undef X
};

// return 0 => done
int bci_step(Bc_Interpreter* interp)
{
    if (interp->pc >= interp->count) {
        interp->flags |= BC_INTERP_FLAGS_OVERFLOW;
        return 0;
    }

    uint8_t operation = interp->bytecode[interp->pc++];

    if (operation < 100) {
        bc__debug_log("ex %s", bc__op_to_name[operation]);
    }

    switch (operation)
    {
    case BC_OP_LOAD_CONSTANT: {
        Bc_Reg dst;
        uint8_t type;
        Bc_Value value = {0};

        if (bci__next_reg(interp, &dst)) return 0;
        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_value(interp, type, &value)) return 0;
        if (bci__write_register(interp, dst, value)) return 0;
    } break;

    case BC_OP_ADD:
    case BC_OP_SUB:
    case BC_OP_MUL:
    case BC_OP_DIV:
    {
        Bc_Reg dst;
        uint8_t type;

        if (bci__next_reg(interp, &dst)) return 0;
        if (bci__next_u8(interp, &type)) return 0;

        uint8_t is_value = type & 0x80;
        type &= 0x7F;

        Bc_Value va, vb;
        {
            Bc_Reg a;
            if (bci__next_reg(interp, &a)) return 0;
            if (bci__read_register(interp, a, &va)) return 0;
        }

        if (is_value) {
            if (bci__next_value(interp, type, &vb)) return 0;
        } else {
            Bc_Reg b;
            if (bci__next_reg(interp, &b)) return 0;
            if (bci__read_register(interp, b, &vb)) return 0;
        }

        Bc_Value result = bci__compute_math(type, operation, va, vb);
        if (bci__write_register(interp, dst, result)) return 0;
    } break;

    case BC_OP_COMPARE: {
        Bc_Reg dst;
        uint8_t type;
        uint8_t comp;
		
        if (bci__next_reg(interp, &dst)) return 0;
        if (bci__next_u8(interp, &type)) return 0;
        if (bci__next_u8(interp, &comp)) return 0;
		
        uint8_t is_value = type & 0x80;
        type &= 0x7F;
		
        Bc_Value va, vb;
        {
			uint32_t a;
            if (bci__next_reg(interp, &a)) return 0;
            if (bci__read_register(interp, a, &va)) return 0;
        }
		
        if (is_value) {
			if (bci__next_value(interp, type, &vb)) return 0;
        } else {
			uint32_t b;
            if (bci__next_reg(interp, &b)) return 0;
            if (bci__read_register(interp, b, &vb)) return 0;
        }

        
        Bc_Value result = {.U8 = bci__compute_comparison(type, comp, va, vb)};
        if (bci__write_register(interp, dst, result)) return 0;
    } break;

    case BC_OP_BRANCH: {
        uint32_t location;
        Bc_Reg src;

        if (bci__next_u32(interp, &location)) return 0;
        if (bci__next_reg(interp, &src)) return 0;

        Bc_Value value;
        if (bci__read_register(interp, src, &value)) return 0;
        if (value.U8 != 0) interp->pc = location;
    } break;

    case BC_OP_JUMP: {
        if (bci__next_u32(interp, &interp->pc)) return 0;
    } break;

    case BC_OP_CALL: {
        uint32_t location;
        uint8_t ret_count, arg_count;

        if (bci__next_u32(interp, &location)) return 0;
        if (bci__next_u8(interp, &ret_count)) return 0;
        if (bci__next_u8(interp, &arg_count)) return 0;

        Bc_Reg base = interp->call_stack[interp->call_stack_count - 1].base_register;
        
        bci__for (ret_count) {
            Bc_Reg reg;
            bci__next_reg(interp, &reg);
            // TODO: bounds check
            interp->return_registers[interp->return_registers_count + (ret_count - 1) - i] = reg;
            bci__use_reg(interp, reg);
        }
        interp->return_registers_count += ret_count;

        Bc_Reg proc_base = base + interp->frame_max_register_written + 1;

        bci__for (arg_count) {
            Bc_Reg reg;
            bci__next_reg(interp, &reg);
            interp->registers[proc_base + i] = interp->registers[base + reg];
        }

        interp->call_stack[interp->call_stack_count++] = (Bc_Procedure_Frame) {
            .return_address = interp->pc,
            .base_register  = proc_base,
        };
        interp->frame_max_register_written = 0;
        interp->pc = location;
    } break;

    case BC_OP_RETURN: {
        uint8_t ret_count;
        if (bci__next_u8(interp, &ret_count)) return 0;

        Bc_Reg base = interp->call_stack[interp->call_stack_count - 1].base_register;
        Bc_Reg ret_base = (interp->call_stack_count <= 1) ? 0
                        : interp->call_stack[interp->call_stack_count - 2].base_register;

        bci__for (ret_count) {
            Bc_Reg src;
            if (bci__next_reg(interp, &src)) return 0;
            // TODO: bounds check
            Bc_Reg dst = interp->return_registers[interp->return_registers_count - 1];
            interp->registers[ret_base + dst] = interp->registers[base + src];
            interp->return_registers_count -= 1;
        }

        interp->call_stack_count -= 1;

        if (interp->call_stack_count == 0) {
            interp->flags |= BC_INTERP_FLAGS_DONE;
            return 0;
        }

        interp->pc = interp->call_stack[interp->call_stack_count].return_address;
    } break;

    case BC_OP_NATIVE_CALL: {
        uintptr_t address;
        uint8_t use_dst;
        Bc_Reg dst = 0;
        uint8_t input_count;

        if (bci__next_address(interp, &address)) return 0;
        if (bci__next_u8(interp, &use_dst)) return 0;
        if (use_dst && bci__next_reg(interp, &dst)) return 0;
        if (bci__next_u8(interp, &input_count)) return 0;

        Bc_Value* args = interp->scratch_buffer;
        bci__for (input_count) {
            Bc_Reg reg;
            if (bci__next_reg(interp, &reg)) return 0;
            if (bci__read_register(interp, reg, args + i)) return 0;
        }

        union {
            uintptr_t in;
            Bc_Value value;
        } result = { .in = bci__call_native_procedure(address, (uintptr_t*)args, input_count) };

        if (use_dst) {
            if (bci__write_register(interp, dst, result.value)) return 0;
        }
    } break;

    case BC_OP_LOAD: {
        // reg dst
        // u8  type
        // reg addr
        // u32 offset
        bc__debug_log("BC_OP_LOAD not implemented!");
    } break;

    case BC_OP_STORE: {
        // reg src
        // u8  type
        // reg addr
        // u32 offset
        bc__debug_log("BC_OP_STORE not implemented!");
    } break;

    case BC_OP_STACK_ALLOC: {
        // reg dst
        // u32 size
        bc__debug_log("BC_OP_STACK_ALLOC not implemented!");
    } break;

    case BC_OP_STACK_FREE: {
        // u32 size
        bc__debug_log("BC_OP_STACK_FREE not implemented!");
    } break;
    
    default: {
        bc__debug_log("invalid bytecode instruction! (pc=%x op=%d)", interp->pc - 1, operation);
        interp->flags |= BC_INTERP_FLAGS_INCOMPLETE;
        return 0;
    } break;
    }

    return 1;
}

