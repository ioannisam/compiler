#include "parser/ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// implementations of AST constructors
ASTNode* create_print_node(ASTNode *expr) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_print_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_PRINT;
    node->print_expr.expr = expr;
    return node;
}

ASTNode* create_str_node(char* str) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_str_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_STR;
    node->str_value = strdup(str);
    return node;
}

ASTNode* create_ident_node(char* id) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_ident_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_IDENT;
    node->str_value = strdup(id);
    return node;
}

ASTNode* create_num_node(int value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_num_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_NUM;
    node->num_value = value;
    return node;
}

ASTNode* create_program_node(ASTNode* functions, ASTNode* main_block) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_program_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_PROGRAM;
    node->program.functions = functions;
    node->program.main_block = main_block;
    return node;
}

ASTNode* create_func_node(char* return_type, char* name, ASTNode* params, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_func_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_FUNC;
    node->func.return_type = strdup(return_type);
    node->func.name = strdup(name);
    node->func.params = params;
    node->func.body = body;
    return node;
}

ASTNode* create_call_node(char* func_name, ASTNode* args) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_call_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_CALL;
    node->func_call.func_name = strdup(func_name);
    node->func_call.args = args;
    return node;
}

ASTNode* append_arg(ASTNode* arg_list, ASTNode* arg) {
    if (!arg_list) {
        return create_compound_node(arg, NULL);
    }
    ASTNode* current = arg_list;
    while (current->binop.right != NULL) {
        current = current->binop.right;
    }
    current->binop.right = create_compound_node(arg, NULL);
    return arg_list;
}

ASTNode* append_function(ASTNode* func_list, ASTNode* func) {
    if (!func_list) {
        return create_compound_node(func, NULL);
    }
    ASTNode* current = func_list;
    while (current->binop.right != NULL) {
        current = current->binop.right;
    }
    current->binop.right = create_compound_node(func, NULL);
    return func_list;
}

ASTNode* create_param_node(char* type, char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_param_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_PARAM;
    node->param.type = strdup(type);
    node->param.name = strdup(name);
    return node;
}

ASTNode* append_param(ASTNode* param_list, ASTNode* param) {
    if (!param_list) {
        return create_compound_node(param, NULL);
    }
    ASTNode* current = param_list;
    while (current->binop.right != NULL) {
        current = current->binop.right;
    }
    current->binop.right = create_compound_node(param, NULL);
    return param_list;
}

ASTNode* create_if_node(ASTNode* cond, ASTNode* if_body, ASTNode* else_body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_if_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_IF;
    node->control.condition = cond;
    node->control.if_body = if_body;
    node->control.else_body = else_body;
    return node;
}

ASTNode* create_while_node(ASTNode* cond, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_while_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_WHILE;
    node->control.condition = cond;
    node->control.loop_body = body;
    return node;
}

ASTNode* create_break_node(void) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for break node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_BREAK;
    return node;
}

ASTNode* create_return_node(ASTNode* expr) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->return_stmt.expr = expr;
    return node;
}

ASTNode* create_decl_node(char* type, char* name, ASTNode* init_expr) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = NODE_DECL;
    node->decl.type = strdup(type);
    node->decl.name = strdup(name);
    node->decl.init_expr = init_expr;
    return node;
}

ASTNode* create_assign_node(char* id, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_assign_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_ASSIGN;
    node->assign.target = create_ident_node(id);
    node->assign.value = value;
    return node;
}

ASTNode* create_binop_node(Operator op, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_binop_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_BINOP;
    node->binop.op = op;
    node->binop.left = left;
    node->binop.right = right;
    return node;
}

ASTNode* create_compound_node(ASTNode* stmt, ASTNode* next) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_compound_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_COMPOUND;
    node->binop.left = stmt;
    node->binop.right = next;
    return node;
}

ASTNode* append_statement(ASTNode* compound, ASTNode* stmt) {
    if (!compound) {
        return create_compound_node(stmt, NULL);
    }
    ASTNode* current = compound;
    while (current->type == NODE_COMPOUND && current->binop.right != NULL) {
        current = current->binop.right;
    }
    if (current->type == NODE_COMPOUND) {
        current->binop.right = create_compound_node(stmt, NULL);
    } else {
        // handle error or wrap existing node
        ASTNode* new_compound = create_compound_node(current, create_compound_node(stmt, NULL));
        compound = new_compound;
    }
    return compound;
}

ASTNode* create_unop_node(Operator op, ASTNode* operand) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_unop_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_UNOP;
    node->unop.op = op;
    node->unop.operand = operand;
    return node;
}

