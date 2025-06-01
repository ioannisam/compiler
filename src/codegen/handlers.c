#include "codegen/handlers.h"
#include "codegen/symbol.h"
#include "codegen/helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void handle_program(ASTNode* node, FILE* output) {
    generate_code(node->program.functions, output);
    
    if (has_main_function(node->program.functions)) {
        // structured programs
        fprintf(output, "START   JMP     main\n\n");
        
        fprintf(output, "RETURN  STA     RETVAL\n");
        fprintf(output, "        LDA     RETVAL\n");
        fprintf(output, "        CHAR\n");
        fprintf(output, "        STX     BUFFER\n");
        fprintf(output, "        OUT     BUFFER(TERM)\n");
        fprintf(output, "        HLT\n");
        fprintf(output, "        HLT\n");
        fprintf(output, "        HLT\n");
    } else if (node->program.main_block) {
        // For scripted programs
        fprintf(output, "START   NOP\n");
        generate_code(node->program.main_block, output);
        fprintf(output, "        HLT\n");
    }
}

void handle_function(ASTNode* node, FILE* output) {
    char func_label[7];
    strncpy(func_label, node->func.name, 6);
    func_label[6] = '\0';
    
    static int func_counter = 0;
    int this_func = func_counter++;
    
    fprintf(output, "%s    STJ  RT%d\n", 
            func_label, this_func);
    
    fprintf(output, "        ST1  T1%d\n", this_func);
    fprintf(output, "        ST2  T2%d\n", this_func);
    
    ASTNode* params = node->func.params;
    if (params && params->type == NODE_COMPOUND) {
        if (params->binop.left && params->binop.left->type == NODE_PARAM) {
            char* param_name = params->binop.left->param.name;
            fprintf(output, "        STA  %s              * Store first parameter\n", param_name);
        }
    }
    
    if (node->func.body) {
        generate_code(node->func.body, output);
    }
    
    fprintf(output, "        LD1  T1%d\n", this_func);
    fprintf(output, "        LD2  T2%d\n", this_func);
    
    fprintf(output, "RT%d    JMP  *\n\n", this_func);
    
    fprintf(output, "T1%d    CON  0\n", this_func);
    fprintf(output, "T2%d    CON  0\n", this_func);
}

void handle_call(ASTNode* node, FILE* output) {
    if (node->func_call.args && node->func_call.args->binop.left) {
        generate_code(node->func_call.args->binop.left, output);
    }
    
    char func_label[7];
    strncpy(func_label, node->func_call.func_name, 6);
    func_label[6] = '\0';
    
    fprintf(output, "        JMP  %s\n", func_label);
}

void handle_num(ASTNode* node, FILE* output) {
    fprintf(output, "        LDA =%d=\n", node->num_value);
}

void handle_ident(ASTNode* node, FILE* output) {
    Symbol* sym = lookup_symbol(node->str_value);
    if (!sym) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->str_value);
        exit(EXIT_FAILURE);
    }
    fprintf(output, "        LDA %s\n", node->str_value);
}

void handle_print(ASTNode* node, FILE* output) {
    ASTNode* expr = node->print_expr.expr;
    
    if (expr->type == NODE_STR) {
        fprintf(output, "        OUT STR%d(TERM)\n", expr->num_value);
    } else {
        generate_code(expr, output);
        fprintf(output, "        CHAR\n");
        fprintf(output, "        STX BUFFER\n");
        fprintf(output, "        OUT BUFFER(TERM)\n");
    }
}

void handle_decl(ASTNode* node, FILE* output) {
    add_symbol(node->decl.name, NULL, node->decl.type);
    
    if (node->decl.init_expr) {
        generate_code(node->decl.init_expr, output);
        fprintf(output, "        STA %s\n", node->decl.name);
    }
}

void handle_assign(ASTNode* node, FILE* output) {
    if (node->assign.target->type != NODE_IDENT) {
        fprintf(stderr, "Error: Assignment target must be an identifier\n");
        exit(EXIT_FAILURE);
    }
    
    Symbol* sym = lookup_symbol(node->assign.target->str_value);
    if (!sym) {
        fprintf(stderr, "Error: Variable '%s' not declared\n");
        exit(EXIT_FAILURE);
    }
    
    generate_code(node->assign.value, output);
    fprintf(output, "        STA %s\n", node->assign.target->str_value);
}

void handle_compound(ASTNode* node, FILE* output) {
    generate_code(node->binop.left, output);
    generate_code(node->binop.right, output);
}

void handle_if(ASTNode* node, FILE* output) {
    int else_label = code_label_counter++;
    int end_label = code_label_counter++;
    
    generate_code(node->control.condition, output);
    
    fprintf(output, "        CMPA =0=\n");
    fprintf(output, "        JE   ELSE%d\n", else_label);
    
    generate_code(node->control.if_body, output);
    
    fprintf(output, "        JMP  END%d\n", end_label);
    
    fprintf(output, "ELSE%d   NOP\n", else_label);
    
    if (node->control.else_body) {
        generate_code(node->control.else_body, output);
    }
    
    fprintf(output, "END%d    NOP\n", end_label);
}

