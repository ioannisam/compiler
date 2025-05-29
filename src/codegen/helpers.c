#include "codegen/helpers.h"
#include "codegen/symbol.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LOOP_DEPTH 100
int loop_stack[MAX_LOOP_DEPTH];
int loop_stack_top = -1;

int data_label_counter = 0;
int code_label_counter = 0;

void push_loop(int label) {
    if (loop_stack_top >= MAX_LOOP_DEPTH - 1) {
        fprintf(stderr, "Error: Loop nesting too deep\n");
        exit(EXIT_FAILURE);
    }
    loop_stack[++loop_stack_top] = label;
}

int pop_loop(void) {
    if (loop_stack_top < 0) {
        fprintf(stderr, "Error: Loop stack underflow\n");
        exit(EXIT_FAILURE);
    }
    return loop_stack[loop_stack_top--];
}

int current_loop(void) {
    if (loop_stack_top < 0) {
        fprintf(stderr, "Error: Break outside of loop\n");
        exit(EXIT_FAILURE);
    }
    return loop_stack[loop_stack_top];
}

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

void emit_string_data(ASTNode* node, FILE* output) {

    static ASTNode* processed_strings[100] = {NULL};
    static int processed_count = 0;
    
    if (!node) return;
    
    if (node->type == NODE_PRINT && node->print_expr.expr && 
        node->print_expr.expr->type == NODE_STR) {
        
        int already_processed = 0;
        for (int i = 0; i < processed_count; i++) {
            if (processed_strings[i] == node->print_expr.expr) {
                already_processed = 1;
                break;
            }
        }
        
        if (!already_processed) {
            processed_strings[processed_count++] = node->print_expr.expr;
            
            const char* str = node->print_expr.expr->str_value;
            static int str_label = 0;
            
            fprintf(output, "MSG%d   ", str_label++);
            
            int len = strlen(str);
            int i = 0;
            
            while (i < len) {
                fprintf(output, "ALF \"");
                
                // up to 5 characters per ALF directive
                int j;
                for (j = 0; j < 5 && i < len; j++, i++) {
                    fputc(str[i], output);
                }
                
                // pad with spaces
                for (; j < 5; j++) {
                    fputc(' ', output);
                }
                
                fprintf(output, "\"\n");
                
                // if there is a continuation
                if (i < len) {
                    fprintf(output, "        ");
                }
            }
            fprintf(output, "        ALF \"     \"\n");
        }
    }
    
    if (node->type == NODE_PROGRAM) {
        emit_string_data(node->program.functions, output);
        emit_string_data(node->program.main_block, output);
    } else if (node->type == NODE_COMPOUND) {
        emit_string_data(node->binop.left, output);
        emit_string_data(node->binop.right, output);
    } else if (node->type == NODE_IF) {
        emit_string_data(node->control.condition, output);
        emit_string_data(node->control.if_body, output);
        if (node->control.else_body) {
            emit_string_data(node->control.else_body, output);
        }
    } else if (node->type == NODE_WHILE) {
        emit_string_data(node->control.condition, output);
        emit_string_data(node->control.loop_body, output);
    } else if (node->type == NODE_FUNC) {
        emit_string_data(node->func.body, output);
    }
}

void collect_and_define_strings(ASTNode* node, FILE* output) {
    if (!node) return;
    
    if (node->type == NODE_PRINT && node->print_expr.expr && 
        node->print_expr.expr->type == NODE_STR) {
        
        static int str_counter = 0;
        int current_str = str_counter++;
        const char* str = node->print_expr.expr->str_value;
        
        node->print_expr.expr->num_value = current_str;
        
        fprintf(output, "STR%d   ", current_str);
        
        // MIXAL strings use ALF directives (5 chars each)
        int len = strlen(str);
        int pos = 0;
        while (pos < len) {
            fprintf(output, "ALF \"");
            
            int chars_to_output = (len - pos > 5) ? 5 : len - pos;
            for (int i = 0; i < chars_to_output; i++) {
                fputc(str[pos + i], output);
            }
            
            // pad with spaces if needed
            for (int i = chars_to_output; i < 5; i++) {
                fputc(' ', output);
            }
            
            fprintf(output, "\"\n");
            pos += chars_to_output;
            
            // next ALF if needed
            if (pos < len) {
                fprintf(output, "        ");
            }
        }
        
        fprintf(output, "        ALF \"     \"\n");
    }
    
    if (node->type == NODE_PROGRAM) {
        collect_and_define_strings(node->program.functions, output);
        collect_and_define_strings(node->program.main_block, output);
    } else if (node->type == NODE_COMPOUND) {
        collect_and_define_strings(node->binop.left, output);
        collect_and_define_strings(node->binop.right, output);
    } else if (node->type == NODE_IF) {
        collect_and_define_strings(node->control.condition, output);
        collect_and_define_strings(node->control.if_body, output);
        if (node->control.else_body) {
            collect_and_define_strings(node->control.else_body, output);
        }
    } else if (node->type == NODE_WHILE) {
        collect_and_define_strings(node->control.condition, output);
        collect_and_define_strings(node->control.loop_body, output);
    } else if (node->type == NODE_FUNC) {
        collect_and_define_strings(node->func.body, output);
    }
}