
# Type Constraints
All expressions will have an implicit type rule, where the type of the
expression must "match" the type that is expected from the outer expression.

## Match
A type that matches means:

1. Type must be exactly the type expected
2. TODO

## Definitions
| Term        | Meaning                                                        |
| ----------- | -------------------------------------------------------------- |
| Struct-like | is type struct, type pointer to struct, or type module         |
| Indexable   | is type pointer, type array, type slice, or type dynamic array |

## Binary Expression Constraints
| # | Operators                   | (Left) Type  | (Right) Type     | Constraint                                                   |
| - | --------------------------- | ------------ | ---------------- | ------------------------------------------------------------ |
| 1 | `+` `-` `*` `/` `\|` `&`    | Integer      | Integer          | Integers must be the same bit width                          |
| 2 | `+` `-`                     | Pointer      | Integer          | Pointer subtype must have size > 0 (i.e. not `void`)         |
| 3 | `+` `-` `*` `/`             | Float        | Float            | Floats must be the same bit width                            |
| 4 | `<<` `>>`                   | Integer      | Unsigned Integer | ...                                                          |
| 5 | `->`                        | Any          | #Type            | (Left) type must be convertable to (Right) value             |
| 6 | `<` `<=` `>` `>=` `==` `!=` | Any          | Any              | Both types must be comparable **AND** types must be the same |
| 7 | `.`                         | Struct-like  | N/A              | (Left) type must contain the (Right) identifier              |
| 8 | `[]`                        | Indexable    | Integer          | ...                                                          |



 +  u8 u16 u32 ... f32 f64
u8  Y  N   N       N   N
u16
u32
...
f32
f64

