# Types
What are types? Types define how to interpret bytes stored in memory.

## Primitive Types
| Name                               | Meaning                    |
| ---------------------------------- | -------------------------- |
| u**8**, u**16**, u**32**, u**64**  | **N**-bit unsigned integer |
| s**8**, s**16**, s**32**, s**64**  | **N**-bit signed integer   |
| f**32**, f**64**                   | **N**-bit floating point   |
| type                               | The type of types          |

## Non-Primitive Types
| Name (**T** = any type) | Meaning                               |
| ----------------------- | ------------------------------------- |
| * **T**                            | Pointer to **T**           |
| [**N**,**M**] **T**     | **N** x **M** Matrix                  |
| [**N**] **T**           | Fixed Array of size **N** (or Vector) |
| [] **T**                | Array Slice                           |
| [..] **T**              | Dynamic Array                         |

**Note**: *Where are strings? Strings are defined to be `[] u8`* (array of bytes), no special type for strings.

# Variables

There are two kinds of variables, **Constants** and **Non-Constants**.
More specifically, **Constants** are special *compile-time constants*
and **Non-Constants** are just regular variables,
so **Non-Constants** are what you will use most of the time.
```C
my_constant     :: 1; // Constants use ':'
my_non_constant := 2; // Non-Constants use '='
```
Variable types are able to be inferred (most of the time) from the context. 
But to specify the type of a variable, put it's type after the first `:`
```C
my_var : u32 : 0x45; // 'u32' can be replaced with any type!
```

Types are *not* special, types are also values.
That means that your variable can be assigned to a type!
```C
My_Type :: f32;          // Assign a variable to a type
my_var : My_Type : 1.23; // Use the type when declaring another variable
```

The value of a variable can be omitted, in which case
the variable is given the initial value of 'zero'
```C
my_var : f32; // same as my_var : f32 = 0;
```

## Variable Assignment
The syntax for reassigning a variable is to just use '='
```C
my_var = 4;
my_var = my_var + 1;
```
**Note**: *<b>Constants</b> cannot be reassigned*

## Value Literals
```Javascript
a :: 123;     // Basic integer
b :: 100_000; // can use '_' for readability (does not affect the value)
c :: 0xFE;    // Hexidecimal base
d :: 3.1415;  // Basic floating point
e :: 1.23e-5; // can use exp notation
f :: "helo!"; // Strings!

// Floating point values have special "directives"
inf :: #infinity;
nan :: #nan;

// Use this idiom for characters
ch :: "h" => u8;
```

## Procedures (Functions)
This language is a procedure programming language,
that means that there must be some way to define *procedures*.
(Think of procedures as just functions,
something that takes *some* input and
transforms it into *some* output)
```C
// define a procedure
add_two :: (num: s32) -> s32 {
	ret num + 2;
}

my_var :: add_two(3); // call the procedure we defined
```
`(num: s32) -> s32` is the type portion of the procedure,
and `{ ret num + 2; }` is the *body*.

The general syntax for the procedure type is
```
(var1: type1, var2: type2, ...) -> return_type
```
But procedures are actually allowed to return more than one value
```
(var1: type1, var2: type2, ...) -> (ret1, ret2, ret3, ...)
```

## Example Program 1
```C
print :: #builtin;

iterate_fibonacci :: (a, b: u32) -> (u32, u32) {
	ret b, a + b;
}

main :: () {
	curr, next : s32 = 1, 1;
	print(curr);
	curr, next = iterate_fibonacci(curr, next);
	print(curr);
	curr, next = iterate_fibonacci(curr, next);
	print(curr);
	curr, next = iterate_fibonacci(curr, next);
	print(curr);
	curr, next = iterate_fibonacci(curr, next);
	print(curr);
}
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

# Structures (Structs)
Most of the time when programming, you want to bundle
multiple types together into one singular type.
This can be done with structs!
```C
Person :: struct {
	name          : [] u8;
	age           : u32;
	height        : u32;
	social_credit : s32;
}

// Now person can be used just like any other type
me : Person;
```
## Struct field access
Every variable contained in a struct is called a "field",
and can be accessed with '.'
```C
me.name = "John";
me.age = 100;
```

## Struct literals
Structure literals are made by using `{}` with assignment statements
```C
person: Person = {
	name = "John";
	age  = 100;
};
```

# Math Types
```C++
// Scalar values are casted to vectors by
// copying it's value to each vector component
vecA : [4] f32 = 4;

// Same applies to when using vectors and
// scalars for arithmetic 
vecB := vecA * 2 - 1;

// Matrix utilities are builtin to the language
matA : [4,4] f32 = math.translation({1, 2, 3});

// Matrix multiplication and vector math
// are also builtin to the language
vecC := matA * math.cross(vecA, vecB);
```
# Fixed Arrays
# Array Slices
# Dynamic Arrays

# Directives
```C
#import
#run
#size_of()
#type_of()
```

```
if T == s32 {
	ret f32;
}

fun :: (n: $T, s: T) -> T {
	ret n + s - 1;
}
```