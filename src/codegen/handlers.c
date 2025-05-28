#include "codegen/handlers.h"
#include "codegen/symbol.h"
#include "codegen/helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void handle_program(ASTNode* node, FILE* output) {
    // First generate all functions
    generate_code(node->program.functions, output);
    
    if (has_main_function(node->program.functions)) {
        // For structured programs with main function
        fprintf(output, "START   JMP     main            * Jump to main function\n\n");
        
        // Add the RETURN section with proper terminal device handling
        fprintf(output, "RETURN  STA     RETVAL          * Save return value\n");
        fprintf(output, "        LDA     RETVAL          * Load return value\n");
        fprintf(output, "        CHAR                    * Convert numeric to characters (in X)\n");
        fprintf(output, "        STX     BUFFER          * Store chars to buffer\n");
        fprintf(output, "        OUT     BUFFER(TERM)    * Output buffer to terminal\n");
        fprintf(output, "        HLT                     * Halt\n");
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
    fprintf(output, "%s    STJ     RETADDR         * Save return address\n", node->func.name);
    
    // Handle parameters more robustly
    ASTNode* params = node->func.params;
    if (params && params->type == NODE_COMPOUND && params->binop.left) {
        ASTNode* param = params->binop.left;
        if (param->type == NODE_PARAM) {
            fprintf(output, "        LDA     ARG             * Load argument\n");
            fprintf(output, "        STA     %s              * Store to parameter\n", param->param.name);
        }
    }
    
    // Function body
    if (node->func.body) {
        generate_code(node->func.body, output);
    }
    
    // Default return if no explicit return
    fprintf(output, "        LDA     =0=             * Default return\n");
    fprintf(output, "        JMP     RETADDR         * Return to caller\n\n");
}

void handle_call(ASTNode* node, FILE* output) {
    // Process arguments (first argument only for simplicity)
    if (node->func_call.args && node->func_call.args->binop.left) {
        generate_code(node->func_call.args->binop.left, output);
        fprintf(output, "        STA     ARG             * Store argument\n");
    }
    
    // Save return address and call function
    fprintf(output, "        STJ     RETADDR         * Set return address\n");
    fprintf(output, "        JMP     %s              * Call function\n", 
            node->func_call.func_name);
}

void handle_num(ASTNode* node, FILE* output) {
    // In MIXAL, load immediate values with LDA
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
        // Use the stored string ID
        fprintf(output, "        OUT STR%d(TERM)\n", expr->num_value);
    } else {
        // Numeric output with proper formatting
        generate_code(expr, output);
        fprintf(output, "        CHAR\n");
        fprintf(output, "        STX BUFFER\n");
        fprintf(output, "        OUT BUFFER(TERM)\n");
    }
}

void handle_decl(ASTNode* node, FILE* output) {
    // Just add to symbol table, MIXAL variables will be declared in data section
    add_symbol(node->decl.name, NULL, node->decl.type);
    
    // For initialization, we'll use the STJ instruction to store a value
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
    
    // Generate condition code
    generate_code(node->control.condition, output);
    
    // Compare accumulator with 0 and jump to else if equal (condition is false)
    fprintf(output, "        CMPA =0=\n");
    fprintf(output, "        JE   ELSE%d\n", else_label);
    
    // Generate if-body code
    generate_code(node->control.if_body, output);
    
    // Jump to end after if-body - CRITICAL FIX HERE
    fprintf(output, "        JMP  END%d\n", end_label);
    
    // Define else label
    fprintf(output, "ELSE%d   NOP\n", else_label);
    
    // Generate else-body code if it exists
    if (node->control.else_body) {
        generate_code(node->control.else_body, output);
    }
    
    // Define end label
    fprintf(output, "END%d    NOP\n", end_label);
}

void handle_while(ASTNode* node, FILE* output) {
    int loop_label = code_label_counter++;
    int exit_label = code_label_counter++;
    
    // Push this loop's end label onto the stack
    push_loop(exit_label);
    
    // Add a JMP instruction from START to the loop label
    fprintf(output, "        JMP LOOP%d\n", loop_label);  // Jump to the loop start
    
    // Define the loop label
    fprintf(output, "LOOP%d   NOP\n", loop_label);  // Label with proper spacing
    
    // Generate condition code
    generate_code(node->control.condition, output);
    
    // Compare and jump if condition is false
    fprintf(output, "        CMPA =0=\n");
    fprintf(output, "        JE   EXIT%d\n", exit_label);
    
    // Generate loop body
    generate_code(node->control.loop_body, output);
    
    // Jump back to start of loop
    fprintf(output, "        JMP LOOP%d\n", loop_label);
    
    // Define the exit label
    fprintf(output, "EXIT%d   NOP\n", exit_label);
    
    // Pop loop end label
    pop_loop();
}

void handle_break(ASTNode* node, FILE* output) {
    int target_label = current_loop();
    fprintf(output, "        JMP  EXIT%d\n", target_label);
}

void handle_return(ASTNode* node, FILE* output) {
    // Generate the return expression
    if (node->return_stmt.expr) {
        generate_code(node->return_stmt.expr, output);
    } else {
        // Default return 0
        fprintf(output, "        LDA     =0=             * Default return value\n");
    }
    
    // Return to caller
    fprintf(output, "        JMP     RETADDR         * Return to caller\n");
}

void handle_binop(ASTNode* node, FILE* output) {
    switch (node->binop.op) {
        case OP_ADD:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA TEMP2\n");
            fprintf(output, "        LDA TEMP\n");
            fprintf(output, "        ADD TEMP2\n");
            break;
            
        case OP_SUB:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA TEMP2\n");
            fprintf(output, "        LDA TEMP\n");
            fprintf(output, "        SUB TEMP2\n");
            break;
            
        case OP_MUL:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA TEMP2\n");
            fprintf(output, "        LDA TEMP\n");
            fprintf(output, "        MUL TEMP2\n");
            break;
            
        case OP_DIV:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA TEMP2\n");
            fprintf(output, "        LDA TEMP\n");
            fprintf(output, "        DIV TEMP2\n");
            break;
            
        case OP_LT:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA TEMP2\n");
            fprintf(output, "        LDA TEMP\n");
            fprintf(output, "        CMPA TEMP2\n");
            fprintf(output, "        JL 1F\n");
            fprintf(output, "        LDA =0=\n");
            fprintf(output, "        JMP 2F\n");
            fprintf(output, "1H      LDA =1=\n");
            fprintf(output, "2H      NOP\n");
            break;
            
        case OP_GT:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA TEMP2\n");
            fprintf(output, "        LDA TEMP\n");
            fprintf(output, "        CMPA TEMP2\n");
            fprintf(output, "        JG 1F\n");
            fprintf(output, "        LDA =0=\n");
            fprintf(output, "        JMP 2F\n");
            fprintf(output, "1H      LDA =1=\n");
            fprintf(output, "2H      NOP\n");
            break;
            
        case OP_EQ:
            generate_code(node->binop.left, output);
            fprintf(output, "        STA TEMP\n");
            generate_code(node->binop.right, output);
            fprintf(output, "        STA TEMP2\n");
            fprintf(output, "        LDA TEMP\n");
            fprintf(output, "        CMPA TEMP2\n");
            fprintf(output, "        JE 1F\n");
            fprintf(output, "        LDA =0=\n");
            fprintf(output, "        JMP 2F\n");
            fprintf(output, "1H      LDA =1=\n");
            fprintf(output, "2H      NOP\n");
            break;
            
        default:
            fprintf(stderr, "Unsupported binary operation: %d\n", node->binop.op);
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