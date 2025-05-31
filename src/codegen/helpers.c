#include "codegen/helpers.h"
#include "codegen/symbol.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Main Check
bool has_main_function(ASTNode* functions) {
    while (functions) {
        if (functions->type == NODE_COMPOUND) {
            ASTNode* func_node = functions->binop.left;
            if (func_node->type == NODE_FUNC && 
                strcmp(func_node->func.name, "main") == 0) {
                return true;
            }
        }
        functions = functions->binop.right;
    }
    return false;
}

void verify_symbols(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_IDENT:
            if (!lookup_symbol(node->str_value)) {
                fprintf(stderr, "Error: Undefined variable '%s'\n", node->str_value);
                exit(EXIT_FAILURE);
            }
            break;
            
        case NODE_ASSIGN:
            if (node->assign.target->type == NODE_IDENT) {
                if (!lookup_symbol(node->assign.target->str_value)) {
                    fprintf(stderr, "Error: Undefined variable '%s'\n", node->assign.target->str_value);
                    exit(EXIT_FAILURE);
                }
            }
            verify_symbols(node->assign.value);
            break;
        case NODE_CALL:
            {
                ASTNode* args = node->func_call.args;
                while (args) {
                    if (args->type == NODE_COMPOUND) {
                        verify_symbols(args->binop.left);
                        args = args->binop.right;
                    } else {
                        verify_symbols(args);
                        break;
                    }
                }
            }
            break;
        case NODE_BINOP:
            verify_symbols(node->binop.left);
            verify_symbols(node->binop.right);
            break;
        case NODE_UNOP:
            verify_symbols(node->unop.operand);
            break;
        case NODE_PRINT:
            verify_symbols(node->print_expr.expr);
            break;
        case NODE_IF:
            verify_symbols(node->control.condition);
            verify_symbols(node->control.if_body);
            if (node->control.else_body) {
                verify_symbols(node->control.else_body);
            }
            break;
        case NODE_WHILE:
            verify_symbols(node->control.condition);
            verify_symbols(node->control.loop_body);
            break;
        case NODE_RETURN:
            if (node->return_stmt.expr) {
                verify_symbols(node->return_stmt.expr);
            }
            break;
        case NODE_COMPOUND:
            verify_symbols(node->binop.left);
            verify_symbols(node->binop.right);
            break;
        case NODE_PROGRAM:
            verify_symbols(node->program.functions);
            verify_symbols(node->program.main_block);
            break;
        case NODE_FUNC:
            verify_symbols(node->func.body);
            break;
    }
}

// Data Section Helpers
void collect_print_messages(ASTNode* node, FILE* output) {

    if (!node) return;
    switch (node->type) {
        case NODE_PRINT:
            if (node->print_expr.expr && node->print_expr.expr->type == NODE_STR) {
                fprintf(output, "msg%d db \"%s\", 0xA\n", data_label_counter++, 
                        node->print_expr.expr->str_value);
            }
            break;
        case NODE_PROGRAM:
            collect_print_messages(node->program.functions, output);
            collect_print_messages(node->program.main_block, output);
            break;
        case NODE_COMPOUND:
            collect_print_messages(node->binop.left, output);
            collect_print_messages(node->binop.right, output);
            break;
        case NODE_IF:
            collect_print_messages(node->control.if_body, output);
            if (node->control.else_body) {
                collect_print_messages(node->control.else_body, output);
            }
            break;
        case NODE_WHILE:
            collect_print_messages(node->control.loop_body, output);
            break;
        case NODE_FUNC:
            collect_print_messages(node->func.body, output);
            break;
    }
}

void emit_data_section(ASTNode* node, FILE* output) {
    fprintf(output, "section .data\n");
    collect_print_messages(node, output);
}

// BSS Section Helpers
void collect_variables(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_DECL:
            add_symbol(node->decl.name, NULL, node->decl.type);
            break;
        case NODE_PARAM:
            add_symbol(node->param.name, NULL, node->param.type);
            break;
        case NODE_FUNC:
            collect_variables(node->func.params);
            collect_variables(node->func.body);
            break;
        case NODE_PROGRAM:
            collect_variables(node->program.functions);
            collect_variables(node->program.main_block);
            break;
        /* was type agnostic
        case NODE_ASSIGN:
            if (node->binop.left && node->binop.left->type == NODE_IDENT)
                add_symbol(node->binop.left->str_value, NULL, "int");
            break;
        */
    }
    if (node->type == NODE_COMPOUND || node->type == NODE_ASSIGN || node->type == NODE_DECL) {
        collect_variables(node->binop.left);
        collect_variables(node->binop.right);
    } else if (node->type == NODE_IF) {
        collect_variables(node->control.if_body);
        collect_variables(node->control.else_body);
    } else if (node->type == NODE_WHILE) {
        collect_variables(node->control.loop_body);
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
    generate_code(node, output);
}

void emit_itoa(FILE* output) {
    fprintf(output, "\nitoa:\n"
                    "    push rbx\n    push rcx\n    push rdx\n"
                    "    mov rax, rdi          ; rdi contains the number to convert\n"
                    "    mov rdi, rsi          ; rsi is the buffer address\n"
                    "    add rdi, 19           ; move to the end of the buffer\n"
                    "    mov rcx, 10           ; divisor for base 10\n"
                    "    mov rbx, 0            ; character count\n"
                    "    xor r8, r8            ; flag for negative (0 = positive)\n"
                    "    test rax, rax         ; check if the number is negative\n"
                    "    jns .itoa_loop_start  ; jump if non-negative\n"
                    "    mov r8, 1             ; set negative flag\n"
                    "    neg rax               ; make the number positive\n"
                    ".itoa_loop_start:\n"
                    ".itoa_loop:\n"
                    "    xor rdx, rdx          ; clear rdx for division\n"
                    "    div rcx               ; divide rax by 10\n"
                    "    add dl, '0'           ; convert remainder to ASCII\n"
                    "    dec rdi               ; move buffer pointer back\n"
                    "    mov [rdi], dl         ; store the character\n"
                    "    inc rbx               ; increment character count\n"
                    "    test rax, rax         ; check if quotient is 0\n"
                    "    jnz .itoa_loop        ; repeat if quotient is not 0\n"
                    "    test r8, r8           ; check if negative\n"
                    "    jz .itoa_no_sign\n"
                    "    dec rdi               ; move pointer back for '-'\n"
                    "    mov byte [rdi], '-'   ; store the negative sign\n"
                    "    inc rbx               ; increment character count\n"
                    ".itoa_no_sign:\n"
                    "    mov byte [rdi + rbx], 0xA ; add newline character\n"
                    "    inc rbx               ; increment character count\n"
                    "    mov rax, rbx          ; return character count in rax\n"
                    "    pop rdx\n    pop rcx\n    pop rbx\n"
                    "    ret\n");
}