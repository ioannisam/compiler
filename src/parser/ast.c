// src/parser/ast.c
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

ASTNode* create_if_node(ASTNode* cond, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_if_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_IF;
    node->control.condition = cond;
    node->control.if_body = body;
    node->control.else_body = NULL;
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

ASTNode* create_assign_node(char* id, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_assign_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_ASSIGN;
    node->binop.left = create_ident_node(id);
    node->binop.right = value;
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

ASTNode* create_if_else_node(ASTNode* cond, ASTNode* if_body, ASTNode* else_body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in create_if_else_node\n");
        exit(EXIT_FAILURE);
    }
    node->type = NODE_IF;
    node->control.condition = cond;
    node->control.if_body = if_body;
    node->control.else_body = else_body;
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
        case OP_EQ:      return "EQ";
        case OP_LT:      return "LT";
        case OP_GT:      return "GT";
        case OP_AND:     return "AND";
        case OP_OR:      return "OR";
        case OP_XOR:     return "XOR";
        case OP_NOT:     return "NOT";
        case OP_LSHIFT:  return "LSHIFT";
        case OP_RSHIFT:  return "RSHIFT";
        case OP_ADD:     return "ADD";
        case OP_SUB:     return "SUB";
        case OP_MUL:     return "MUL";
        case OP_DIV:     return "DIV";
        default:         return "UNKNOWN";
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