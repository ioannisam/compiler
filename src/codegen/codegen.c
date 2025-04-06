// codegen.c
#include "codegen/codegen.h"
#include "codegen/symbol.h"
#include "parser/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int data_label_counter = 0;
static int code_label_counter = 0;

static void generate_num(ASTNode* node, FILE* output) {
    fprintf(output, "    mov rax, %d\n", node->num_value);
}

static void generate_ident(ASTNode* node, FILE* output) {
    Symbol* sym = lookup_symbol(node->str_value);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->str_value);
        exit(EXIT_FAILURE);
    }
    fprintf(output, "    mov rax, [%s]    ; load variable %s\n", sym->label, sym->name);
}

void generate_code(ASTNode* node, FILE* output) {
    if (!node) return;
    switch (node->type) {
        case NODE_PRINT: {
            ASTNode *expr = node->print_expr.expr;
            if (expr->type == NODE_STR) {
                fprintf(output, "    mov rax, 1         ; sys_write\n");
                fprintf(output, "    mov rdi, 1         ; stdout\n");
                fprintf(output, "    mov rsi, msg%d     ; message address\n", code_label_counter);
                int len = (int)strlen(expr->str_value);
                fprintf(output, "    mov rdx, %d        ; message length\n", len + 1);
                fprintf(output, "    syscall\n");
                code_label_counter++;
            } else {
                generate_code(expr, output);
                fprintf(output, "    push rax\n");
                fprintf(output, "    push rbx\n");
                fprintf(output, "    push rcx\n");
                fprintf(output, "    push rdx\n");
                fprintf(output, "    push rdi\n");
                fprintf(output, "    push rsi\n");
                fprintf(output, "    mov rdi, rax          ; number to convert\n");
                fprintf(output, "    mov rsi, print_buffer ; buffer address\n");
                fprintf(output, "    call itoa\n");
                fprintf(output, "    mov rsi, print_buffer\n");
                fprintf(output, "    add rsi, 20\n");
                fprintf(output, "    sub rsi, rax    ; start = buffer_end - length\n");
                fprintf(output, "    mov rdx, rax    ; length\n");
                fprintf(output, "    mov rax, 1      ; sys_write\n");
                fprintf(output, "    mov rdi, 1      ; stdout\n");
                fprintf(output, "    syscall\n");
                fprintf(output, "    pop rsi\n");
                fprintf(output, "    pop rdi\n");
                fprintf(output, "    pop rdx\n");
                fprintf(output, "    pop rcx\n");
                fprintf(output, "    pop rbx\n");
                fprintf(output, "    pop rax\n");
            }
            break;
        }
        case NODE_ASSIGN: {
            generate_code(node->binop.right, output);
            if (node->binop.left && node->binop.left->type == NODE_IDENT) {
                Symbol* sym = lookup_symbol(node->binop.left->str_value);
                if (!sym) {
                    fprintf(stderr, "Error: Undefined variable '%s'\n", node->binop.left->str_value);
                    exit(EXIT_FAILURE);
                }
                fprintf(output, "    mov [%s], rax    ; assign to variable %s\n", sym->label, sym->name);
            }
            break;
        }
        case NODE_NUM:
            generate_num(node, output);
            break;
        case NODE_IDENT:
            generate_ident(node, output);
            break;
        case NODE_BINOP: {
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
            break;
        }
        case NODE_COMPOUND:
            generate_code(node->binop.left, output);
            generate_code(node->binop.right, output);
            break;
        default:
            break;
    }
}

static void collect_print_messages(ASTNode* node, FILE* output) {
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
        default:
            break;
    }
}

static void collect_variables(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_ASSIGN:
            if (node->binop.left && node->binop.left->type == NODE_IDENT) {
                add_symbol(node->binop.left->str_value, NULL);
            }
            collect_variables(node->binop.right);
            break;
        case NODE_COMPOUND:
            collect_variables(node->binop.left);
            collect_variables(node->binop.right);
            break;
        case NODE_PRINT:
            collect_variables(node->print_expr.expr);
            break;
        default:
            break;
    }
}

void generate_code_to_file(ASTNode* node) {
    FILE* output = fopen("build/asm/program.asm", "w");
    if (!output) {
        perror("Failed to open build/asm/program.asm for writing");
        exit(EXIT_FAILURE);
    }
    
    data_label_counter = 0;
    code_label_counter = 0;
    init_symbol_table();
    
    fprintf(output, "section .data\n");
    collect_print_messages(node, output);
    
    collect_variables(node);
    
    fprintf(output, "section .bss\n");
    fprintf(output, "print_buffer: resb 20\n");
    Symbol *sym = get_symbol_table();
    while (sym) {
        fprintf(output, "%s: resq 1\n", sym->label);
        sym = sym->next;
    }
    
    fprintf(output, "section .text\n");
    fprintf(output, "global _start\n");
    fprintf(output, "_start:\n");
    
    generate_code(node, output);
    
    fprintf(output, "    mov rax, 60\n");
    fprintf(output, "    xor rdi, rdi\n");
    fprintf(output, "    syscall\n");
    
    fprintf(output, "\nitoa:\n");
    fprintf(output, "    push rbx\n");
    fprintf(output, "    push rcx\n");
    fprintf(output, "    push rdx\n");
    fprintf(output, "    mov rax, rdi        ; number to convert\n");
    fprintf(output, "    mov rdi, rsi        ; buffer address\n");
    fprintf(output, "    add rdi, 19         ; move to end of buffer\n");
    fprintf(output, "    mov rcx, 10\n");
    fprintf(output, "    mov rbx, 0          ; length starts at 0\n");
    fprintf(output, ".itoa_loop:\n");
    fprintf(output, "    xor rdx, rdx\n");
    fprintf(output, "    div rcx             ; rax = quotient, rdx = remainder\n");
    fprintf(output, "    add dl, '0'         ; convert to ASCII\n");
    fprintf(output, "    dec rdi             ; move to previous byte\n");
    fprintf(output, "    mov [rdi], dl       ; store digit\n");
    fprintf(output, "    inc rbx             ; increment length\n");
    fprintf(output, "    test rax, rax\n");
    fprintf(output, "    jnz .itoa_loop      ; loop if quotient != 0\n");
    fprintf(output, "    ; Add newline after digits\n");
    fprintf(output, "    mov byte [rdi + rbx], 0xA ; store newline\n");
    fprintf(output, "    inc rbx             ; include newline in length\n");
    fprintf(output, "    mov rax, rbx        ; return total length\n");
    fprintf(output, "    pop rdx\n");
    fprintf(output, "    pop rcx\n");
    fprintf(output, "    pop rbx\n");
    fprintf(output, "    ret\n");
    
    fclose(output);
    free_symbol_table();
}