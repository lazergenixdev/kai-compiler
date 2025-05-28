#include "kai.h"

// TODO: remove this
#include <stdio.h>

#define bcs__for(N) for (int i = 0; i < (int)N; ++i)
#define __type_to_size(T) ((T) >> 4)

extern char const* bc__op_to_name [100];

uint32_t bcs__grow_function(uint32_t x)
{
    return x + x / 2;
}

// TODO: no reason why this should be specific to Bytecode Arrays
Kai_Result bcs__ensure_space(Kai_BC_Stream* stream, uint32_t added_count)
{
    uint32_t required_capacity = stream->count + added_count;
    
    if (stream->capacity >= required_capacity)
        return KAI_SUCCESS;
    
    uint32_t new_capacity = bcs__grow_function(required_capacity);

    Kai_Allocator* allocator = stream->allocator;
    void* new_data = kai__allocate(stream->data, new_capacity, stream->capacity);

    if (new_data == NULL)
        return KAI_BC_ERROR_MEMORY;

    stream->data = new_data;
    stream->capacity = new_capacity;
    return KAI_SUCCESS;
}

#define bcs__make_space(BYTES_REQUIRED)            \
    if (bcs__ensure_space(stream, BYTES_REQUIRED)) \
        return KAI_BC_ERROR_MEMORY

#define bcs__push_u8(stream, value) stream->data[stream->count++] = value;
#define bcs__push_u32(stream, value) *(Kai_u32*)(stream->data + stream->count) = value, stream->count += 4
#define bcs__push_address(stream, value) *(uintptr_t*)(stream->data + stream->count) = (uintptr_t)value, stream->count += sizeof(uintptr_t)

static void bcs__push_value(Kai_BC_Stream* stream, Kai_u8 type, Kai_Value value)
{
    uint32_t size = __type_to_size(type);
    kai__memory_copy(stream->data + stream->count, &value, size);
    stream->count += size;
}

// BYTECODE INSTRUCTION DESCRIPTION SYNTAX:
//
// Basic: [WHAT_IS_STORED]:bits_required
//
// Concatenation: [option1:size1 | option2:size2]:size   [ size = max(size1, size2) ]
// Addition:      [part1  :4     + part2  :4    ]:size   [ size = 4 + 4 ]
//
// S = size of platform (ex: 64-bit -> 64)
// R = size to hold register index (hard-coded to 32 bits)
// $ = size inferred from TYPE
// - = use rest of bits available


