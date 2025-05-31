#include "codegen/handlers.h"
#include "codegen/symbol.h"
#include "codegen/helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void handle_program(ASTNode* node, FILE* output) {

    generate_code(node->program.functions, output);

    fprintf(output, "global _start\n_start:\n");
    
    if (has_main_function(node->program.functions)) { 
        fprintf(output,
            "    call main\n"
            "    mov rdi, rax\n"
            "    mov rax, 60\n"
            "    syscall\n");
    } else if (node->program.main_block) { 
        generate_code(node->program.main_block, output);
        fprintf(output, "    mov rax, 60\n    xor rdi, rdi\n    syscall\n");
    } else {
        fprintf(stderr, "Error: No entry point (main function or MAIN block)\n");
        exit(EXIT_FAILURE);
    }
}

void handle_function(ASTNode* node, FILE* output) {
    fprintf(output, "%s:\n", node->func.name);
    fprintf(output, "    push rbp\n");
    fprintf(output, "    mov rbp, rsp\n");

    ASTNode* params = node->func.params;
    int param_offset = 16;
    while (params) {
        if (params->type == NODE_COMPOUND) {
            ASTNode* param_node = params->binop.left;
            if (param_node->type == NODE_PARAM) {
                Symbol* sym = lookup_symbol(param_node->param.name);
                if (sym) {
                    fprintf(output, "    mov rax, [rbp + %d]\n", param_offset);
                    fprintf(output, "    mov [%s], rax\n", sym->label);
                    param_offset += 8;
                }
            }
            params = params->binop.right;
        } else if (params->type == NODE_PARAM) {
            Symbol* sym = lookup_symbol(params->param.name);
            if (sym) {
                fprintf(output, "    mov rax, [rbp + %d]\n", param_offset);
                fprintf(output, "    mov [%s], rax\n", sym->label);
            }
            break;
        } else {
            break;
        }
    }

    generate_code(node->func.body, output);

    fprintf(output, "    mov rsp, rbp\n");
    fprintf(output, "    pop rbp\n");
    fprintf(output, "    ret\n\n");
}

void handle_call(ASTNode* node, FILE* output) {
    int arg_count = 0;
    ASTNode* current_arg = node->func_call.args;

    while (current_arg) {
        if (current_arg->type == NODE_COMPOUND) {
            generate_code(current_arg->binop.left, output);
            fprintf(output, "    push rax\n");
            arg_count++;
            current_arg = current_arg->binop.right;
        } else {
            generate_code(current_arg, output);
            fprintf(output, "    push rax\n");
            arg_count++;
            break;
        }
    }

    fprintf(output, "    call %s\n", node->func_call.func_name);
    if (arg_count > 0) {
        fprintf(output, "    add rsp, %d\n", arg_count * 8);
    }
}

void handle_num(ASTNode* node, FILE* output) {
    fprintf(output, "    mov rax, %d\n", node->num_value);
}

void handle_ident(ASTNode* node, FILE* output) {
    Symbol* sym = lookup_symbol(node->str_value);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->str_value);
        exit(EXIT_FAILURE);
    }
    fprintf(output, "    mov rax, [%s]\n", sym->label);
}

void handle_print(ASTNode* node, FILE* output) {

    ASTNode* expr = node->print_expr.expr;
    if (expr->type == NODE_STR) {
        int len = (int)strlen(expr->str_value);
        static int msg_ref_counter = 0;
        fprintf(output,
            "    mov rax, 1\n"
            "    mov rdi, 1\n"
            "    mov rsi, msg%d\n"
            "    mov rdx, %d\n"
            "    syscall\n",
            msg_ref_counter++, len + 1);
    } else {
        generate_code(expr, output);
        fprintf(output,
            "    push rax\n    push rbx\n    push rcx\n    push rdx\n    push rdi\n    push rsi\n"
            "    mov rdi, rax\n"
            "    mov rsi, print_buffer\n"
            "    call itoa\n"
            "    mov rsi, print_buffer\n"
            "    add rsi, 20\n"
            "    sub rsi, rax\n"
            "    mov rdx, rax\n"
            "    mov rax, 1\n"
            "    mov rdi, 1\n"
            "    syscall\n"
            "    pop rsi\n    pop rdi\n    pop rdx\n    pop rcx\n    pop rbx\n    pop rax\n");
    }
}

void handle_decl(ASTNode* node, FILE* output) {
    Symbol* sym = add_symbol(node->decl.name, NULL, node->decl.type);
    if (node->decl.init_expr) {
        generate_code(node->decl.init_expr, output);
        fprintf(output, "    mov [%s], rax\n", sym->label);
    }
}

void handle_assign(ASTNode* node, FILE* output) {
    if (node->assign.target->type != NODE_IDENT) {
        fprintf(stderr, "Error: Assignment target must be an identifier\n");
        exit(EXIT_FAILURE);
    }
    Symbol* sym = lookup_symbol(node->assign.target->str_value);
    if (!sym) {
        fprintf(stderr, "Error: Variable '%s' not declared\n", node->assign.target->str_value);
        exit(EXIT_FAILURE);
    }
    generate_code(node->assign.value, output);
    fprintf(output, "    mov [%s], rax\n", sym->label);
}

void handle_compound(ASTNode* node, FILE* output) {
    generate_code(node->binop.left, output);
    generate_code(node->binop.right, output);
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
    int start_label = code_label_counter++;
    int end_label = code_label_counter++;
    
    fprintf(output, ".Lwhile%d:\n", start_label);
    generate_code(node->control.condition, output);
    
    fprintf(output, "    cmp rax, 0\n");
    fprintf(output, "    je .Lend%d\n\n", end_label);
    
    generate_code(node->control.loop_body, output);
    
    fprintf(output, "    jmp .Lwhile%d\n", start_label);
    fprintf(output, ".Lend%d:\n\n", end_label);
}

void handle_break(ASTNode* node, FILE* output) {
    fprintf(output, "    jmp .Lend%d\n", code_label_counter - 1);
}

void handle_return(ASTNode* node, FILE* output) {
    if (node->return_stmt.expr) {
        generate_code(node->return_stmt.expr, output);
    } else {
        fprintf(output, "    xor rax, rax\n");
    }
    fprintf(output,
        "    mov rsp, rbp\n"
        "    pop rbp\n"
        "    ret\n\n");
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
            fprintf(output, "    mov rcx, rax\n");
            fprintf(output, "    mov rax, rbx\n");
            fprintf(output, "    mov rbx, rcx\n");
            fprintf(output, "    cqo\n");
            fprintf(output, "    idiv rbx\n");
            fprintf(output, "    mov rax, rdx\n");
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
        case OP_GE:
            fprintf(output, "    cmp rbx, rax\n");
            fprintf(output, "    setge al\n");
            fprintf(output, "    movzx rax, al\n");
            break;
        case OP_LE:
            fprintf(output, "    cmp rbx, rax\n");
            fprintf(output, "    setle al\n");
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