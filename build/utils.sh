#!/bin/bash

generate() {
    echo "Generating files..."
    bison -d -o src/parser/parser.tab.c --header=include/parser.tab.h src/parser/parser.y
    flex -o src/lexer/lex.yy.c src/lexer/lang.l
}

compile() {
    generate
    echo "Compiling executable..."
    gcc -o bin/compiler -Iinclude src/lexer/lex.yy.c src/parser/parser.tab.c src/parser/ast.c -lfl
}

run() {
    compile
    echo "Running executable..."
    echo "Enter input (press Ctrl+D when done):"
    ./bin/compiler /dev/stdin
}

example() { 
    compile
    echo "Running executable..."
    echo "Running example..."
    ./bin/compiler test/test0.txt
}

clean() {
    echo "Cleaning up generated files..."
    rm -f src/parser/parser.tab.c include/parser.tab.h
    rm -f src/lexer/lex.yy.c 
    rm -f bin/* 
}

help() {
    echo "Usage: $0 {generate|compile|run|example|clean}"
    echo "  generate   - Generate files"
    echo "  compile    - Generate and compile files into an executable"
    echo "  run        - Generate, compile and run interactively"
    echo "  example    - Generate, compile and run an example"
    echo "  clean      - Remove all generated files"
}

# logic
case "$1" in
    generate)
        generate
        ;;
    compile)
        compile
        ;;
    run)
        run
        ;;
    example)
        example
        ;;
    clean)
        clean
        ;;
    *)
        help
        ;;
esac