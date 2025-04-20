#include "bytecode.h"
#include "stdlib.h" // --> realloc
#include "stdio.h" // --> snprintf

#define bcs__for(N) for (int i = 0; i < (int)N; ++i)

extern uint8_t bc__type_to_size [16];
extern char const* bc__op_to_name [100];

uint32_t bcs__grow_function(uint32_t x) {
    return x + x / 2;
}

int bcs__ensure_space(Bc_Stream* stream, uint32_t added_count) {
    uint32_t const required_capacity = stream->count + added_count;
    if (stream->capacity >= required_capacity)
        return BC_STREAM_SUCCESS;
    uint32_t const new_capacity = bcs__grow_function(required_capacity);
    void* new_data = realloc(stream->data, new_capacity);
    if (new_data == NULL)
        return BC_STREAM_ERROR_MEMORY;
    stream->data = new_data;
    stream->capacity = new_capacity;
    return BC_STREAM_SUCCESS;
}

#define bcs__make_space(BYTES_REQUIRED)           \
    if (bcs__ensure_space(stream, BYTES_REQUIRED)) \
        return BC_STREAM_ERROR_MEMORY

// NOTE: NO CHECKS
#define bcs__push_u8(stream, value) stream->data[stream->count++] = value;
#define bcs__push_u32(stream, value) *(uint32_t*)(stream->data + stream->count) = value, stream->count += 4
#define bcs__push_address(stream, value) *(uintptr_t*)(stream->data + stream->count) = (uintptr_t)value, stream->count += sizeof(uintptr_t)
static void bcs__push_value(Bc_Stream* stream, Bc_Type type, Bc_Value value) {
    uint32_t size = bc__type_to_size[type];
    for (uint32_t i = 0; i < size; ++i) {
        stream->data[stream->count + i] = ((uint8_t*)&value)[i];
    }
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
int bcs_insert_load_constant(Bc_Stream* stream, uint8_t type, uint32_t reg_dst, Bc_Value value)
{
    bcs__make_space(2 + sizeof(Bc_Reg) + bc__type_to_size[type]);
    bcs__push_u8(stream, BC_OP_LOAD_CONSTANT);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_value(stream, type, value);
    return BC_STREAM_ERROR_MEMORY;
}

// ---------------------------------------------------------------------
//  ADD, SUB, MUL, DIV, ... with register
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [LEFT]:R [RIGHT]:R
// ---------------------------------------------------------------------
int bcs_insert_math(Bc_Stream* stream, uint8_t type, uint8_t operation, uint32_t reg_dst, uint32_t reg_src1, uint32_t reg_src2)
{
    bcs__make_space(2 + sizeof(Bc_Reg) * 3);
    bcs__push_u8(stream, operation);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_src1);
    bcs__push_u32(stream, reg_src2);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  ADD, SUB, MUL, DIV, ... with value 
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [LEFT]:R [VALUE]:$
// ---------------------------------------------------------------------
int bcs_insert_math_value(Bc_Stream* stream, uint8_t type, uint8_t operation, uint32_t reg_dst, uint32_t reg_src1, Bc_Value value)
{
    bcs__make_space(2 + sizeof(Bc_Reg) * 2 + bc__type_to_size[type]);
    bcs__push_u8(stream, operation);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, 0x80 | type);
    bcs__push_u32(stream, reg_src1);
    bcs__push_value(stream, type, value);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  COMPARE
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [COMPARISON]:8 [LEFT]:R [RIGHT]:R
// ---------------------------------------------------------------------
int bcs_insert_compare(Bc_Stream* stream, uint8_t type, uint8_t comparison, uint32_t reg_dst, uint32_t reg_src1, uint32_t reg_src2)
{
    bcs__make_space(3 + sizeof(Bc_Reg) * 3);
    bcs__push_u8(stream, BC_OP_COMPARE);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u8(stream, comparison);
    bcs__push_u32(stream, reg_src1);
    bcs__push_u32(stream, reg_src2);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  COMPARE
// >> [DST]:R [VALUE_FLAG:1 + TYPE:-]:8 [COMPARISON]:8 [LEFT]:R [VALUE]:$
// ---------------------------------------------------------------------
int bcs_insert_compare_value(Bc_Stream* stream, uint8_t type, uint8_t comparison, uint32_t reg_dst, uint32_t reg_src1, Bc_Value value)
{
    bcs__make_space(3 + sizeof(Bc_Reg) * 2 + bc__type_to_size[type]);
    bcs__push_u8(stream, BC_OP_COMPARE);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, 0x80 | type);
    bcs__push_u8(stream, comparison);
    bcs__push_u32(stream, reg_src1);
    bcs__push_value(stream, type, value);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  BRANCH
// >> [LOCATION]:32 [SRC]:R
// ---------------------------------------------------------------------
int bcs_insert_branch_location(Bc_Stream* stream, uint32_t location, uint32_t reg_src)
{
    bcs__make_space(1 + sizeof(Bc_Reg) * 2);
    bcs__push_u8(stream, BC_OP_BRANCH);
    bcs__push_u32(stream, location);
    bcs__push_u32(stream, reg_src);
    return BC_STREAM_SUCCESS;
}
int bcs_insert_branch(Bc_Stream* stream, uint32_t* branch, uint32_t reg_src)
{
    bcs__make_space(1 + sizeof(Bc_Reg) * 2);
    bcs__push_u8(stream, BC_OP_BRANCH);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    bcs__push_u32(stream, reg_src);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  JUMP
// >> [LOCATION]:32
// ---------------------------------------------------------------------
int bcs_insert_jump_location(Bc_Stream* stream, uint32_t location)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, BC_OP_JUMP);
    bcs__push_u32(stream, location);
    return BC_STREAM_SUCCESS;
}
int bcs_insert_jump(Bc_Stream* stream, uint32_t* branch)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, BC_OP_JUMP);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  CALL
// >> [LOCATION]:32 [RET_COUNT]:8 [ARG_COUNT]:8 ([DST]:R ...) ([SRC]:R ...)
// ---------------------------------------------------------------------
int bcs_insert_call(Bc_Stream* stream, uint32_t* branch, uint8_t ret_count, uint32_t* reg_ret, uint32_t arg_count, uint32_t* reg_arg)
{
    bcs__make_space(3 + sizeof(uint32_t) + sizeof(Bc_Reg) * (ret_count + arg_count));
    bcs__push_u8(stream, BC_OP_CALL);
    *branch = stream->count;
    bcs__push_u32(stream, 0xFFFFFFFF); // location not set yet
    bcs__push_u8(stream, ret_count);
    bcs__push_u8(stream, arg_count);
    bcs__for (ret_count) bcs__push_u32(stream, reg_ret[i]);
    bcs__for (arg_count) bcs__push_u32(stream, reg_arg[i]);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  RETURN
// >> [COUNT]:8 ([SRC]:R ...)
// ---------------------------------------------------------------------
int bcs_insert_return(Bc_Stream* stream, uint8_t count, uint32_t* regs)
{
    bcs__make_space(2 + sizeof(Bc_Reg) * count);
    bcs__push_u8(stream, BC_OP_RETURN);
    bcs__push_u8(stream, count);
    bcs__for (count) bcs__push_u32(stream, regs[i]);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  NATIVE_CALL
// >> [ADDRESS]:S [USE_DST]:8 ([DST]:R) [COUNT]:8 ([SRC]:R ...)
// ---------------------------------------------------------------------
int bcs_insert_native_call(Bc_Stream* stream, uint8_t use_dst, uint32_t reg_dst, Bc_Native_Procedure* proc, uint32_t* reg_src)
{
    bcs__make_space(3 + sizeof(uintptr_t) + (use_dst? sizeof(Bc_Reg):0) + sizeof(Bc_Reg) * proc->input_count);
    bcs__push_u8(stream, BC_OP_NATIVE_CALL);
    bcs__push_address(stream, proc->address);
    bcs__push_u8(stream, use_dst? 1:0);
    if (use_dst) bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, proc->input_count);
    bcs__for (proc->input_count) bcs__push_u32(stream, reg_src[i]);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  LOAD, STORE
// >> [DST_OR_SRC]:R [TYPE]:8 [ADDR]:R [OFFSET]:32
// ---------------------------------------------------------------------
int bcs_insert_load(Bc_Stream* stream, uint32_t reg_dst, uint8_t type, uint32_t reg_addr, uint32_t offset)
{
    bcs__make_space(2 + sizeof(Bc_Reg) * 2 + sizeof(uint32_t));
    bcs__push_u8(stream, BC_OP_LOAD);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_addr);
    bcs__push_u32(stream, offset);
    return BC_STREAM_SUCCESS;
}
int bcs_insert_store(Bc_Stream* stream, uint32_t reg_src, uint8_t type, uint32_t reg_addr, uint32_t offset)
{
    bcs__make_space(2 + sizeof(Bc_Reg) * 2 + sizeof(uint32_t));
    bcs__push_u8(stream, BC_OP_STORE);
    bcs__push_u32(stream, reg_src);
    bcs__push_u8(stream, type);
    bcs__push_u32(stream, reg_addr);
    bcs__push_u32(stream, offset);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  STACK_ALLOC (type of R = u32)
// >> [DST]:R [SIZE]:32
// ---------------------------------------------------------------------
int bcs_insert_stack_alloc(Bc_Stream* stream, uint32_t reg_dst, uint32_t size)
{
    bcs__make_space(1 + sizeof(Bc_Reg) + sizeof(uint32_t));
    bcs__push_u8(stream, BC_OP_STACK_ALLOC);
    bcs__push_u32(stream, reg_dst);
    bcs__push_u32(stream, size);
    return BC_STREAM_SUCCESS;
}

// ---------------------------------------------------------------------
//  STACK_FREE
// >> [SIZE]:32
// ---------------------------------------------------------------------
int bcs_insert_stack_free(Bc_Stream* stream, uint32_t size)
{
    bcs__make_space(1 + sizeof(uint32_t));
    bcs__push_u8(stream, BC_OP_STACK_FREE);
    bcs__push_u32(stream, size);
    return BC_STREAM_SUCCESS;
}

int bcs_set_branch(Bc_Stream* stream, uint32_t branch, uint32_t location)
{
    if (branch >= stream->count) return BC_STREAM_ERROR_BRANCH;
    *(uint32_t*)(stream->data + branch) = location;
    return BC_STREAM_SUCCESS;
}

void bc_convert_to_c(Bc_Def* def, Bc_Writer_Proc* writer, void* user)
{
    //! TODO: use type info
    //! TODO: find all branch locations
    //! TODO: use procedure name and argument count

    char temp_buffer [1024];

    #define bcc__write(...) \
    { \
        int length = snprintf(temp_buffer, sizeof(temp_buffer), __VA_ARGS__); \
        writer(user, temp_buffer, length); \
    } (void)0

    #define bcc__indent() bcc__write(";   ")
    #define bcc__branch_check() \
    bcs__for (def->branch_count) { \
        if (cursor == def->branch_hints[i]) { \
            bcc__write("__loc_%i:\n", i); \
            break; \
        } \
    }    

    uint8_t const* bytecode = def->bytecode;
    uint32_t cursor = 0;

    bcc__write("int32_t func(int32_t __0) {\n");

    while (cursor < def->count) {
        bcc__branch_check();

        uint8_t operation = bytecode[cursor++];

        //if (operation < 100) {
        //    bc__debug_log("decode %s", bc__op_to_name[operation]);
        //}

        switch (operation)
        {
        case BC_OP_LOAD_CONSTANT: {
            Bc_Reg dst;
            dst = *(Bc_Reg*)(bytecode + cursor);
            cursor += sizeof(Bc_Reg);

            Bc_Type type;
            type = *(Bc_Type*)(bytecode + cursor);
            cursor += sizeof(Bc_Type);

            Bc_Value value = {0};
            value = *(Bc_Value*)(bytecode + cursor);
            cursor += bc__type_to_size[type];

            bcc__indent();
            bcc__write("%s __%d = %d;\n", "int32_t", dst, value.S32);
        } break;

        case BC_OP_ADD:
        case BC_OP_SUB:
        {
            Bc_Reg dst;
            dst = *(Bc_Reg*)(bytecode + cursor);
            cursor += sizeof(Bc_Reg);

            Bc_Type type;
            type = *(Bc_Type*)(bytecode + cursor);
            cursor += sizeof(Bc_Type);

            uint8_t is_value = type & 0x80;
            type &= 0x7F;

            Bc_Reg a;
            a = *(Bc_Reg*)(bytecode + cursor);
            cursor += sizeof(Bc_Reg);

            bcc__indent();
            if (is_value) {
                Bc_Value value;
                uint8_t size = bc__type_to_size[type];
                uint8_t* u8_out = (uint8_t*)&value;
                for (int i = 0; i < size; ++i)
                    u8_out[i] = bytecode[cursor + i];
                cursor += size;
                bcc__write("%s __%d = __%d %s %i;\n", "int32_t", dst, a, operation==BC_OP_ADD? "+":"-", value.S32);
            } else {
                uint32_t b = *(Bc_Reg*)(bytecode + cursor);
                cursor += sizeof(Bc_Reg);
                bcc__write("%s __%d = __%d %s __%d;\n", "int32_t", dst, a, operation==BC_OP_ADD? "+":"-", b);
            }

        } break;

        case BC_OP_NATIVE_CALL: {
            uintptr_t address;
            address = *(uintptr_t*)(bytecode + cursor);
            cursor += sizeof(uintptr_t);

            uint8_t use_dst;
            use_dst = *(uint8_t*)(bytecode + cursor);
            cursor += sizeof(uint8_t);

            Bc_Reg dst;
            if (use_dst) {
                dst = *(Bc_Reg*)(bytecode + cursor);
                cursor += sizeof(Bc_Reg);
            }

            uint8_t input_count;
            input_count = *(uint8_t*)(bytecode + cursor);
            cursor += sizeof(uint8_t);

            char const* name = "???";
            bcs__for (def->native_count) {
                if (address == (uintptr_t)def->natives[i].address) {
                    name = def->natives[i].name;
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
                Bc_Reg reg;
                reg = *(Bc_Reg*)(bytecode + cursor);
                cursor += sizeof(Bc_Reg);

                bcc__write("__%d", reg);

                if (i != input_count - 1) {
                    bcc__write(", ");
                }
            }

            bcc__write(");\n");
        } break;

        case BC_OP_COMPARE: {
            Bc_Reg dst;
            dst = *(Bc_Reg*)(bytecode + cursor);
            cursor += sizeof(Bc_Reg);

            uint8_t type;
            type = *(uint8_t*)(bytecode + cursor);
            cursor += sizeof(uint8_t);

            uint8_t is_value = type & 0x80;
            type &= 0x7F;

            uint8_t comp;
            comp = *(uint8_t*)(bytecode + cursor);
            cursor += sizeof(uint8_t);

            Bc_Reg a;
            a = *(Bc_Reg*)(bytecode + cursor);
            cursor += sizeof(Bc_Reg);

            char const* tab[] = { "<", ">=", ">", "<=", "==", "!=" };

            bcc__indent();
            bcc__write("%s __%d = __%d %s ", "int32_t", dst, a, tab[comp]);

            if (is_value) {
                Bc_Value value;
                uint8_t size = bc__type_to_size[type];
                uint8_t* u8_out = (uint8_t*)&value;
                for (int i = 0; i < size; ++i)
                    u8_out[i] = bytecode[cursor + i];
                cursor += size;
                bcc__write("%i;\n", value.S32);
            } else {
                uint32_t b = *(Bc_Reg*)(bytecode + cursor);
                cursor += sizeof(Bc_Reg);
                bcc__write("__%d;\n", b);
            }
        } break;

        case BC_OP_BRANCH: {
            uint32_t location = *(uint32_t*)(bytecode + cursor);
            cursor += sizeof(uint32_t);

            Bc_Reg src = *(Bc_Reg*)(bytecode + cursor);
            cursor += sizeof(Bc_Reg);

            bcc__indent();
            bcs__for (def->branch_count) {
                if (location == def->branch_hints[i]) {
                    bcc__write("if (__%d) goto __loc_%i;\n", src, i);
                    break;
                }
            }
        } break;

        case BC_OP_JUMP: {
            cursor += sizeof(uint32_t);
            bcc__indent();
            bcc__write("goto __endif;\n");
            bcc__write("__else:\n");
        } break;

        case BC_OP_CALL: {
            //uint32_t location = *(uint32_t*)(bytecode + cursor);
            cursor += sizeof(uint32_t);

            uint8_t ret_count = *(uint8_t*)(bytecode + cursor);
            cursor += sizeof(uint8_t);

            uint8_t arg_count = *(uint8_t*)(bytecode + cursor);
            cursor += sizeof(uint8_t);
            
            bcc__indent();
            bcs__for (ret_count) {
                uint32_t dst = *(uint32_t*)(bytecode + cursor);
                cursor += sizeof(uint32_t);
                bcc__write("%s __%d = ", "int32_t", dst);
            }
            bcc__write("%s(", "func");
            bcs__for (arg_count) {
                uint32_t src = *(uint32_t*)(bytecode + cursor);
                cursor += sizeof(uint32_t);
                bcc__write("__%d%s", src, i == arg_count - 1 ? "" : ", ");
            }
            bcc__write(");\n");
        } break;

        case BC_OP_RETURN: {
            uint8_t ret_count;
            ret_count = *(uint8_t*)(bytecode + cursor);
            cursor += sizeof(uint8_t);

            Bc_Reg a;
            bcc__indent();
            bcs__for (ret_count) {
                a = *(Bc_Reg*)(bytecode + cursor);
                cursor += sizeof(Bc_Reg);
                bcc__write("return __%d;\n", a);
            }
        } break;
        
        default: {
            bc__debug_log("invalid bytecode instruction! (pc=%x op=%d)", cursor - 1, operation);
            return;
        } break;
        }
    }

    bcc__write("}");
}
