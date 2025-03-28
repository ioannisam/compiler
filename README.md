# Compiler

This project aims to implement a `compiler` for a simple programming language.

## Tools

- **Bison**: A parser generator that converts a grammar description for an LALR(1) context-free grammar into a C program for parsing.
- **Flex**: A lexical analyzer generator that produces C scanner programs based on regular expressions.
- **GCC**: The GNU Compiler Collection, used to compile the generated C code into an executable.

## Features

- **Lexer**:
  - Recognizes keywords like `print`, `if`, `else`, and `while`.
  - Identifies numbers, identifiers, operators (`=`, `==`, `<`, `>`, `+`, `-`, `*`, `/`) and special characters like `;`.
  - Ignores whitespace and comments.

- **Parser**:
  - Supports basic statements:
    - `print` statements.
    - `if` and `while` constructs.
    - Simple expressions with assignment and comparison.
  - Grammar rules are defined in `parser.y`.

## File Structure

- `lang.l`: Lexer definition written in Flex.
- `parser.y`: Parser definition written in Bison.
- `parser.tab.c` and `parser.tab.h`: Generated parser files.
- `lex.yy.c`: Generated lexer file.
- `.gitignore`: Ignores generated files (`parser.tab.c`, `parser.tab.h`, `lex.yy.c`, etc.).
- `README.md`: Documentation for the project.

## How to Build

1. Install **Flex** and **Bison**:
   ```bash
   sudo pacman -S flex bison
   ```
   *(Use the package manager specific to your operating system)*
2. Generate parser files:
   ```bash
   bison -d parser.y
   ```
3. Generate lexer file:
   ```bash
   flex lang.l
   ```
4. Compile executable `lexer`:
   ```bash
   gcc lex.yy.c parser.tab.c -o lexer
   ```
5. Run executable `lexer`:
   ```bash
   ./lexer
   ```
6. Input expression:
   ```bash
   print "Hello World!";
   ```
   *Expected output:* PRINT STRING("Hello World!") SEMICOLON NEWLINE
   
   ```bash
   print "No Semiclon"
   ```
   *Expected output:* PRINT STRING("No Semicolon") NEWLINE Error: syntax error