ASTNode* create_empty_node(void) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed for empty node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_EMPTY;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    switch (node->type) {
        case NODE_CALL:
            free(node->func_call.func_name);
            free_ast(node->func_call.args);
            break;
        case NODE_ASSIGN:
            free_ast(node->binop.left);
            free_ast(node->binop.right);
            break;
        case NODE_IDENT:
        case NODE_STR:
            free(node->str_value);
            break;
        case NODE_BINOP:
            free_ast(node->binop.left);
            free_ast(node->binop.right);
            break;
        case NODE_IF:
            free_ast(node->control.condition);
            free_ast(node->control.if_body);
            free_ast(node->control.else_body);
            break;
        case NODE_WHILE:
            free_ast(node->control.condition);
            free_ast(node->control.loop_body);
            break;
        case NODE_BREAK:
            break;
        case NODE_RETURN:
           free_ast(node->return_stmt.expr);
           break;
        case NODE_PRINT:
            free_ast(node->print_expr.expr);
            break;
        case NODE_COMPOUND:
            free_ast(node->binop.left);
            free_ast(node->binop.right);
            break;
        case NODE_UNOP:
            free_ast(node->unop.operand);
            break;
        case NODE_EMPTY:
            break;
    }
    free(node);
}

const char* operator_to_string(Operator op) {
    switch (op) {
        case OP_POS:    return "POS";
        case OP_NEG:    return "NEG";
        case OP_LAND:   return "LAND";
        case OP_LOR:    return "LOR";
        case OP_LNOT:   return "LNOT";
        case OP_BNOT:   return "BNOT";
        case OP_BAND:   return "BAND";
        case OP_BOR:    return "BOR";
        case OP_BXOR:   return "BXOR";
        case OP_BNAND:  return "BNAND";
        case OP_BNOR:   return "BNOR";
        case OP_BXNOR:  return "BXNOR";
        case OP_EQ:     return "EQ";
        case OP_NEQ:    return "NEQ";
        case OP_GE:     return "GE";
        case OP_LE:     return "LE";
        case OP_LT:     return "LT";
        case OP_GT:     return "GT";
        case OP_ADD:    return "ADD";
        case OP_SUB:    return "SUB";
        case OP_MUL:    return "MUL";
        case OP_DIV:    return "DIV";
        case OP_MOD:    return "MOD";
        case OP_LSHIFT: return "LSHIFT";
        case OP_RSHIFT: return "RSHIFT";
        default:        return "UNKNOWN";
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    for (int i = 0; i < indent; i++) printf("  ");
    switch (node->type) {
        case NODE_PRINT:
            printf("PRINT\n");
            print_ast(node->print_expr.expr, indent+1);
            break;
        case NODE_BINOP:
            printf("BINOP(%s)\n", operator_to_string(node->binop.op));
            print_ast(node->binop.left, indent+1);
            print_ast(node->binop.right, indent+1);
            break;
        case NODE_NUM:
            printf("NUM(%d)\n", node->num_value);
            break;
        case NODE_STR:
            printf("STR(%s)\n", node->str_value);
            break;
        case NODE_IDENT:
            printf("IDENT(%s)\n", node->str_value);
            break;
        case NODE_IF:
            printf("IF\n");
            print_ast(node->control.condition, indent+1);
            printf("%*sTHEN:\n", indent*2, "");
            print_ast(node->control.if_body, indent+1);
            if (node->control.else_body) {
                printf("%*sELSE:\n", indent*2, "");
                print_ast(node->control.else_body, indent+1);
            }
            break;
        case NODE_WHILE:
            printf("WHILE\n");
            print_ast(node->control.condition, indent+1);
            printf("%*sBODY:\n", indent*2, "");
            print_ast(node->control.loop_body, indent+1);
            break;
        case NODE_BREAK:
            printf("BREAK\n");
            break;
        case NODE_ASSIGN:
            printf("ASSIGN\n");
            // For assignment, print both the left-hand side (target) and the right-hand side (expression)
            printf("%*sLHS:\n", indent*2, "");
            print_ast(node->binop.left, indent+1);
            printf("%*sRHS:\n", indent*2, "");
            print_ast(node->binop.right, indent+1);
            break;
        case NODE_COMPOUND:
            printf("COMPOUND\n");
            print_ast(node->binop.left, indent+1);
            print_ast(node->binop.right, indent);
            break;
        case NODE_UNOP:
            printf("UNOP(%s)\n", operator_to_string(node->unop.op));
            print_ast(node->unop.operand, indent+1);
            break;
        case NODE_EMPTY:
            printf("EMPTY\n");
            break;
    }
}