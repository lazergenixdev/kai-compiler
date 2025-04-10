
# WIP Experimental Scripting language made for real-time applications.

- Strict static type checking
- Written in C with **no external dependencies**
- Language is most similar to WASM in execution model
- Compile-then-execute model (no JIT/Interpreter)
	- *there is a bytecode interpreter for compile-time execution
- Runtime Safety will be achieved with runtime checks (able to be toggled off for faster performance)
- Kai is a place-holder name, I an have no idea what to call this language
- My first compiler, so don't expect anything crazy

# Building & Running
Code is built to work with MSVC and Clang and GCC.
Build is managed with CMake, so compiling is straightforward
```sh
mkdir build
cd build
cmake ..
```

Running the code (compilation under development)
```sh
./bin/kai --parse-only ../tests/scripts/fibo.kai
```

# Example Code
```
fibonacci :: (n: int) {
    if n <= 2 ret 1;
    ret fibonacci(n-2) + fibonacci(n-1);
}

int :: s32;
```

# Example Usage (C++ API)
```C
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
```
