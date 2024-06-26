#include <iostream>
#include <iomanip>
#include <vector>
#include "config.hpp"
#include "builtin_types.hpp"

// @TODO: branching, like cmon bro implement some branching already smh my head.
// @TODO: need a stack, need some kind of static context, need globals maybe?
// @TODO: some pointer math would be nice tbh, but should bytecode know about pointer type?? (e.g. *u32 "pointer to u32")
//                                             -> yes probably, instructions on x86 have builtin support for indexing with different strides
//                                                "mov rax, qword ptr [rcx + rax*8]" stride = 8
//                                                "mov rax, qword ptr [rcx + rax*4]" stride = 4

// Note: 
// Stack only really needs to be used for arrays, structs, and values that are "pointed to"

enum Operation {

	Operation_Add = 1, // %dst = add.type %left, %right/imm
	Operation_Sub = 2,
	Operation_Mul = 3,
	Operation_Div = 4,
	
	Operation_Test             = 8,  // cmp.type %left
	Operation_Compare          = 9,  // cmp.type %left, %right/imm
	Operation_Branch           = 10, // branch    address
	Operation_Condition_Branch = 11, // branch.le address
	
	Operation_Copy        = 41, // %dst = %src
	Operation_Load_Value  = 42, // mov rax, 740
	Operation_Load        = 43, // mov rcx, qword ptr [rax + rbx*n] ? %2 = load.s64 [%0 + %1 * n] <=> value := ptr[index];
	Operation_Store       = 44, // mov qword ptr [rax + rbx*n], rcx

	// @Note: need stack for local variables
	// Bytecode does not need to know about stack pointer (rsp)
	Operation_Stack_Alloc = 45, // stack_alloc %1  { sub rsp, %1 }
	Operation_Stack_Load  = 46, // Load from stack ; mov rax, [rbp + offset]
	Operation_Stack_Store = 47, // Store into stack ; mov [rbp + offset], rax
	// stack_alloc 32
	// stack_store.s32 %0, [28]
	// stack_store.s32 %1, [24]

#define X(NAME,TYPE) Operation_Convert_From_ ## NAME,
	XPRIMITIVE_TYPES
#undef  X
	
	Operation_Procedure_Call        = 100,
	Operation_Return                = 101,
	Operation_Native_Procedure_Call = 255, // need dyncall (for compile-time execution)
};

enum Condition {
	LESS_THAN        = 0,
	GREATER_THAN     = 1,
	LESS_OR_EQUAL    = 2,
	GREATER_OR_EQUAL = 3,
	EQUAL            = 4,
	NOT_EQUAL        = 5,
	ZERO             = 6,
	NOT_ZERO         = 7,
};

enum Primitive_Type {
#define X(NAME,TYPE) Prim_Type_ ## NAME,
	XPRIMITIVE_TYPES
#undef  X
	Prim_Type_Pointer,
};

#define PROC_ARG(N) (0xFFFFFFFF - (N))
#define MAX_PROC_ARG_REGISTER (0xFFFFFFFF - 0x100)
#define IMMEDIATE MAX_PROC_ARG_REGISTER

struct Register {
	union {
#define X(NAME,TYPE) NAME NAME ## value;
		XPRIMITIVE_TYPES
#undef  X
		void* ptrvalue;
		u8 _bytes[8];
	};
};

static constexpr inline u32 sizeof_Type(u8 primtype) {
	switch (primtype)
	{
#define X(NAME,TYPE) case Prim_Type_ ## NAME: return sizeof(NAME);
	XPRIMITIVE_TYPES
#undef  X
	default:return 0;
	}
}

// branch position of where to jump when condition is true/false
struct Branch {
	u64 true_branch_pos;
	u64 false_branch_pos;
};

struct Bytecode_Instruction_Stream {
	std::vector<u8> stream; // @TODO: write own dynamic array, we can optimize writing to the buffer way better
	
	u64 position() { return (u64)stream.size(); }

	void insert_copy_operation(u8 primtype, u32 reg_dst, u32 reg_src);

	void insert_primitive_operation(u8 op, u8 primtype, u32 reg_dst, u32 reg_op1, u32 reg_op2);
	void insert_primitive_operation_imm(u8 op, u8 primtype, u32 reg_dst, u32 reg_op1, Register value);

	void insert_load_value(u8 primtype, u32 reg_dst, Register value);
	void insert_load(u8 primtype, u32 reg_dst, u32 reg_index);
	void insert_load(u8 primtype, u32 reg_dst, u64 index_value);

	void stack_allocate(u32 num_bytes);
	void stack_load(u32 offset, u32 reg_dst);
	void stack_store(u32 offset, u32 reg_src);

	void insert_test(u8 primtype, u32 reg_left);
	void insert_compare(u8 primtype, u32 reg_left, u32 reg_right);
	void insert_compare_imm(u8 primtype, u32 reg_left, Register value);
	u64 insert_branch(); // returns branch position
	u64 insert_conditional_branch(u8 condition); // returns branch position
	void set_branch_position(u64 branch_position, u64 address);

	// Example: %6, %7 = call "main" (%1, %4)
	u64 procedure_call(u8 arg_count); // -> branch_position: u64

	// Procedure knows about the types of it's arguments (dont need type metadata, unlike for operations)
	// 
	// Caller should never put the incorrect types into the proc call
	// (because it knows the types at compile-time, so that would be a super idiot move)
	void procedure_input(u32 reg);

	// `count` does NOT need to match the procedure's actual output count,
	// it is for storing the procedure's output into registers
	void procedure_output_count(u8 count);
	
	void procedure_output(u32 reg);

	void insert_return(u32 reg);
	void insert_return(u32 count, u32 const* reg);
	
	template <size_t Count>
	void insert_return(u32 const(&registers)[Count]) { insert_return(Count, registers); }

	void _insert_register(u32 reg);
	void _insert_immediate(u8 primtype, Register value);

	void debug_print();
};