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
    
    ASTNode* expr = node->print_expr.expr;
    if (expr->type == NODE_STR) {
        int len = (int)strlen(expr->str_value);
        // Use global counter and don't increment it - we're referencing existing labels
        static int msg_ref_counter = 0;
        fprintf(output,
            "    mov rax, 1         ; sys_write\n"
            "    mov rdi, 1         ; stdout\n"
            "    mov rsi, msg%d     ; message address\n"
            "    mov rdx, %d        ; message length\n"
            "    syscall\n",
            msg_ref_counter++, len + 1);
    } else {
        generate_code(expr, output);
        fprintf(output,
            "    push rax\n    push rbx\n    push rcx\n    push rdx\n    push rdi\n    push rsi\n"
            "    mov rdi, rax          ; number to convert (signed)\n"
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

void handle_if(ASTNode* node, FILE* output) {
    int current_label = code_label_counter;
    code_label_counter += 2;

    generate_code(node->control.condition, output);
    fprintf(output, "    cmp rax, 0\n");
    fprintf(output, "    je .Lelse%d\n\n", current_label);

    generate_code(node->control.if_body, output);
    fprintf(output, "    jmp .Lend%d\n", current_label);

    fprintf(output, ".Lelse%d:\n", current_label);
    if (node->control.else_body) {
        generate_code(node->control.else_body, output);
    }

    fprintf(output, ".Lend%d:\n\n", current_label);
}

void handle_while(ASTNode* node, FILE* output) {
    // Handle while statement
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
            fprintf(output, "    xchg rax, rbx\n");
            fprintf(output, "    cqo\n");
            fprintf(output, "    idiv rbx\n");
            break;
        case OP_MOD:
            fprintf(output, "    mov rcx, rax      ; Save right value\n");
            fprintf(output, "    mov rax, rbx      ; Move left operand into rax\n");
            fprintf(output, "    mov rbx, rcx      ; Move right operand into rbx\n");
            fprintf(output, "    cqo               ; Sign-extend rax into rdx\n");
            fprintf(output, "    idiv rbx          ; Signed division; remainder in rdx\n");
            fprintf(output, "    mov rax, rdx      ; Move remainder to rax\n");
            break;
        case OP_EQ:
            fprintf(output, "    cmp rbx, rax\n");
            fprintf(output, "    sete al\n");
            fprintf(output, "    movzx rax, al\n");
            break;
        case OP_NEQ:
            fprintf(output, "    cmp rbx, rax\n");
            fprintf(output, "    setne al\n");
            fprintf(output, "    movzx rax, al\n");
            break;
        case OP_LT:
            fprintf(output, "    cmp rbx, rax\n");
            fprintf(output, "    setl al\n");
            fprintf(output, "    movzx rax, al\n");
            break;
        case OP_GT:
            fprintf(output, "    cmp rbx, rax\n");
            fprintf(output, "    setg al\n");
            fprintf(output, "    movzx rax, al\n");
            break;
        case OP_LAND:
            generate_code(node->binop.left, output);
            fprintf(output, "    cmp rax, 0\n");
            fprintf(output, "    setne al\n");
            fprintf(output, "    movzx rax, al\n");
            fprintf(output, "    push rax\n");

            generate_code(node->binop.right, output);
            fprintf(output, "    cmp rax, 0\n");
            fprintf(output, "    setne al\n");
            fprintf(output, "    movzx rax, al\n");

            fprintf(output, "    pop rbx\n");
            fprintf(output, "    and rax, rbx\n");
            break;
        case OP_LOR:
            generate_code(node->binop.left, output);
            fprintf(output, "    cmp rax, 0\n");
            fprintf(output, "    setne al\n");
            fprintf(output, "    movzx rax, al\n");
            fprintf(output, "    push rax\n");

            generate_code(node->binop.right, output);
            fprintf(output, "    cmp rax, 0\n");
            fprintf(output, "    setne al\n");
            fprintf(output, "    movzx rax, al\n");

            fprintf(output, "    pop rbx\n");
            fprintf(output, "    or rax, rbx\n");
            break;
        case OP_BAND:
            fprintf(output, "    and rax, rbx\n");
            break;
        case OP_BOR:
            fprintf(output, "    or rax, rbx\n");
            break;
        case OP_BXOR:
            fprintf(output, "    xor rax, rbx\n");
            break;
        case OP_LSHIFT:
            fprintf(output, "    mov rcx, rax\n");
            fprintf(output, "    mov rax, rbx\n");
            fprintf(output, "    shl rax, cl\n");
            break;
        case OP_RSHIFT:
            fprintf(output, "    mov rcx, rax\n");
            fprintf(output, "    mov rax, rbx\n");
            fprintf(output, "    sar rax, cl\n");
            break;
        case OP_BNAND:
            fprintf(output, "    and rax, rbx\n");
            fprintf(output, "    not rax\n");
            break;
        case OP_BNOR:
            fprintf(output, "    or rax, rbx\n");
            fprintf(output, "    not rax\n");
            break;
        case OP_BXNOR:
            fprintf(output, "    xor rax, rbx\n");
            fprintf(output, "    not rax\n");
            break;
        default:
            fprintf(output, "    ; unsupported operator\n");
            break;
    }
}

void handle_unop(ASTNode* node, FILE* output) {
    generate_code(node->unop.operand, output);
    switch (node->unop.op) {
        case OP_LNOT:
            fprintf(output, "    cmp rax, 0\n    sete al\n    movzx rax, al\n");
            break;
        case OP_BNOT:
            fprintf(output, "    not rax\n");
            break;
        case OP_NEG:
            fprintf(output, "    neg rax\n");
            break;
        case OP_POS:
            break;
        default:
            fprintf(output, "    ; unsupported unary operator\n");
            break;
    }
}

void handle_compound(ASTNode* node, FILE* output) {
    generate_code(node->binop.left, output);
    generate_code(node->binop.right, output);
}