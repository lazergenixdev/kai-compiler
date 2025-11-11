
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
| Operators                   | (Left) Type  | (Right) Type | Constraint                                                   |
| --------------------------- | ------------ | ------------ | ------------------------------------------------------------ |
| `+` `-` `*` `/` `\|` `&`    | Integer      | Integer      | Integers must be the same bit width                          |
| `+` `-`                     | Pointer      | Integer      | Pointer subtype must have size > 0 (i.e. not `void`)         |
| `+` `-` `*` `/`             | Float        | Float        | Floats must be the same bit width                            |
| `->`                        | Any          | #Type        | (Left) type must be convertable to (Right) value             |
| `<` `<=` `>` `>=` `==` `!=` | Any          | Any          | Both types must be comparable **AND** types must be the same |
| `.`                         | Struct-like  | N/A          | (Left) type must contain the (Right) identifier              |
| `[]`                        | Indexable    | Integer      | ...                                                          |
