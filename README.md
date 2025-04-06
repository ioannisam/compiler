# Compiler

**Author:** Ioannis Michalainas

This project implements a `compiler` for a simple programming language, supporting basic statements and expressions. The purpose of this project is **educational**.

## Tools

- **Bison**: Parser generator for LALR(1) context-free grammars  
- **Flex**: Lexical analyzer generator using regular expressions  
- **GCC**: Compiler for building the executable  
- **NASM**: Assembler for converting assembly code into object files  
- **LD**: Linker for creating the final binary  

## Features

### Lexer
- Recognizes keywords (`print`, `if`, `else`, `while`)
- Identifiers, numbers, strings
- Operators: `=`, `==`, `<`, `>`, `>>`, `<<`, `!`, `&`, `|`, `^`, `+`, `-`, `*`, `/`
- Special characters: `;`, `(`, `)`, `{`, `}`
- Ignores whitespace and C-style comments (`/* ... */`)

### Parser
- Supports:
   - `print` statements
   - `if`/`else` conditionals
   - `while` loops
   - Variable assignments
   - Arithmetic and comparison operations
- Error handling for syntax issues
- Builds an **Abstract Syntax Tree (AST)** for semantic analysis

### Code Generation
- Generates assembly code (currently supports the `print` statement)
- Outputs an assembly file to **build/asm/program.asm**

## Files

### Core Components
- `src/parser/parser.y`: Bison parser definition (grammar rules)
- `src/lexer/lang.l`: Flex lexer definition (token rules)
- `include/ast.h`: AST node declarations (types, structures, and utilities)
- `src/parser/ast.c`: AST implementation (node constructors and traversal logic)
- `src/codegen/codegen.c`: Code generation implementation (writes assembly code)

### Generated Files
- `src/parser/parser.tab.c` / `include/parser.tab.h`: Parser files generated by **Bison**
- `src/lexer/lex.yy.c`: Lexer file generated by **Flex**

### Build Artifacts
- **Compiler Executable**: `bin/compiler`
- **Assembly File**: `build/asm/program.asm`
- **Object File**: `build/asm/program.o`
- **Final Binary**: `build/bin/program`

### Additional Files
- `utils.sh`: Build script to automate generation, compilation, assembly, linking, and running
- `test/`: Example input files for testing
- `README.md`: Project overview and usage guide
- `LICENSE`: Contains the GNU General Public License (GPLv3)
- `TODO.txt`: List of future improvements

## How to Build

You will need to install the following tools:
```bash
sudo pacman -S bison flex nasm gcc
```
*(Or use the package manager specific to your Operating System.)*

### Build Using the Script

Use the provided build script (`utils.sh` located at the project root) to automate the process. The script supports the following commands:

- **`generate`**: Generate parser and lexer files using **Bison** and **Flex**.
- **`compile`**: Generate parser and lexer files, then compile the compiler executable (located at `bin/compiler`).
- **`run`**: Run the compiler executable with an input file (it will generate the assembly file in `build/asm/program.asm`).
- **`assemble`**: Assemble the generated assembly file (`build/asm/program.asm`) into an object file (`build/asm/program.o`).
- **`link`**: Link the object file (`build/asm/program.o`) to produce the final binary (`build/bin/program`).
- **`binary`**: Run the final binary (`build/bin/program`).
- **`build`**: Run the full pipeline — generate, compile, run the compiler, then assemble and link to produce the binary.
- **`example`**: Run the compiler with a predefined example input (`test/test0.txt`), then assemble, link, and run the final binary.
- **`clean`**: Remove all generated files and build artifacts.
- **`help`**: Display this help message.

#### Example:
```bash
./utils.sh build
./utils.sh binary
```

### Manual Build Steps

1. Generate parser files:
   ```bash
   bison -d -o src/parser/parser.tab.c --header=include/parser.tab.h src/parser/parser.y
   ```
2. Generate lexer file:
   ```bash
   flex -o src/lexer/lex.yy.c src/lexer/lang.l
   ```
3. Compile the compiler executable:
   ```bash
   gcc -o bin/compiler -Iinclude src/lexer/lex.yy.c src/parser/parser.tab.c src/parser/ast.c src/codegen/codegen.c src/codegen/symbol.c -lfl
   ```
4. Run the compiler to generate assembly:
   ```bash
   ./bin/compiler <input_file>
   ```
   *Or run it on an example file:*
   ```bash
   ./bin/compiler test/test0.txt
   ```
5. Assemble the generated assembly file:
   ```bash
   nasm -f elf64 build/asm/program.asm -o build/asm/program.o
   ```
6. Link the object file to produce the final binary:
   ```bash
   ld build/asm/program.o -o build/bin/program
   ```
7. Run the final binary:
   ```bash
   ./build/bin/program
   ```

- **Valid example:**
   ```bash
   Input:
      print "Hello, World!";
   Expected Output:
      (Binary prints "Hello, World!" to stdout)
   ```

- **Error example:**
   ```bash
   Input:
      print "No Semicolon"
   Expected Output:
      Syntax error at line 1: syntax error (near 'end of input')
   ```