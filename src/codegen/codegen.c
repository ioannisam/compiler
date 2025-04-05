// codegen.c
#include "codegen.h"
#include "ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int data_label_counter = 0;  // Separate counter for data section
static int code_label_counter = 0;  // Separate counter for text section

// recursive code generation for each AST node (for the text section)
// this pass generates code for each print statement, referencing the corresponding data label
void generate_code(ASTNode* node, FILE* output) {
    if (!node) return;
    switch (node->type) {
        case NODE_PRINT: {
            fprintf(output, "    mov rax, 1         ; sys_write\n");
            fprintf(output, "    mov rdi, 1         ; stdout\n");
            fprintf(output, "    mov rsi, msg%d     ; message address\n", code_label_counter);
            int len = (int)strlen(node->print_expr.expr->str_value);
            fprintf(output, "    mov rdx, %d        ; message length\n", len + 1); // +1 for newline
            fprintf(output, "    syscall\n");
            code_label_counter++;
            break;
        }
        case NODE_COMPOUND:
            generate_code(node->binop.left, output);
            generate_code(node->binop.right, output);
            break;
        /* this is not ready yet
        case NODE_IF:
            generate_code(node->control.if_body, output);
            generate_code(node->control.else_body, output);
            break;
        case NODE_WHILE:
            generate_code(node->control.loop_body, output);
            break;
        // Extend here for additional node types.
        default:
            break;
        */
    }
}

// recursively traverse the AST to output all print messages into the data section
// each print statement gets its own label (msg0, msg1, ...).
void collect_print_messages(ASTNode* node, FILE* output) {
    if (!node) return;
    switch (node->type) {
        case NODE_PRINT: {
            char* msg = node->print_expr.expr->str_value;
            fprintf(output, "msg%d db \"%s\", 0xA\n", data_label_counter, msg);
            data_label_counter++;
            break;
        }
        case NODE_COMPOUND:
            collect_print_messages(node->binop.left, output);
            collect_print_messages(node->binop.right, output);
            break;
        /* nor is this!
        case NODE_IF:
            collect_print_messages(node->control.if_body, output);
            collect_print_messages(node->control.else_body, output);
            break;
        case NODE_WHILE:
            collect_print_messages(node->control.loop_body, output);
            break;
        default:
            break;
        */
    }
}

// wrapper
void generate_code_to_file(ASTNode* node) {
    FILE* output = fopen("build/asm/program.asm", "w");
    if (!output) {
        perror("Failed to open build/asm/program.asm for writing");
        exit(EXIT_FAILURE);
    }
    
    data_label_counter = 0;
    code_label_counter = 0;

    fprintf(output, "section .data\n");
    collect_print_messages(node, output);

    fprintf(output, "section .text\n");
    fprintf(output, "global _start\n");
    fprintf(output, "_start:\n");
    
    // generate code based on the AST.
    generate_code(node, output);
    
    // exit syscall for Linux.
    fprintf(output, "    mov rax, 60        ; sys_exit\n");
    fprintf(output, "    xor rdi, rdi       ; exit code 0\n");
    fprintf(output, "    syscall\n");
    
    fclose(output);
}