// ---------------------------------------------------------------------
//  LOAD_CONSTANT
// >> [DST]:R [TYPE]:8 [VALUE]:$
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_load_constant(Kai_BC_Stream* stream, Kai_u8 type, Kai_Reg reg_dst, Kai_Value value)
{
    bcs__make_space(2 + sizeof(Kai_Reg) + __type_to_size(type));
    bcs__push_u8(stream, KAI_BOP_LOAD_CONSTANT);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_value(stream, type, value);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  ADD, SUB, MUL, DIV, ... with register
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [LEFT]:R [RIGHT]:R
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_math(Kai_BC_Stream* stream, Kai_u8 type, Kai_u8 operation, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Reg reg_src2)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * 3);
    bcs__push_u8(stream, operation);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_src1);
    bcs__push_u32(stream, reg_src2);
    printf("math %%%i <- %%%i, %%%i\n", reg_dst, reg_src1, reg_src2);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  ADD, SUB, MUL, DIV, ... with value 
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [LEFT]:R [VALUE]:$
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_math_value(Kai_BC_Stream* stream, Kai_u8 type, Kai_u8 operation, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Value value)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * 2 + __type_to_size(type));
    bcs__push_u8(stream, operation);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, 0x80 | type);
    bcs__push_u32(stream, reg_src1);
    bcs__push_value(stream, type, value);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  COMPARE
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [COMPARISON]:8 [LEFT]:R [RIGHT]:R
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_compare(Kai_BC_Stream* stream, Kai_u8 type, Kai_u8 comparison, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Reg reg_src2)
{
    bcs__make_space(3 + sizeof(Kai_Reg) * 3);
    bcs__push_u8(stream, KAI_BOP_COMPARE);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u8(stream, comparison);
    bcs__push_u32(stream, reg_src1);
    bcs__push_u32(stream, reg_src2);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  COMPARE
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [COMPARISON]:8 [LEFT]:R [VALUE]:$
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_compare_value(Kai_BC_Stream* stream, Kai_u8 type, Kai_u8 comparison, Kai_Reg reg_dst, Kai_Reg reg_src1, Kai_Value value)
{
    bcs__make_space(3 + sizeof(Kai_Reg) * 2 + __type_to_size(type));
    bcs__push_u8(stream, KAI_BOP_COMPARE);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, 0x80 | type);
    bcs__push_u8(stream, comparison);
    bcs__push_u32(stream, reg_src1);
    bcs__push_value(stream, type, value);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  BRANCH
// >> [LOCATION]:32 [SRC]:R
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_branch_location(Kai_BC_Stream* stream, uint32_t location, Kai_Reg reg_src)
{
    bcs__make_space(1 + sizeof(Kai_Reg) * 2);
    bcs__push_u8(stream, KAI_BOP_BRANCH);
    bcs__push_u32(stream, location);
    bcs__push_u32(stream, reg_src);
    return KAI_SUCCESS;
}
Kai_Result kai_bc_insert_branch(Kai_BC_Stream* stream, uint32_t* branch, Kai_Reg reg_src)
{
    bcs__make_space(1 + sizeof(Kai_Reg) * 2);
    bcs__push_u8(stream, KAI_BOP_BRANCH);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    bcs__push_u32(stream, reg_src);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  JUMP
// >> [LOCATION]:32
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_jump_location(Kai_BC_Stream* stream, uint32_t location)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_JUMP);
    bcs__push_u32(stream, location);
    return KAI_SUCCESS;
}
Kai_Result kai_bc_insert_jump(Kai_BC_Stream* stream, uint32_t* branch)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_JUMP);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  CALL
// >> [LOCATION]:32 [RET_COUNT]:8 [ARG_COUNT]:8 ([DST]:R ...) ([SRC]:R ...)
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_call(Kai_BC_Stream* stream, uint32_t* branch, Kai_u8 ret_count, Kai_Reg* reg_ret, Kai_u8 arg_count, Kai_Reg* reg_arg)
{
    bcs__make_space(3 + sizeof(uint32_t) + sizeof(Kai_Reg) * (ret_count + arg_count));
    bcs__push_u8(stream, KAI_BOP_CALL);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    bcs__push_u8(stream, ret_count);
    bcs__push_u8(stream, arg_count);
    bcs__for (ret_count) bcs__push_u32(stream, reg_ret[i]);
    bcs__for (arg_count) bcs__push_u32(stream, reg_arg[i]);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  RETURN
// >> [COUNT]:8 ([SRC]:R ...)
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_return(Kai_BC_Stream* stream, Kai_u8 count, Kai_Reg* regs)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * count);
    bcs__push_u8(stream, KAI_BOP_RETURN);
    bcs__push_u8(stream, count);
    bcs__for (count) bcs__push_u32(stream, regs[i]);
    printf("ret %%%i\n", regs[0]);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  NATIVE_CALL
// >> [ADDRESS]:S [USE_DST]:8 ([DST]:R) [COUNT]:8 ([SRC]:R ...)
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_native_call(Kai_BC_Stream* stream, Kai_u8 use_dst, Kai_Reg reg_dst, Kai_Native_Procedure* proc, Kai_Reg* reg_src)
{
    bcs__make_space(3 + sizeof(uintptr_t) + (use_dst? sizeof(Kai_Reg):0) + sizeof(Kai_Reg) * proc->input_count);
    bcs__push_u8(stream, KAI_BOP_NATIVE_CALL);
    bcs__push_address(stream, proc->address);
    bcs__push_u8(stream, use_dst? 1:0);
    if (use_dst) bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, proc->input_count);
    bcs__for (proc->input_count) bcs__push_u32(stream, reg_src[i]);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  LOAD, STORE
