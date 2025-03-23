#ifndef BYTECODE__H
#define BYTECODE__H
#include <stdint.h>

// Namespaces used:
// Main API:    'BC_'  and 'Bc_"
// Interpreter: 'bci_' and 'bci__'
// Stream:      'bcs_' and 'bcs__'

#ifndef BC__LOG_LEVEL
#define BC__LOG_LEVEL 0
#endif

#if BC__LOG_LEVEL > 0 && !defined(bc__debug_log)
#   define bc__debug_log(...) printf(__VA_ARGS__), putchar('\n')
#else
#   define bc__debug_log(...) (void)0
#endif

#define BC_X_PRIMITIVE_TYPES \
    X( U8,  0,  uint8_t )  \
    X( U16, 1, uint16_t )  \
    X( U32, 2, uint32_t )  \
    X( U64, 3, uint64_t )  \
    X( S8,  4,   int8_t )  \
    X( S16, 5,  int16_t )  \
    X( S32, 6,  int32_t )  \
    X( S64, 7,  int64_t )  \
    X( F32, 8,    float )  \
    X( F64, 9,   double )

enum {
#   define X(name, value, type) BC_TYPE_ ## name = value,
    BC_X_PRIMITIVE_TYPES
#   undef X
};

enum {
    BC_OP_NOP           = 0,  //  nop
    BC_OP_LOAD_CONSTANT = 1,  //  %0 <- load_constant.u32 8
    BC_OP_ADD           = 2,  //  %2 <- add.u32 %0, %1
    BC_OP_SUB           = 3,  //  %7 <- sub.u32 %0, %1
    BC_OP_MUL           = 4,  //  %8 <- mul.u32 %0, %1
    BC_OP_DIV           = 5,  //  %9 <- div.u32 %0, %1
    BC_OP_COMPARE       = 6,  //  %3 <- compare.gt.s64 %0, 0
    BC_OP_BRANCH        = 7,  //  branch %5 {0x43}
    BC_OP_JUMP          = 8,  //  jump 0x43
    BC_OP_CALL          = 9,  //  %4, %5 <- call {0x34} (%1, %2)
    BC_OP_RETURN        = 10, //  ret %1, %2
    BC_OP_NATIVE_CALL   = 11, //  %4 <- native_call {0x100f3430} (%1, %2)
    BC_OP_LOAD          = 12, //  %5 <- u64 [%1 + 0x24]
    BC_OP_STORE         = 13, //  u32 [%2 + 0x8] <- %5
    BC_OP_STACK_ALLOC   = 14, //  %6 <- stack_alloc 48
    BC_OP_STACK_FREE    = 15, //  stack_free 48
    BC_OP_CHECK_ADDRESS = 99, //  check_address.u32 (%1 + 0x8)
};

enum {
    BC_CMP_LT  = 0,
    BC_CMP_GTE = 1,
    BC_CMP_GT  = 2,
    BC_CMP_LTE = 3,
    BC_CMP_EQ  = 4,
    BC_CMP_NE  = 5,
};

typedef uint8_t   Bc_Type;
typedef uint32_t  Bc_Reg;

typedef union {
#   define X(name, value, type) type name;
    BC_X_PRIMITIVE_TYPES
#   undef X
} Bc_Value;

typedef struct {
    char const*  name;
    void const*  address;
    Bc_Type*     input_types;
    Bc_Type      output_type;
    uint8_t      input_count;
} Bc_Native_Procedure;

typedef struct {
    uint8_t* data;
    uint32_t count;
    uint32_t capacity;
} Bc_Stream;

typedef struct {
    Bc_Reg   base_register;
    uint32_t return_address;
} Bc_Procedure_Frame;

typedef struct {
    // Bytecode to Execute
    uint8_t             * bytecode;
    uint32_t              count;

    // State of Execution
    uint32_t              pc;
    uint32_t              flags;

    // Virtual Registers
    Bc_Value            * registers;
    uint32_t              max_register_count;
    Bc_Reg                frame_max_register_written; // used to determine next base register

    // Call Frame Information
    Bc_Procedure_Frame  * call_stack;
    uint32_t              call_stack_count;
    uint32_t              max_call_stack_count;
    Bc_Reg              * return_registers; // where to place returned values
    uint32_t              return_registers_count;
    uint32_t              max_return_values_count;
    
    // Native :)
    Bc_Native_Procedure * native_procedures;
    uint32_t              native_procedure_count;

    // Stack Memory
    uint8_t             * stack;
    uint32_t              stack_size;
    uint32_t              max_stack_size;

    void                * scratch_buffer; // at least 255 x 4 Bytes
} Bc_Interpreter;