void handle_while(ASTNode* node, FILE* output) {
    int loop_label = code_label_counter++;
    int exit_label = code_label_counter++;
    
    push_loop(exit_label);
    
    fprintf(output, "        JMP LOOP%d\n", loop_label);
    fprintf(output, "LOOP%d   NOP\n", loop_label);
    generate_code(node->control.condition, output);
    
    fprintf(output, "        CMPA =0=\n");
    fprintf(output, "        JE   EXIT%d\n", exit_label);
    
    generate_code(node->control.loop_body, output);
    fprintf(output, "        JMP LOOP%d\n", loop_label);
    fprintf(output, "EXIT%d   NOP\n", exit_label);
    
    pop_loop();
}

void handle_break(ASTNode* node, FILE* output) {
    int target_label = current_loop();
    fprintf(output, "        JMP  EXIT%d\n", target_label);
}

void handle_return(ASTNode* node, FILE* output) {
    if (node->return_stmt.expr) {
        generate_code(node->return_stmt.expr, output);
    } else {
        fprintf(output, "        LDA  =0=             * Default return 0\n");
    }
}

void handle_binop(ASTNode* node, FILE* output) {
    switch (node->binop.op) {
        case OP_ADD:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA     TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        ADD     TEMP\n");
            break;
            
        case OP_SUB:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA     TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA     TEMP2\n");
            fprintf(output, "        LDA     TEMP\n");
            fprintf(output, "        SUB     TEMP2\n");
            break;
            
        case OP_MUL:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA     TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        MUL     TEMP\n");
            fprintf(output, "        STX     TEMP2\n");
            fprintf(output, "        LDA     TEMP2\n");
            break;

        case OP_DIV:
            // First evaluate and store the divisor
            generate_code(node->binop.right, output);
            fprintf(output, "        STA     TEMP\n");
            
            // Load dividend into X register via A
            generate_code(node->binop.left, output);
            fprintf(output, "        STA     TEMP2\n");
            fprintf(output, "        LDX     TEMP2\n");
            
            // Clear A register AFTER loading X
            fprintf(output, "        ENTA    0\n");
            
            // Perform division
            fprintf(output, "        DIV     TEMP\n");
            
            // Result (quotient) is now in rA
            break;
        case OP_MOD:
            // First evaluate and store the divisor
            generate_code(node->binop.right, output);
            fprintf(output, "        STA     TEMP\n");
            
            // Load dividend into X register via A
            generate_code(node->binop.left, output);
            fprintf(output, "        STA     TEMP2\n");
            fprintf(output, "        LDX     TEMP2\n");
            
            // Clear A register before division
            fprintf(output, "        ENTA    0\n");
            
            // Perform division
            fprintf(output, "        DIV     TEMP\n");
            
            // For modulo, we want the remainder which is in rX
            // Transfer X to A since our convention is to return results in A
            fprintf(output, "        STX     TEMP2\n");
            fprintf(output, "        LDA     TEMP2\n");
            break;
        case OP_LT:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA     TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA     TEMP2\n");
            fprintf(output, "        LDA     TEMP\n");
            fprintf(output, "        CMPA    TEMP2\n");
            fprintf(output, "        JGE     *+3\n");
            fprintf(output, "        LDA     =1=\n");
            fprintf(output, "        JMP     *+2\n");
            fprintf(output, "        LDA     =0=\n");
            break;
            
        case OP_GT:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA     TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA     TEMP2\n");
            fprintf(output, "        LDA     TEMP\n");
            fprintf(output, "        CMPA    TEMP2\n");
            fprintf(output, "        JLE     *+3\n");
            fprintf(output, "        LDA     =1=\n");
            fprintf(output, "        JMP     *+2\n");
            fprintf(output, "        LDA     =0=\n");
            break;
            
        case OP_EQ:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA     TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA     TEMP2\n");
            fprintf(output, "        LDA     TEMP\n");
            fprintf(output, "        CMPA    TEMP2\n");
            fprintf(output, "        JNE     *+3\n");
            fprintf(output, "        LDA     =1=\n");
            fprintf(output, "        JMP     *+2\n");
            fprintf(output, "        LDA     =0=\n");
            break;
            
        default:
            fprintf(output, "        * Unsupported binary operator\n");
            fprintf(output, "        LDA     =0=             * default to 0\n");
    }
}

void handle_unop(ASTNode* node, FILE* output) {
    generate_code(node->unop.operand, output);
    
    switch (node->unop.op) {
        case OP_LNOT:
            fprintf(output, "        CMPA    =0=\n");
            fprintf(output, "        JNE     *+3\n");
            fprintf(output, "        LDA     =1=\n");
            fprintf(output, "        JMP     *+2\n");
            fprintf(output, "        LDA     =0=\n");
            break;
            
        case OP_BNOT:
            fprintf(output, "        STA     TEMP\n");
            fprintf(output, "        ENTA    0,1\n");
            fprintf(output, "        SUB     TEMP\n");
            fprintf(output, "        SUB     =1=\n");
            break;
            
        case OP_NEG:
            fprintf(output, "        STA     TEMP\n");
            fprintf(output, "        ENTA    0,1\n");
            fprintf(output, "        SUB     TEMP\n");
            break;
            
        case OP_POS:
            break;
        default:
            fprintf(output, "        * Unsupported unary operator\n");
    }
}