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

In order to build, you will need to install both **Bison** and **Flex**:
   ```bash
   sudo pacman -S bison flex
   ```
   *(Use the package manager specific to your operating system)*

### Script

You can use the provided script `build/utils.sh` to automate the build process. The script supports the following commands:

- **`generate`**: Generates the parser and lexer files using `bison` and `flex`.
- **`compile`**: Generates the files (if not already generated) and compiles them into an executable binary.
- **`run`**: Generates, compiles and runs the executable.
- **`example`**: Runs the executable with a predefined example input (`print "Hello, World!";`).
- **`clean`**: Removes all generated files and the compiled binary.

Example:
   ```bash
   ./build/utils.sh run
   ```

### Manual

You can instead opt to build the project manually:

1. Generate parser files:
   ```bash
   bison -d -o src/parser/parser.tab.c src/parser/parser.y
   ```
2. Generate lexer file:
   ```bash
   flex -o src/lexer/lex.yy.c src/lexer/lang.l
   ```
3. Compile executable `lexer`:
   ```bash
   gcc -o bin/lexer src/lexer/lex.yy.c src/parser/parser.tab.c -lfl
   ```
4. Run executable `lexer`:
   ```bash
   ./bin/lexer
   ```
5. Input expression:
   ```bash
   print "Hello World!";
   ```
   *Expected output:* PRINT STRING("Hello World!") SEMICOLON NEWLINE
   
   ```bash
   print "No Semiclon"
   ```
   *Expected output:* PRINT STRING("No Semicolon") NEWLINE Error: syntax error