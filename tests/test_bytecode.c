#include "test.h"
#include "stdlib.h"

int run(Kai_Interpreter* interp, Kai_s32 value, Kai_s32* out) {
    kai_interp_reset(interp, 0);
	kai_interp_set_input(interp, 0, (Kai_Value) {.s32 = value});
	kai_interp_push_output(interp, 0);

    int i = 0, max_step_count = 1000;
    while(kai_interp_step(interp) && ++i < max_step_count);

    if (interp->flags != KAI_INTERP_FLAGS_DONE) {
        return 1;
    }

    *out = interp->registers[0].s32;
    return 0;
}

int bytecode(void)
{
    TEST();

	Kai_Allocator allocator = {0};
	kai_memory_create(&allocator);

    // setup bytecode stream
    Kai_Bytecode_Encoder encoder = {
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
	    Kai_u32 location_call = encoder.code.count;
	    kai_encode_compare_constant(&encoder, KAI_S32, KAI_CMP_GT, 1, 0, (Kai_Value) {.s32 = 2});
	    kai_encode_branch(&encoder, &branch_endif, 1);
	    kai_encode_load_constant(&encoder, KAI_S32, 2, (Kai_Value) {.s32 = 1});
	    kai_encode_return(&encoder, 1, (uint32_t[]) {2});
	    location_endif = encoder.code.count;
	    kai_encode_math_constant(&encoder, KAI_S32, KAI_MATH_SUB, 3, 0, (Kai_Value) {.s32 = 1});
	    kai_encode_call(&encoder, &branch_call0, 1, (uint32_t[]) {4}, 1, (uint32_t[]) {3});
	    kai_encode_math_constant(&encoder, KAI_S32, KAI_MATH_SUB, 5, 0, (Kai_Value) {.s32 = 2});
	    kai_encode_call(&encoder, &branch_call1, 1, (uint32_t[]) {6}, 1, (uint32_t[]) {5});
	    kai_encode_math(&encoder, KAI_S32, KAI_MATH_ADD, 7, 4, 6);
	    kai_encode_return(&encoder, 1, (uint32_t[]){7});

	    kai_encoder_set_branch(&encoder, branch_endif, location_endif);
	    kai_encoder_set_branch(&encoder, branch_call0, location_call);
	    kai_encoder_set_branch(&encoder, branch_call1, location_call);
    }

    // create & setup interpreter
    Kai_Interpreter interp;
    {
        Kai_Interpreter_Create_Info info = {
            .max_register_count     = 4096,
            .max_call_stack_count   = 1024,
            .max_return_value_count = 1024,
			.allocator              = &allocator,
        };
        kai_interp_create(&info, &interp);
    }
    kai_interp_load_from_encoder(&interp, &encoder);
	
    Kai_s32 expected[] = { 1, 1, 2, 3, 5, 8, 13, 21 };

    for (int i = 0; i < 8; ++i) {
        Kai_s32 actual = 0;
        if (run(&interp, i+1, &actual)) return FAIL;
        if (actual != expected[i]) return FAIL;
    }

	kai_interp_destroy(&interp);

#if 1 // bytecode -> C
    Kai_Bytecode bytecode = {
        .data  = encoder.code.elements,
        .count = encoder.code.count,
        .branch_hints = &location_endif,
        .branch_count = 1,
    };
    kai_bytecode_to_c(&bytecode, error_writer());
    //error_writer.write_string(error_writer.user, KAI_STRING("\n"));
#endif
	kai_memory_destroy(&allocator);

    return PASS;
}