typedef struct {
    uint32_t max_register_count;
    uint32_t max_call_stack_count;
    uint32_t max_return_value_count;
    uint32_t max_stack_size;
    void*    memory;
} Bc_Interpreter_Setup;

typedef void Bc_Writer_Proc(void* user, char const* data, int count);

typedef struct {
    void const* bytecode;
    uint32_t    count;
    uint8_t     ret_count;
    uint8_t     arg_count;
    Bc_Native_Procedure* natives;
    uint32_t native_count;
    uint32_t* branch_hints;
    uint32_t  branch_count;
} Bc_Def; // TODO: more explicit name

enum {
    BC_STREAM_SUCCESS      = 0,
    BC_STREAM_ERROR_MEMORY = 1,
    BC_STREAM_ERROR_BRANCH = 2,
};

enum {
    BC_INTERP_FLAGS_DONE       = 1 << 0, // returned from call stack
    BC_INTERP_FLAGS_OVERFLOW   = 1 << 1, // the next bytecode instruction went outside of buffer
    BC_INTERP_FLAGS_INCOMPLETE = 1 << 2, // a bytecode instruction being executed was not able to be fully read
    BC_INTERP_FLAGS_INVALID    = 1 << 3, // instruction was invalid (e.g. %0xFFFFFFF <- 69)
};

int bcs_insert_load_constant(Bc_Stream* stream, uint8_t type, Bc_Reg reg_dst, Bc_Value value);
int bcs_insert_math(Bc_Stream* stream, uint8_t type, uint8_t operation, Bc_Reg reg_dst, Bc_Reg reg_src1, Bc_Reg reg_src2);
int bcs_insert_math_value(Bc_Stream* stream, uint8_t type, uint8_t operation, Bc_Reg reg_dst, Bc_Reg reg_src1, Bc_Value value);
int bcs_insert_compare(Bc_Stream* stream, uint8_t type, uint8_t comparison, Bc_Reg reg_dst, Bc_Reg reg_src1, Bc_Reg reg_src2);
int bcs_insert_compare_value(Bc_Stream* stream, uint8_t type, uint8_t comparison, Bc_Reg reg_dst, Bc_Reg reg_src1, Bc_Value value);
int bcs_insert_branch_location(Bc_Stream* stream, uint32_t location, Bc_Reg reg_src);
int bcs_insert_branch(Bc_Stream* stream, uint32_t* branch, Bc_Reg reg_src);
int bcs_insert_jump_location(Bc_Stream* stream, uint32_t location);
int bcs_insert_jump(Bc_Stream* stream, uint32_t* branch);
int bcs_insert_call(Bc_Stream* stream, uint32_t* branch, uint8_t ret_count, Bc_Reg* reg_ret, uint32_t arg_count, Bc_Reg* reg_arg);
int bcs_insert_return(Bc_Stream* stream, uint8_t count, Bc_Reg* regs);
int bcs_insert_native_call(Bc_Stream* stream, uint8_t use_dst, Bc_Reg reg_dst, Bc_Native_Procedure* proc, Bc_Reg* reg_src);
int bcs_insert_load(Bc_Stream* stream, Bc_Reg reg_dst, uint8_t type, Bc_Reg reg_addr, uint32_t offset);
int bcs_insert_store(Bc_Stream* stream, Bc_Reg reg_src, uint8_t type, Bc_Reg reg_addr, uint32_t offset);
int bcs_insert_check_address(Bc_Stream* stream, uint8_t type, Bc_Reg reg_addr, uint32_t offset);
int bcs_insert_stack_alloc(Bc_Stream* stream, Bc_Reg reg_dst, uint32_t size);
int bcs_insert_stack_free(Bc_Stream* stream, uint32_t size);
int bcs_set_branch(Bc_Stream* stream, uint32_t branch, uint32_t location);
int bcs_reserve(Bc_Stream* stream, uint32_t byte_count);

uint32_t bci_required_memory_size(Bc_Interpreter_Setup* info);
int bci_create(Bc_Interpreter* interp, Bc_Interpreter_Setup* info);
int bci_step(Bc_Interpreter* interp);
void bci_reset(Bc_Interpreter* interp, uint32_t location);
void bci_reset_from_stream(Bc_Interpreter* interp, Bc_Stream* stream);
void bci_reset_from_memory(Bc_Interpreter* interp, void* code, uint32_t size);

void bc_convert_to_string(Bc_Def* def, Bc_Writer_Proc* writer, void* user);
void bc_convert_to_c(Bc_Def* def, Bc_Writer_Proc* writer, void* user);

#endif // BYTECODE__H
