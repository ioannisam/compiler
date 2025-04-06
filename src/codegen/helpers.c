#include "codegen/helpers.h"
#include "codegen/symbol.h"

#include <stdio.h>
#include <stdlib.h>

// Data Section Helpers
void collect_print_messages(ASTNode* node, FILE* output) {
    if (!node) return;
    if (node->type == NODE_PRINT && node->print_expr.expr->type == NODE_STR) {
        fprintf(output, "msg%d db \"%s\", 0xA\n", data_label_counter++, node->print_expr.expr->str_value);
    }
    if (node->type == NODE_COMPOUND) {
        collect_print_messages(node->binop.left, output);
        collect_print_messages(node->binop.right, output);
    }
}

void emit_data_section(ASTNode* node, FILE* output) {
    fprintf(output, "section .data\n");
    collect_print_messages(node, output);
}

// BSS Section Helpers
void collect_variables(ASTNode* node) {
    if (!node) return;
    if (node->type == NODE_ASSIGN && node->binop.left && node->binop.left->type == NODE_IDENT) {
        add_symbol(node->binop.left->str_value, NULL);
    }
    if (node->type == NODE_COMPOUND || node->type == NODE_ASSIGN) {
        collect_variables(node->binop.left);
        collect_variables(node->binop.right);
    }
}

void emit_bss_section(FILE* output) {
    fprintf(output, "section .bss\n");
    fprintf(output, "print_buffer: resb 20\n");
    Symbol* sym = get_symbol_table();
    while (sym) {
        fprintf(output, "%s: resq 1\n", sym->label);
        sym = sym->next;
    }
}

// Text Section Helpers
void emit_text_section(ASTNode* node, FILE* output) {
    fprintf(output, "section .text\n");
    fprintf(output, "global _start\n");
    fprintf(output, "_start:\n");
    generate_code(node, output);
    fprintf(output, "    mov rax, 60\n    xor rdi, rdi\n    syscall\n");
}

void emit_itoa(FILE* output) {
    fprintf(output, "\nitoa:\n"
                    "    push rbx\n    push rcx\n    push rdx\n"
                    "    mov rax, rdi\n    mov rdi, rsi\n    add rdi, 19\n"
                    "    mov rcx, 10\n    mov rbx, 0\n"
                    ".itoa_loop:\n"
                    "    xor rdx, rdx\n    div rcx\n    add dl, '0'\n"
                    "    dec rdi\n    mov [rdi], dl\n    inc rbx\n"
                    "    test rax, rax\n    jnz .itoa_loop\n"
                    "    mov byte [rdi + rbx], 0xA\n    inc rbx\n"
                    "    mov rax, rbx\n    pop rdx\n    pop rcx\n    pop rbx\n    ret\n");
}