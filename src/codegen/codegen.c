#include "codegen/codegen.h"
#include "codegen/handlers.h"
#include "codegen/helpers.h"
#include "codegen/symbol.h"
#include "parser/ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void generate_code(ASTNode* node, FILE* output) {

    if (!node) return;
    switch (node->type) {
        case NODE_PROGRAM: {
            handle_program(node, output);
            break;
        }
        case NODE_FUNC: {
            handle_function(node, output);
            break;
        }
        case NODE_CALL:
            handle_call(node, output);
            break;
        case NODE_PRINT: {
            handle_print(node, output);
            break;
        }
        case NODE_DECL:
            handle_decl(node, output);
            break;
        case NODE_ASSIGN: {
            handle_assign(node, output);
            break;
        }
        case NODE_IF: {
            handle_if(node, output);
            break;
        }
        case NODE_WHILE: {
            handle_while(node, output);
            break;
        }
        case NODE_BREAK: {
            handle_break(node, output);
            break;
        }
        case NODE_RETURN: {
           handle_return(node, output);
           break;
        }
        case NODE_NUM:
            handle_num(node, output);
            break;
        case NODE_IDENT:
            handle_ident(node, output);
            break;
        case NODE_BINOP: {
            handle_binop(node, output);
            break;
        }
        case NODE_UNOP: {
            handle_unop(node, output);
            break;
        }
        case NODE_COMPOUND:
            handle_compound(node, output);
            break;
        default:
            break;
    }
}

void generate_code_to_file(ASTNode* node) {
    FILE* output = fopen("build/MIXAL/program.mixal", "w");
    if (!output) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    data_label_counter = code_label_counter = 0;
    init_symbol_table();
    collect_variables(node);
    
    fprintf(output, "* GENERATED MIXAL PROGRAM\n");
    fprintf(output, "TERM    EQU 19\n");
    fprintf(output, "        ORIG 3000\n");
    
    collect_and_define_strings(node, output);
    
    generate_code(node, output);
    
    fprintf(output, "* Variables\n");
    fprintf(output, "TEMP    CON 0\n");
    fprintf(output, "TEMP2   CON 0\n");
    fprintf(output, "RETADDR CON 0\n");
    fprintf(output, "ARG     CON 0\n");
    fprintf(output, "RETVAL  CON 0\n");
    fprintf(output, "BUFFER  CON 0\n");
    
    Symbol* sym = get_symbol_table();
    while (sym) {
        fprintf(output, "%-8s CON 0\n", sym->name);
        sym = sym->next;
    }
    
    fprintf(output, "        END START\n");
    fclose(output);
    free_symbol_table();
}