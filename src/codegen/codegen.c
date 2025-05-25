#include "codegen/codegen.h"
#include "codegen/handlers.h"
#include "codegen/helpers.h"
#include "codegen/symbol.h"
#include "parser/ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int data_label_counter = 0;
int code_label_counter = 0;

void generate_code(ASTNode* node, FILE* output) {

    if (!node) return;
    switch (node->type) {
        case NODE_PRINT: {
            handle_print(node, output);
            break;
        }
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

    FILE* output = fopen("build/asm/program.asm", "w");
    if (!output) {
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
    }

    // init state
    data_label_counter = code_label_counter = 0;
    init_symbol_table();

    collect_variables(node);
    emit_data_section(node, output);
    emit_bss_section(output);
    emit_text_section(node, output);
    emit_itoa(output);

    fclose(output);
    free_symbol_table();
}