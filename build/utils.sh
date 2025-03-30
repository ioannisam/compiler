#!/bin/bash

generate() {
    echo "Generating files..."
    bison -d -o src/parser/parser.tab.c src/parser/parser.y 
    flex -o src/lexer/lex.yy.c src/lexer/lang.l
}

compile() {
    generate
    echo "Compiling executable..."
    gcc -o bin/lexer src/lexer/lex.yy.c src/parser/parser.tab.c -lfl
}

run() {
    compile
    echo "Running executable..."
    ./bin/lexer
}

example() { 
    compile
    echo "Running executable..."
    echo "Running example..."
    echo 'print "Hello, World!";' | ./bin/lexer
}

clean() {
    echo "Cleaning up generated files..."
    rm -f src/parser/parser.tab.c src/parser/parser.tab.h
    rm -f src/lexer/lex.yy.c 
    rm -f bin/* 
}

help() {
    echo "Usage: $0 {generate|compile|run|example|clean}"
    echo "  generate   - Generate files"
    echo "  compile    - Generate and compile files"
    echo "  run        - Generate, compile and run files"
    echo "  example    - Generate, compile, run and execute an example"
    echo "  clean      - Remove all generated files"
}

# script logic
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