# `li1I`

`li1I` is a domain-specific programming language for integer calculations. It supports recursive functions, conditional execution, and expresses arithmetic in [Reverse Polish notation](https://en.wikipedia.org/wiki/Reverse_Polish_notation). 

The only valid characters in an `li1I` program are `l`, `i`, `1`, `I` and whitespace. There is no syntax for comments. Comments are not necessary in a language of such clarity.

This repository contains both a definition of the `li1I` language and an implementation based on LLVM 10 which supports interpretation through a JIT compiler and AOT compilation for linking with other programs. 

## Language
### Program

An `li1I` program consists of the token `li1I` followed by any number of functions enclosed in braces. Left brace is spelled `l1iI` and right brace is spelled `l1Ii`. Here is an empty program:

```
li1I
l1iI

l1Ii
```

### Literals and Identifiers

Function identifiers are of the form `I[iI1l]*`.

Variable identifiers are of the form `i[iI1l]*`.

The only type supported in `li1I` is an integer. I don't know how big the integral type is and I can't be bothered checking. The value of an integer is given by the length of a string of `1`s. E.g. `1` has the value `0`, `11` has the value `2`, `1111111111` has the value `10` and so on. 

### Functions

A function is indicated with the token `lI1i`, which is followed by a function identifier, any number of variable identifiers (parameters), then a brace-enclosed block.

In that brace enclosed block, any number of statements are allowed.

### Statements

A statement is an expression followed by the token `l1ii`.

### Expression

An expression can be:

- The name of a variable, which evaluates to its value.
- A variable declaration, spelled `liI1 &gt;variable identifier&lt; lIi1 &gt;expression&lt;`, which evaluates to the value of the initialiser.
- A function call, spelled `&gt;function identifier&lt; li1l &gt;expression&lt;* lil1`, which evaluates to the result of the function call.
- An arithmetic or logical expression (see below).
- A conditional expression (see below).

#### Arithmetic and Logical Expressions

Arithmetic and logical expressions in `li1I` are written in [Reverse Polish notation](https://en.wikipedia.org/wiki/Reverse_Polish_notation).

The following arithmetic operations and their spelling in `li1I` are:

- Addition: `llli`
- Subtraction: `llii`
- Multiplication: `liil`
- Integer division: `llil`
- Exponentiation: `liii`


The following logical operations and their spelling in `li1I` are:

- Greater than: `ll1i`
- Less than: `ll1I`
- Equality: `ll11`
- Inequality: `l111`

#### Conditional Expressions

A conditional expression is of the form:

```
l1i1 li1l &gt;arithmetic or logical expression&lt; lil1
l1iI
        &gt;then statements&lt;*
l1Ii 
l1il l1iI
        &gt;else statements&lt;*
l1Ii
```

Is a then with no else supported? Who knows.

### Examples

Here's what a factorial function looks like:

```
li1I
l1iI
        lI1i I li1l i lil1
                l1i1 li1l i 11 ll11 l1ii lil1
                        11 l1ii
                l1il
                        i 11 llii I i liil l1ii
                l1ii

        lI1i IIII
                liI1 i lIi1 11111111111 l1ii
                I l1ii
l1Ii
``` 

## Compiler

The compiler supplied in this repository is also called `li1I` and is based on LLVM 10. It supports interpretation through JIT compilation and also AOT compilation to an object file (which you'll need to link using your system linker).

### Usage

```
l1iI &gt;input file&lt; [&gt;flags&lt;]
```

Flags:

- `-o &gt;file&lt;`: Write output to `file`
- `-e`: Execute the program immediately
- `-c`: Compile to an object file
- `--emit-ast`: Emit l1iI AST files for source inputs
- `--emit-llvm`: Emit the LLVM representation for assembler and object files
- `--emit-tokens`: Emit lexer tokens

Object files output by `l1iI` depend on libc, so you'll want to link them like so:

```bash
l1iI factorial.li #outputs factorial.o
gcc factorial.o -o factorial
```

