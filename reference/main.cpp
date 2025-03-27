#include <cstdio>
#define KAI_USE_DEBUG_API
#define KAI_USE_MEMORY_API
#define KAI_USE_CPP_API
#include "../include/kai/kai.h"

using P_main = Kai_s32(Kai_slice);

namespace exports {
    extern "C" void print(Kai_s32 x) {
        printf("%d", x);
    }
}

// ./kai "main.kai" arg1 arg2 ...
int main(int argc, char* argv[]) {
    if (argc < 2) return 0;

    Kai::Program program;
    program.add_native_procedure("print", (void*)&exports::print);

    if (program.compile_from_file("scripts/main.js") != Kai::Success) {
        program.error.print();
        return 1;
    }

    P_main* main_proc = program.find_procedure<P_main>("main");
    if (!main_proc) {
        return 1;
    }

    Kai_slice args {
        .data  = Kai_ptr(argv + 2),
        .count = Kai_u32(argc - 2),
    };
    return main_proc(args);
}
