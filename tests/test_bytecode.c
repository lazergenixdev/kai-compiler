#include "test.h"
#include "../src/bytecode.h"
#include "stdlib.h"

int run(Bc_Interpreter* interp, Kai_s32 value, Kai_s32* out) {
    bci_reset(interp, 0);
    interp->return_registers[interp->return_registers_count++] = 0;
    interp->registers[0].S32 = value;

    int i = 0, max_step_count = 1000;
    while(bci_step(interp) && i++ < max_step_count);

    if (interp->flags != BC_INTERP_FLAGS_DONE) {
        return 1;
    }

    *out = interp->registers[0].S32;
    return 0;
}

static void write_to_file(void* user, char const* data, int count) {
    error_writer.write_string(error_writer.user, (Kai_str) {
        .count = (Kai_u32)count, .data = (Kai_u8*)data,
    });
}

int32_t func(int32_t __0) {
;   int32_t __1 = __0 > 2;
;   if (__1) goto __loc_0;
;   int32_t __2 = 1;
;   return __2;
__loc_0:
;   int32_t __3 = __0 - 1;
;   int32_t __4 = func(__3);
;   int32_t __5 = __0 - 2;
;   int32_t __6 = func(__5);
;   int32_t __7 = __4 + __6;
;   return __7;
}

int bytecode() {
    TEST();

    // setup bytecode stream
    Bc_Stream stream = {0};

    uint32_t branch_endif, location_endif;
    uint32_t branch_call0, branch_call1,  location_call;
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
        location_call = stream.count;
        bcs_insert_compare_value(&stream, BC_TYPE_S32, BC_CMP_GT, 1, 0, (Bc_Value) {.S32 = 2});
        bcs_insert_branch(&stream, &branch_endif, 1);
        bcs_insert_load_constant(&stream, BC_TYPE_S32, 2, (Bc_Value) {.S32 = 1});
        bcs_insert_return(&stream, 1, (uint32_t[]) {2});
        location_endif = stream.count;
        bcs_insert_math_value(&stream, BC_TYPE_S32, BC_OP_SUB, 3, 0, (Bc_Value) {.S32 = 1});
        bcs_insert_call(&stream, &branch_call0, 1, (uint32_t[]) {4}, 1, (uint32_t[]) {3});
        bcs_insert_math_value(&stream, BC_TYPE_S32, BC_OP_SUB, 5, 0, (Bc_Value) {.S32 = 2});
        bcs_insert_call(&stream, &branch_call1, 1, (uint32_t[]) {6}, 1, (uint32_t[]) {5});
        bcs_insert_math(&stream, BC_TYPE_S32, BC_OP_ADD, 7, 4, 6);
        bcs_insert_return(&stream, 1, (uint32_t[]){7});

        bcs_set_branch(&stream, branch_endif, location_endif);
        bcs_set_branch(&stream, branch_call0, location_call);
        bcs_set_branch(&stream, branch_call1, location_call);
    }

    // create & setup interpreter
    Bc_Interpreter interp;
    {
        Bc_Interpreter_Setup interp_info = {
            .max_register_count     = 4096,
            .max_call_stack_count   = 1024,
            .max_return_value_count = 1024,
        };
        interp_info.memory = malloc(bci_required_memory_size(&interp_info));
        bci_create(&interp, &interp_info);
    }
    bci_reset_from_stream(&interp, &stream);

    Kai_s32 expected[] = { 1, 1, 2, 3, 5, 8, 13, 21 };

    for (int i = 0; i < 8; ++i) {
        Kai_s32 actual = 0;
        if (run(&interp, i+1, &actual)) return FAIL;
        if (actual != expected[i]) return FAIL;
    }

#if 1 // bytecode -> C
    Bc_Def def = {
        .bytecode = stream.data,
        .count    = stream.count,
        .branch_hints = &location_endif,
        .branch_count = 1,
    };
    bc_convert_to_c(&def, &write_to_file, NULL);
    write_to_file(NULL, "\n", 1);
#endif

    return PASS;
}