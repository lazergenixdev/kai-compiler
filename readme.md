
# WIP Experimental Scripting language made for real-time applications.

- Strict static type checking
- Compile-then-execute model (no JIT/Interpreter)
	- *there will be an bytecode interpreter for compile-time execution
- Compiler API written in C, and implemented in C++
- Kai is a place-holder name, I an have no idea what to call this language
- My first compiler, so don't expect anything crazy

Notes:
- only works on Windows (because costum allocator uses VirtualAlloc to reserve multiple pages at a time).
- will use re2c to parse keywords in lexer, I think it is better than using a hash table, but this will need to be tested.
- will use AsmJit for machine code generation, as it will speed up development time significantly.
- todo: put license information in source files
- will need dyncall for compile-time execution of bytecode

# Problems :(

## Typing needs to happen out-of-order (because headers suck very much)
```C++
// Example:
main :: () {
	print("🤩"); // Typing does not know what "print" is!
}

print :: (str) -> void #native
```
Solution: Dependencies and stuff??? IDK lol, typing is suck a huge and interconnected problem

---

## Typing needs to know information that can only be determined at compile-time
```C++
// Example:
test :: () -> Type { ret s32; } // This can be whatever arbitrary code
A :: test();
B : A = 5; // Typing needs to know what the Heck "A" is!
```
Solution: Every declaration gets their own compilation unit that compiles to bytecode, that is able to be executed when necessary. Exactly how do you implement something like that? Please answer my question.