// >> [DST_OR_SRC]:R [TYPE]:8 [ADDR]:R [OFFSET]:32
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_load(Kai_BC_Stream* stream, Kai_Reg reg_dst, Kai_u8 type, Kai_Reg reg_addr, uint32_t offset)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * 2 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_LOAD);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_addr);
    bcs__push_u32(stream, offset);
    return KAI_SUCCESS;
}
Kai_Result kai_bc_insert_store(Kai_BC_Stream* stream, Kai_Reg reg_src, Kai_u8 type, Kai_Reg reg_addr, uint32_t offset)
{
    bcs__make_space(2 + sizeof(Kai_Reg) * 2 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_STORE);
    bcs__push_u32(stream, reg_src);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_addr);
    bcs__push_u32(stream, offset);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  STACK_ALLOC (type of R = u32)
// >> [DST]:R [SIZE]:32
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_stack_alloc(Kai_BC_Stream* stream, Kai_Reg reg_dst, uint32_t size)
{
    bcs__make_space(1 + sizeof(Kai_Reg) + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_STACK_ALLOC);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u32(stream, size);
    return KAI_SUCCESS;
}

// ---------------------------------------------------------------------
//  STACK_FREE
// >> [SIZE]:32
// ---------------------------------------------------------------------
Kai_Result kai_bc_insert_stack_free(Kai_BC_Stream* stream, uint32_t size)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, KAI_BOP_STACK_FREE);
    bcs__push_u32(stream, size);
    return KAI_SUCCESS;
}

Kai_Result kai_bc_set_branch(Kai_BC_Stream* stream, uint32_t branch, uint32_t location)
{
    if (branch >= stream->count) return KAI_BC_ERROR_BRANCH;
    *(uint32_t*)(stream->data + branch) = location;
    return KAI_SUCCESS;
}

#ifndef __WASM__
#include <stdio.h>

const char* __math_op_to_string(Kai_u8 op)
{
    switch (op)
    {
        case KAI_BOP_ADD: return "+";
        case KAI_BOP_MUL: return "*";
        case KAI_BOP_DIV: return "/";
        case KAI_BOP_SUB: return "-";
        default: return "?";
    }
}

