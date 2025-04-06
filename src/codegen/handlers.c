#include "codegen/handlers.h"
#include "codegen/symbol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_num(ASTNode* node, FILE* output) {
    fprintf(output, "    mov rax, %d\n", node->num_value);
}

void handle_ident(ASTNode* node, FILE* output) {
    Symbol* sym = lookup_symbol(node->str_value);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->str_value);
        exit(EXIT_FAILURE);
    }
    fprintf(output, "    mov rax, [%s]    ; load variable %s\n", sym->label, sym->name);
}

void handle_print(ASTNode* node, FILE* output) {
    ASTNode *expr = node->print_expr.expr;
    if (expr->type == NODE_STR) {
        int len = (int)strlen(expr->str_value);
        fprintf(output,
            "    mov rax, 1         ; sys_write\n"
            "    mov rdi, 1         ; stdout\n"
            "    mov rsi, msg%d     ; message address\n"
            "    mov rdx, %d        ; message length\n"
            "    syscall\n",
            code_label_counter, len + 1);
        code_label_counter++;
    } else {
        generate_code(expr, output);
        fprintf(output,
            "    push rax\n    push rbx\n    push rcx\n    push rdx\n    push rdi\n    push rsi\n"
            "    mov rdi, rax          ; number to convert\n"
            "    mov rsi, print_buffer ; buffer address\n"
            "    call itoa\n"
            "    mov rsi, print_buffer\n"
            "    add rsi, 20\n"
            "    sub rsi, rax    ; start = buffer_end - length\n"
            "    mov rdx, rax    ; length\n"
            "    mov rax, 1      ; sys_write\n"
            "    mov rdi, 1      ; stdout\n"
            "    syscall\n"
            "    pop rsi\n    pop rdi\n    pop rdx\n    pop rcx\n    pop rbx\n    pop rax\n");
    }
}

void handle_assign(ASTNode* node, FILE* output) {
    generate_code(node->binop.right, output);
    if (node->binop.left && node->binop.left->type == NODE_IDENT) {
        Symbol* sym = lookup_symbol(node->binop.left->str_value);
        if (!sym) {
            fprintf(stderr, "Error: Undefined variable '%s'\n", node->binop.left->str_value);
            exit(EXIT_FAILURE);
        }
        fprintf(output, "    mov [%s], rax    ; assign to variable %s\n", sym->label, sym->name);
    }
}

void handle_binop(ASTNode* node, FILE* output) {
    generate_code(node->binop.left, output);
    fprintf(output, "    push rax\n");
    generate_code(node->binop.right, output);
    fprintf(output, "    pop rbx\n");
    switch (node->binop.op) {
        case OP_ADD:
            fprintf(output, "    add rax, rbx\n");
            break;
        case OP_SUB:
            fprintf(output, "    sub rbx, rax\n");
            fprintf(output, "    mov rax, rbx\n");
            break;
        case OP_MUL:
            fprintf(output, "    imul rax, rbx\n");
            break;
        case OP_DIV:
            fprintf(output, "    xor rdx, rdx\n");
            fprintf(output, "    idiv rbx\n");
            break;
        default:
            fprintf(output, "    ; unsupported operator\n");
            break;
    }
}

void handle_compound(ASTNode* node, FILE* output) {
    generate_code(node->binop.left, output);
    generate_code(node->binop.right, output);
}