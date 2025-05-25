#!/bin/bash

generate() {
    echo "Generating parser and lexer files..."
    bison -d -o src/parser/parser.tab.c --header=include/parser/parser.tab.h src/parser/parser.y
    flex -o src/lexer/lex.yy.c src/lexer/lang.l
}

compile() {
    generate
    echo "Compiling the compiler executable..."
    mkdir -p bin
    gcc -o bin/compiler -Iinclude \
        src/lexer/lex.yy.c        \
        src/parser/parser.tab.c   \
        src/parser/ast.c          \
        src/codegen/codegen.c     \
        src/codegen/handlers.c    \
        src/codegen/helpers.c     \
        src/codegen/symbol.c      \
        -lfl
}

run() {
    echo "Running the compiler executable..."
    mkdir -p build/asm
    ./bin/compiler "$@"
}

assemble() {
    echo "Assembling the generated assembly file..."
    nasm -f elf64 build/asm/program.asm -o build/asm/program.o || { echo "Assembly failed"; return 1; }
}

link() {
    echo "Linking the object file to produce the final binary..."
    mkdir -p build/bin
    ld build/asm/program.o -o build/bin/program || { echo "Linking failed"; exit 1; }
}

binary() {
    echo "Running the final binary..."
    echo "|-------------------------|"
    chmod +x build/bin/program
    ./build/bin/program
}

build() {
    echo "Running the full build pipeline..."
    compile
    run "$@"
    assemble
    link
}

example() {
    echo "Running the compiler with predifined example input..."
    compile
    run test/print.txt
    assemble
    link
    binary
}

test() {
    echo "Running all tests from the test folder..."
    
    TEST_FILES=$(find test -type f -name "*.txt" | sort)
    echo -e "Found files: $(ls test/*.txt | wc -l)\n"
    
    set +e # disable automatic exit on error
    compile || true
    for test_file in $TEST_FILES; do
        echo -e "\n➢ Testing with $test_file..."
        
        run "$test_file"
        rc=$?
        # if run was successful
        if [ $rc -eq 0 ]; then
            assemble || true
            # if assembly was successful
            if [ $? -eq 0 ]; then
                link || true
                # if linking was successful
                if [ $? -eq 0 ]; then
                    binary || true
                fi
            fi
        else
            echo "Compiler returned error (skipping assemble/link/binary)"
        fi
        
        echo "-----------------------------------"
    done
}

clean() {
    echo "Cleaning up generated files and build artifacts..."
    rm -f src/parser/parser.tab.c include/parser/parser.tab.h
    rm -f src/lexer/lex.yy.c 
    rm -rf bin
    rm -rf build
}

help() {
    echo "Usage: $0 {generate|compile|run|assemble|link|binary|build|example|clean|help}"
    echo ""
    echo "Commands:"
    echo "  generate       - Generate parser and lexer files using Bison and Flex."
    echo "  compile        - Generate parser and lexer files, then compile the compiler executable."
    echo "  run {input}    - Run the compiler executable with input file."
    echo "  assemble       - Assemble the generated assembly file into an object file."
    echo "  link           - Link the object file to produce the final binary."
    echo "  binary         - Run the final binary."
    echo "  build {input}  - Run the full pipeline: generate, compile, run, assemble and link."
    echo "  example        - Run compiler with predefined example input and run the binary."
    echo "  clean          - Remove all generated files and build artifacts."
    echo "  test           - Run all tests from the test folder."
    echo "  help           - Display this help message."
}

# Command logic
case "$1" in
    generate)
        generate
        ;;
    compile)
        compile
        ;;
    run)
        run "${@:2}"
        ;;
    assemble)
        assemble
        ;;
    link)
        link
        ;;
    binary)
        binary
        ;;
    build)
        build "${@:2}"
        ;;
    example)
        example
        ;;
    test)
        test
        ;;
    clean)
        clean
        ;;
    help|*)
        help
        ;;
esac