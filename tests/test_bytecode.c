#include "test.h"
#include "stdlib.h"

int run(Kai_Interpreter* interp, Kai_s32 value, Kai_s32* out) {
    kai_interp_reset(interp, 0);
	kai_interp_set_input(interp, 0, (Kai_Value) {.s32 = value});
	kai_interp_push_output(interp, 0);

    int i = 0, max_step_count = 1000;
    while(bci_step(interp) && ++i < max_step_count);

    if (interp->flags != KAI_INTERP_FLAGS_DONE) {
        return 1;
    }

    *out = interp->registers[0].s32;
    return 0;
}

static void write_to_file(void* User, Kai_str String) {
    error_writer.write_string(error_writer.user, String);
}

int bytecode() {
    TEST();

	Kai_Allocator allocator = {0};
	kai_memory_create(&allocator);

    // setup bytecode stream
    Kai_BC_Stream stream = {
		.allocator = &allocator,
	};
	
    uint32_t branch_endif, location_endif;
    uint32_t branch_call0, branch_call1;
	
    {
/*
    procedure "fibonacci" (%0 (n))
        %1 = cmp.bgt.s32 %0 (n), 2
        branch %1 .endif
        %2 = load_value.s32 1
        ret %0
    .endif:
        %3 = sub.s32 %0 (n), 1
        %4 = call "fibonacci" (%3)
        %5 = sub.s32 %0 (n), 2
        %6 = call "fibonacci" (%5)
        %7 = add.s32 %4, %6
        ret %7
*/
	    Kai_u32 location_call = stream.count;
	    kai_bc_insert_compare_value(&stream, KAI_S32, KAI_CMP_GT, 1, 0, (Kai_Value) {.s32 = 2});
	    kai_bc_insert_branch(&stream, &branch_endif, 1);
	    kai_bc_insert_load_constant(&stream, KAI_S32, 2, (Kai_Value) {.s32 = 1});
	    kai_bc_insert_return(&stream, 1, (uint32_t[]) {2});
	    Kai_u32 location_endif = stream.count;
	    kai_bc_insert_math_value(&stream, KAI_S32, KAI_BOP_SUB, 3, 0, (Kai_Value) {.s32 = 1});
	    kai_bc_insert_call(&stream, &branch_call0, 1, (uint32_t[]) {4}, 1, (uint32_t[]) {3});
	    kai_bc_insert_math_value(&stream, KAI_S32, KAI_BOP_SUB, 5, 0, (Kai_Value) {.s32 = 2});
	    kai_bc_insert_call(&stream, &branch_call1, 1, (uint32_t[]) {6}, 1, (uint32_t[]) {5});
	    kai_bc_insert_math(&stream, KAI_S32, KAI_BOP_ADD, 7, 4, 6);
	    kai_bc_insert_return(&stream, 1, (uint32_t[]){7});

	    kai_bc_set_branch(&stream, branch_endif, location_endif);
	    kai_bc_set_branch(&stream, branch_call0, location_call);
	    kai_bc_set_branch(&stream, branch_call1, location_call);
    }

    // create & setup interpreter
    Kai_Interpreter interp;
    {
        Kai_Interpreter_Setup interp_info = {
            .max_register_count     = 4096,
            .max_call_stack_count   = 1024,
            .max_return_value_count = 1024,
        };
        interp_info.memory = malloc(kai_interp_required_memory_size(&interp_info));
        kai_interp_create(&interp, &interp_info);
    }
    kai_interp_load_from_stream(&interp, &stream);
	
    Kai_s32 expected[] = { 1, 1, 2, 3, 5, 8, 13, 21 };

    for (int i = 0; i < 8; ++i) {
        Kai_s32 actual = 0;
        if (run(&interp, i+1, &actual)) return FAIL;
        if (actual != expected[i]) return FAIL;
    }

#if 0 // bytecode -> C
    Kai_Bytecode bytecode = {
        .data  = stream.data,
        .count = stream.count,
        .branch_hints = &location_endif,
        .branch_count = 1,
    };
    Kai_Writer writer = {
        .write = write_to_file,
        .user = NULL,
    };
    kai_bytecode_to_c(&bytecode, &writer);
    write_to_file(NULL, KAI_STRING("\n"));
#endif
	kai_memory_destroy(&allocator);

    return PASS;
}