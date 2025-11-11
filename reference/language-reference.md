# Hello world
```odin
#import "debug";

main :: () {
	debug.println("hello world");
}
```

# Types

## Primitive Types
| Name                   | Meaning                            |
|------------------------| ---------------------------------- |
| `void`                 |  `void` type (same as C)           |
| `u8` `u16` `u32` `u64` | (8,16,32,64)-bit unsigned integer  |
| `s8` `s16` `s32` `s64` | (8,16,32,64)-bit signed integer    |
| `f32` `f64`            | (32,64)-bit floating point         |
| `#Type`                | Type of types                      |

## Non-Primitive Types
| Syntax      | Meaning                                         |
| ----------- | ----------------------------------------------- |
| `* T`       | Pointer to `T`                                  |
| `[N,M] T`   | `N` x `M` Matrix of `T`                         |
| `[N] T`     | Fixed Array of `T` of size `N` (or Vector)      |
| `[] T`      | Array Slice of `T`                              |
| `[..] T`    | Dynamic Array of `T`                            |
| `[K] T`     | Hash Table with key type `K` and value type `T` |
| `string`    | String (builtin alias for `[] u8`)              |

# Directives
| Name               | Meaning                                                              |
| ------------------ | -------------------------------------------------------------------- |
| `#import`          | import a module                                                      |
| `#export`          | expose variable/procedure to host environment                        |
| `#host_import`     | value imported from host environment                                 |
| `#require_export`  | ...                                                                  |
| `#optional_export` | ...                                                                  |
| `#size(T)`         | get size of type `T` (in bytes)                                      |
| `#type(E)`         | get type of expression `E`                                           |
| `#Type`            | the type of all types                                                |
| `#Number`          | special number type only available at compile-time (remove?)         |
| `#Code`            | the type of AST nodes, used for meta-programming                     |
| `#through`         | flow into next case statement                                        |
| `#char`            | get unicode codepoint from next string token                         |
| `#multi`           | create u32 number from up to 4 characters (u8) in next string token  |
| `#array`           | parse next expression as an array type                               |
| `#map`             | parse next expression as a hash table type                           |
| `#proc`            | parse next expression as a procedure type                            |

## Value Literals
```Javascript
a :: 123;         // Basic integer
b :: 100_000;     // can use any number of '_' for readability (does not affect the value)
c :: 0b01011;     // Binary (base 2)
c :: 0xFE;        // Hexidecimal (base 16)
d :: 3.1415;      // Basic floating point
e :: 1.23e-5;     // can use exp notation (reads as 1.23 times 10 to the power -5)
f :: "helo!";     // Strings!
g :: #char "h";   // Single character (Unicode codepoint value)
h :: #multi "->"; // Multi character

// Floating point values also have special "directives"
inf :: #infinity;
nan :: #nan;
```

## Type Casting and Type Punning
Commonly you want to convert from one type to another,
this is called **Type Casting** and this is done by using
the `->` operator.
```C
a : f32 = 4.0;
b : s32 = 23;

// Cast f32 to s32
b = a -> s32; // b is now set to 4
```
Another common situation is if you want to convert to another type,
but without any conversion so the bytes stay the same.
In C++ this is called `reinterpret_cast` or `bit_cast` because you
are literally reinterpreting the data (bytes) from one format to another.
In this language this is called **Type Punning**.
```C
// Pun s32 to f32
a :: b => f32; // now the bytes of b are seen as a floating point
```
**Note**: *With type punning, the size of the result type
<b>must</b> be less than or equal to the size of source type*

# Polymorphic Procedures
```odin
base :: #import "Base";

fun :: (n: $T, s: T) -> T
where base.is_integer(T)
{
	ret n + s - 1;
}
```