Kai_Result kai_bytecode_to_c(Kai_Bytecode* bytecode, Kai_Writer* writer)
{
    //! TODO: use type info
    //! TODO: find all branch locations
    //! TODO: use procedure name and argument count

    char temp_buffer [1024];

    #define bcc__write(...) \
    { \
        int length = snprintf(temp_buffer, sizeof(temp_buffer), __VA_ARGS__); \
        writer->write(writer->user, (Kai_str) {.data = (Kai_u8*)temp_buffer, .count = length}); \
    } (void)0

    #define bcc__indent() bcc__write(";   ")
    #define bcc__branch_check() \
    bcs__for (bytecode->branch_count) { \
        if (cursor == bytecode->branch_hints[i]) { \
            bcc__write("__loc_%i:\n", i); \
            break; \
        } \
    }    

    Kai_u8 const* data = bytecode->data;
    uint32_t cursor = 0;

    bcc__write("int32_t func(");
    for (int i = 0;;)
    {
        bcc__write("int32_t __%i", i);
        if (++i < bytecode->arg_count)
        {
            bcc__write(", ");
        }
        else break;
    }
    bcc__write(") {\n");

    while (cursor < bytecode->count) {
        bcc__branch_check();

        Kai_u8 operation = data[cursor++];

        //if (operation < 100) {
        //    bc__debug_log("decode %s", bc__op_to_name[operation]);
        //}

        switch (operation)
        {
        case KAI_BOP_LOAD_CONSTANT: {
            Kai_Reg dst;
            dst = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            Kai_u8 type;
            type = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_Value value = {0};
            value = *(Kai_Value*)(data + cursor);
            cursor += __type_to_size(type);

            bcc__indent();
            bcc__write("%s __%d = %d;\n", "int32_t", dst, value.s32);
        } break;

        case KAI_BOP_ADD:
        case KAI_BOP_SUB:
        case KAI_BOP_MUL:
        case KAI_BOP_DIV:
        {
            Kai_Reg dst;
            dst = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            Kai_u8 type;
            type = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_u8 is_value = type & 0x80;
            type &= 0x7F;

            Kai_Reg a;
            a = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            bcc__indent();
            if (is_value) {
                Kai_Value value;
                Kai_u8 size = __type_to_size(type);
                Kai_u8* u8_out = (Kai_u8*)&value;
                for (int i = 0; i < size; ++i)
                    u8_out[i] = data[cursor + i];
                cursor += size;
                bcc__write("%s __%d = __%d %s %i;\n", "int32_t", dst, a, __math_op_to_string(operation), value.s32);
            } else {
                Kai_Reg b = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
                bcc__write("%s __%d = __%d %s __%d;\n", "int32_t", dst, a, __math_op_to_string(operation), b);
            }

        } break;

        case KAI_BOP_NATIVE_CALL: {
            uintptr_t address;
            address = *(uintptr_t*)(data + cursor);
            cursor += sizeof(uintptr_t);

            Kai_u8 use_dst = 0;
            use_dst = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_Reg dst = 0;
            if (use_dst) {
                dst = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
            }

            Kai_u8 input_count;
            input_count = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            char const* name = "???";
            bcs__for (bytecode->native_count) {
                if (address == (uintptr_t)bytecode->natives[i].address) {
                    name = bytecode->natives[i].name;
                    break;
                }
            }

            bcc__indent();
            if (use_dst) {
                bcc__write("%s __%d = %s(", "int32_t", dst, name);
            } else {
                bcc__write("%s(", name);
            }

            bcs__for (input_count) {
                Kai_Reg reg;
                reg = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);

                bcc__write("__%d", reg);

                if (i != input_count - 1) {
                    bcc__write(", ");
                }
            }

            bcc__write(");\n");
        } break;

        case KAI_BOP_COMPARE: {
            Kai_Reg dst;
            dst = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            Kai_u8 type;
            type = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_u8 is_value = type & 0x80;
            type &= 0x7F;

            Kai_u8 comp;
            comp = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_Reg a;
            a = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            char const* tab[] = { "<", ">=", ">", "<=", "==", "!=" };

            bcc__indent();
            bcc__write("%s __%d = __%d %s ", "int32_t", dst, a, tab[comp]);

            if (is_value) {
                Kai_Value value;
                Kai_u8 size = __type_to_size(type);
                Kai_u8* u8_out = (Kai_u8*)&value;
                for (int i = 0; i < size; ++i)
                    u8_out[i] = data[cursor + i];
                cursor += size;
                bcc__write("%i;\n", value.s32);
            } else {
                Kai_Reg b = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
                bcc__write("__%d;\n", b);
            }
        } break;

        case KAI_BOP_BRANCH: {
            uint32_t location = *(uint32_t*)(data + cursor);
            cursor += sizeof(uint32_t);

            Kai_Reg src = *(Kai_Reg*)(data + cursor);
            cursor += sizeof(Kai_Reg);

            bcc__indent();
            bcs__for (bytecode->branch_count) {
                if (location == bytecode->branch_hints[i]) {
                    bcc__write("if (__%d) goto __loc_%i;\n", src, i);
                    break;
                }
            }
        } break;

        case KAI_BOP_JUMP: {
            cursor += sizeof(uint32_t);
            bcc__indent();
            bcc__write("goto __endif;\n");
            bcc__write("__else:\n");
        } break;

        case KAI_BOP_CALL: {
            //uint32_t location = *(uint32_t*)(data + cursor);
            cursor += sizeof(uint32_t);

            Kai_u8 ret_count = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_u8 arg_count = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);
            
            bcc__indent();
            bcs__for (ret_count) {
                Kai_Reg dst = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
                bcc__write("%s __%d = ", "int32_t", dst);
            }
            bcc__write("%s(", "func");
            bcs__for (arg_count) {
                Kai_Reg src = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
                bcc__write("__%d%s", src, i == arg_count - 1 ? "" : ", ");
            }
            bcc__write(");\n");
        } break;

        case KAI_BOP_RETURN: {
            Kai_u8 ret_count;
            ret_count = *(Kai_u8*)(data + cursor);
            cursor += sizeof(Kai_u8);

            Kai_Reg a;
            bcc__indent();
            bcs__for (ret_count) {
                a = *(Kai_Reg*)(data + cursor);
                cursor += sizeof(Kai_Reg);
                bcc__write("return __%d;\n", a);
            }
        } break;
        
        default: {
            kai__unreachable();
            //bc__debug_log("invalid bytecode instruction! (pc=%x op=%d)", cursor - 1, operation);
            return KAI_SUCCESS;
        } break;
        }
    }

    bcc__write("}");
	return KAI_SUCCESS;
}
#endif
